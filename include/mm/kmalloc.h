#pragma once
#include <stdint.h>
#include <arch/x86_64/drivers/serial.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/buddy.h>
#include <mm/slab.h>
#include <kernel/spinlock.h>

#define HEAP_START 0xFFFFE00000000000
#define HEAP_SIZE  0x1000000

extern volatile int scheduler;

void init_kheap(void);
void *kmalloc(size_t size);
void kfree(void *ptr);