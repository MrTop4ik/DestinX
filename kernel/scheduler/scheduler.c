#include <kernel/scheduler/scheduler.h>

extern vm_info_t *mmap_list_head;

volatile int scheduler = 0;

void init_scheduler(void){
    process_t *kp = (process_t *)kmalloc(sizeof(process_t));
    memset(kp, 0, sizeof(process_t));

    kp->pml4 = read_cr3();

    thread_t *main_thread = (thread_t *)kmalloc(sizeof(thread_t));
    main_thread->tid = next_thread_id++;
    main_thread->state = RUNNING;
    main_thread->rsp = 0;
    main_thread->kernel_stack.bottom = NULL;
    main_thread->process = kp;

    enqueue_thread(main_thread);
    current_thread = main_thread;

    thread_t *kr_thread = create_thread(&kring_flush, 4096);
    thread_t *cln_thread = create_thread(&cleanup_threads, 4096);

    main_thread->next_pthread = kr_thread;
    kr_thread->next_pthread = cln_thread;

    kp->threads = main_thread;

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

    if (old_thread->process != next_thread->process){
        if (old_thread->process){
            old_thread->process->ustacks_infos = us_list_head;
            old_thread->process->mmap_infos = mmap_list_head;
        }

        if (next_thread->process){
            us_list_head = next_thread->process->ustacks_infos;
            mmap_list_head = next_thread->process->mmap_infos;
            if (read_cr3() != next_thread->process->pml4) {
                write_cr3(next_thread->process->pml4);
            }
        } else {
            us_list_head = NULL;
            mmap_list_head = NULL;
        }
    }

    if (next_thread->process) init_kernel_gs_base();

    tss.rsp0 = (uint64_t)current_thread->kernel_stack.top;
    sstacks.kernel_rsp = (uint64_t)current_thread->kernel_stack.top;
    
    return next_thread->rsp;
}