#pragma once
#include <stdint.h>
#include <arch/x86_64/drivers/serial.h>
#include <drivers/lfb.h>
#include <arch/x86_64/inlineasm.h>
#include <mm/kmalloc.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/spinlock.h>

typedef struct {
    spinlock_t lock;
    int locked;
    thread_t *blocked_head;
    thread_t *blocked_tail;
} mutex_t;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);