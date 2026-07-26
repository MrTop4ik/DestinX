#include <kernel/scheduler/thread.h>

uint64_t next_thread_id = 0;

thread_t *current_thread = NULL;
thread_t *ready_list_head = NULL;
thread_t *dead_list_head = NULL;

thread_t *create_thread(void (*entry_point)(void), size_t stack_size){
    thread_t *t = (thread_t *)kmalloc(sizeof(thread_t));
    if (!t) return NULL;

    memset(t, 0, sizeof(thread_t));

    void *stack_mem = kernel_alloc_stack(stack_size);
    if (!stack_mem){
        kfree(t);
        return NULL;
    }

    serial_print("[THREAD] Kernel Thread Address at %llx\n", (uint64_t)t);
    serial_print("[THREAD] Kernel Thread Stack Bottom at %llx\n", (uint64_t)stack_mem);

    t->kernel_stack.bottom = stack_mem;
    t->kernel_stack.top = (void *)((uint64_t)stack_mem + stack_size);
    t->kernel_stack.size = stack_size;

    uint64_t *stack_top = (uint64_t*)((uint64_t)stack_mem + stack_size);
    stack_top = (uint64_t*)((uint64_t)stack_top & ~15UL);

    stack_top--; *stack_top = (uint64_t)&thread_exit;

    uint64_t *clean_stack_top = stack_top;

    stack_top--; *stack_top = 0x10;
    stack_top--; *stack_top = (uint64_t)clean_stack_top;
    stack_top--; *stack_top = 0x202;
    stack_top--; *stack_top = 0x08;
    stack_top--; *stack_top = (uint64_t)entry_point;

    for (int i = 0; i < 15; i++){
        stack_top--; *stack_top = 0;
    }

    t->rsp = (uint64_t)stack_top;
    t->tid = next_thread_id++;
    t->state = READY;
    t->next = NULL;
    t->prev = NULL;

    enqueue_thread(t);
    return t;
}

thread_t *create_user_thread(void (*entry_point)(void), size_t kstack_size, size_t ustack_size){
    thread_t *t = (thread_t *)kmalloc(sizeof(thread_t));
    if (!t) return NULL;

    memset(t, 0, sizeof(thread_t));

    void *kernel_stack_mem = kernel_alloc_stack(kstack_size);
    if (!kernel_stack_mem){
        kfree(t);
        return NULL;
    }

    void *user_stack_mem = user_alloc_stack(ustack_size);
    if (!user_stack_mem){
        kfree(kernel_stack_mem);
        kfree(t);
        return NULL;
    }

    serial_print("[THREAD] User Thread Address at %llx\n", (uint64_t)t);
    serial_print("[THREAD] User Thread Kernel Stack Bottom at %llx\n", (uint64_t)user_stack_mem);
    serial_print("[THREAD] User Thread User Stack Bottom at %llx\n", (uint64_t)user_stack_mem);

    t->kernel_stack.bottom = kernel_stack_mem;
    t->kernel_stack.top = kernel_stack_mem + kstack_size;
    t->kernel_stack.size = kstack_size;

    t->user_stack.bottom = user_stack_mem;
    t->user_stack.top = (void *)((uint64_t)user_stack_mem + ustack_size);
    t->user_stack.size = ustack_size;

    t->page_guard_max = (uint64_t)t->user_stack.bottom;
    t->page_guard_min = t->page_guard_max - PAGE_SIZE_4KB;

    uint64_t *stack_top = (uint64_t*)((uint64_t)user_stack_mem);
    stack_top = (uint64_t*)((uint64_t)stack_top & ~15UL);

    uint64_t *clean_stack_top = stack_top;

    stack_top--; *stack_top = 0x1B;
    stack_top--; *stack_top = (uint64_t)clean_stack_top;
    stack_top--; *stack_top = 0x202;
    stack_top--; *stack_top = 0x23;
    stack_top--; *stack_top = (uint64_t)entry_point;

    for (int i = 0; i < 15; i++){
        stack_top--; *stack_top = 0;
    }

    t->rsp = (uint64_t)stack_top;
    t->tid = next_thread_id++;
    t->state = READY;
    t->next = NULL;
    t->prev = NULL;

    enqueue_thread(t);
    return t;
}

void destroy_thread(thread_t *t){
    if (t){
        if (t->kernel_stack.bottom) kfree(t->kernel_stack.bottom);
        if (t->user_stack.bottom) user_free_stack(t->user_stack.bottom);
        kfree(t);
    }
}

void thread_exit(void){
    cli();
    serial_print("[THREAD %d] Done\n", current_thread->tid);
    current_thread->state = DEAD;
    yield();
}

void enqueue_thread(thread_t *t){
    if (!ready_list_head){
        t->next = t;
        t->prev = t;
        ready_list_head = t;
    } else {
        thread_t *tail = ready_list_head->prev;
        tail->next = t;
        t->prev = tail;
        t->next = ready_list_head;
        ready_list_head->prev = t;
    }
}

void dequeue_thread(thread_t *t){
    if (t->next == t) ready_list_head = NULL;
    else {
        t->prev->next = t->next;
        t->next->prev = t->prev;
        if (ready_list_head == t) ready_list_head = t->next;
    } 
    t->next = t->prev = NULL;
}

void cleanup_dead_threads(void){
    cli();

    thread_t *t = dead_list_head;
    dead_list_head = NULL;
    
    sti();

    while (t){
        thread_t *next_dead = t->next;
        destroy_thread(t);
        t = next_dead;
    }
}

void idle_thread_entry(void){
    while (1){
        klog_flush_to_screen();
        cleanup_dead_threads();
        hlt();
    }
}

void third_thread(void){
    
}