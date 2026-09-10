#pragma once
#include <stdint.h>
#include <arch/x86_64/inlineasm.h>

typedef struct {
    volatile int lock;
} spinlock_t;

uint64_t spin_lock_irqsave(spinlock_t *lock);
void spin_lock_irqrestore(spinlock_t *lock, uint64_t rflags);