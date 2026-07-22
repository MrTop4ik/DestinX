#include <kernel/scheduler/scheduler.h>

syscalls_stacks_t sstacks;

volatile int scheduler = 0;

void init_scheduler(void){
    thread_t *main_thread = (thread_t *)kmalloc(sizeof(thread_t));
    main_thread->tid = next_thread_id++;
    main_thread->state = RUNNING;
    main_thread->rsp = 0;
    main_thread->kernel_stack.bottom = NULL;

    enqueue_thread(main_thread);
    current_thread = main_thread;

    create_thread(&idle_thread_entry, 0x1000);
    create_thread(&third_thread, 0x1000);

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

    return next_thread->rsp;
}