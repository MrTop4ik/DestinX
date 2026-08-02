#include <kernel/scheduler/scheduler.h>

const char user_program_code[14] = {
    0xB8, 0x01, 0x00, 0x00, 0x00,
    0xBB, 0x02, 0x00, 0x00, 0x00,
    0x48, 0x01, 0xD8,
    0xC3
};

const char user_exit_trampoline[10] = {
    0xB8, 0x3C, 0x00, 0x00, 0x00, 
    0x48, 0x31, 0xFF, 
    0x0F, 0x05
};

volatile int scheduler = 0;

void init_scheduler(void){
    thread_t *main_thread = (thread_t *)kmalloc(sizeof(thread_t));
    main_thread->tid = next_thread_id++;
    main_thread->state = RUNNING;
    main_thread->rsp = 0;
    main_thread->kernel_stack.bottom = NULL;

    enqueue_thread(main_thread);
    current_thread = main_thread;

    create_thread(&idle_thread_entry, DEFAULT_STACK_SIZE);
    create_thread(&third_thread, DEFAULT_STACK_SIZE);

    process_t *proc = create_user_process();

    uint64_t old_pml4_phys = read_cr3();
    write_cr3(proc->pml4);

    vmm_map_page(read_cr3(), pmm_alloc_page(), 0x400000, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));
    memcpy((void*)0x400000, user_program_code, sizeof(user_program_code));

    vmm_map_page(read_cr3(), pmm_alloc_page(), 0x300000, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));
    memcpy((void*)0x300000, user_exit_trampoline, sizeof(user_exit_trampoline));

    write_cr3(old_pml4_phys);

    create_user_thread(proc, (void*)0x400000, 4096, 4096);

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

    if (old_thread->process != next_thread->process) {
        if (old_thread->process) {
            old_thread->process->ustacks_infos = us_list_head;
        }

        if (next_thread->process) {
            us_list_head = next_thread->process->ustacks_infos;
            if (read_cr3() != next_thread->process->pml4) {
                write_cr3(next_thread->process->pml4);
            }
        } else {
            us_list_head = NULL;
        }
    }

    tss.rsp0 = (uint64_t)current_thread->kernel_stack.top;
    sstacks.kernel_rsp = (uint64_t)current_thread->kernel_stack.top;
    
    return next_thread->rsp;
}