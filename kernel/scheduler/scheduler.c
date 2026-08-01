#include <kernel/scheduler/scheduler.h>

const char user_program_code[14] = {
    0xB8, 0x01, 0x00, 0x00, 0x00,
    0xBB, 0x02, 0x00, 0x00, 0x00,
    0x48, 0x01, 0xD8,
    0xC3
};

volatile int scheduler = 0;

void init_scheduler(void){
    process_t *main_process = (process_t*)kmalloc(sizeof(process_t));
    memset(main_process, 0, sizeof(process_t));

    thread_t *main_thread = (thread_t *)kmalloc(sizeof(thread_t));
    main_thread->tid = next_thread_id++;
    main_thread->state = RUNNING;
    main_thread->rsp = 0;
    main_thread->kernel_stack.bottom = NULL;

    enqueue_thread(main_thread);
    current_thread = main_thread;

    main_process->pml4 = read_cr3();
    main_process->threads = main_thread;
    main_process->pid = 0;

    create_thread(&idle_thread_entry, DEFAULT_STACK_SIZE);
    create_thread(&third_thread, DEFAULT_STACK_SIZE);

    init_user_thread_exit();

    vmm_map_page(read_cr3(), pmm_alloc_page(), 0x400000, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));
    memcpy((void*)0x400000, user_program_code, sizeof(user_program_code));

    // create_user_thread((void*)0x400000, 4096, 4096);
    create_user_process((void*)0x400000, 4096, 4096);

    scheduler = 1;
}

uint64_t scheduler_handler(uint64_t old_rsp){
    if (!ready_list_head || ready_list_head->next == ready_list_head) return old_rsp;

    thread_t *old_thread = current_thread;

    thread_t *starting_point = old_thread->next;

    if (old_thread->state == DEAD){
        dequeue_thread(old_thread);
        old_thread->next = dead_list_head;
        dead_list_head = old_thread;
    } else if (old_thread->state == BLOCKED){
        old_thread->rsp = old_rsp;
        dequeue_thread(old_thread);
    } else {
        old_thread->rsp = old_rsp;
        old_thread->state = READY;
    }

    if (old_thread->state == DEAD || old_thread->state == BLOCKED) starting_point = ready_list_head;
    thread_t *next_thread = starting_point;

    if (!next_thread) return old_rsp;

    while (next_thread->state != READY && next_thread->state != RUNNING){
        next_thread = next_thread->next;
        if (next_thread == starting_point){
            if (old_thread->state == DEAD || old_thread->state == BLOCKED) for(;;);
            old_thread->state = RUNNING;
            return old_rsp;
        }
        
    }

    next_thread->state = RUNNING;
    current_thread = next_thread;

    tss.rsp0 = (uint64_t)current_thread->kernel_stack.top;
    sstacks.kernel_rsp = (uint64_t)current_thread->kernel_stack.top;

    if (old_thread->process != next_thread->process && next_thread->process){
        serial_print("%llx %llx\n", (uint64_t)old_thread->process, (uint64_t)next_thread->process);
        write_cr3(next_thread->process->pml4);
        us_list_head = next_thread->process->ustacks_infos;
    }
    
    return next_thread->rsp;
}