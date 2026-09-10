#include <kernel/sync/spinlock.h>

uint64_t spin_lock_irqsave(spinlock_t *lock){
    uint64_t rflags = read_rflags();
    cli();
    while (__atomic_test_and_set(&lock->lock, __ATOMIC_ACQUIRE)) __asm__ volatile ("pause");

    return rflags;
}

void spin_lock_irqrestore(spinlock_t *lock, uint64_t rflags){
    __atomic_clear(&lock->lock, __ATOMIC_RELEASE);
    write_rflags(rflags);
}