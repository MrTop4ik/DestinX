#include <kernel/mutex.h>

void mutex_init(mutex_t *mutex){
    mutex->lock.lock = 0;
    mutex->locked = 0;
    mutex->blocked_head = NULL;
    mutex->blocked_tail = NULL;

    serial_print("[LFB] Mutex was initialized\n");
}

void mutex_lock(mutex_t *mutex){
    uint64_t rflags = spin_lock_irqsave(&mutex->lock);

    if (!mutex->locked){
        mutex->locked = 1;
        spin_lock_irqrestore(&mutex->lock, rflags);
        return;
    }

    thread_t *t = current_thread;
    t->state = BLOCKED;
    t->next_blocked = NULL;

    if (!mutex->blocked_head){
        mutex->blocked_head = t;
        mutex->blocked_tail = t;
    } else {
        mutex->blocked_tail->next_blocked = t;
        mutex->blocked_tail = t;
    }
    
    __atomic_clear(&mutex->lock.lock, __ATOMIC_RELEASE);

    yield();
}

void mutex_unlock(mutex_t *mutex){
    uint64_t rflags = spin_lock_irqsave(&mutex->lock);

    if (!mutex->blocked_head){
        mutex->locked = 0;
        spin_lock_irqrestore(&mutex->lock, rflags);
        return;
    }

    thread_t *wake_thread = mutex->blocked_head;
    mutex->blocked_head = wake_thread->next_blocked;

    if (!mutex->blocked_head) mutex->blocked_tail = NULL;

    wake_thread->state = READY;
    wake_thread->next_blocked = NULL;

    spin_lock_irqrestore(&mutex->lock, rflags);
}