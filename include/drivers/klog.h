#pragma once
#include <stdint.h>
#include <stddef.h>
#include <multiboot2.h>
#include <arch/x86_64/inlineasm.h>
#include <libc/string.h>
#include <arch/x86_64/drivers/serial.h>
#include <kernel/spinlock.h>
#include <mm/kmalloc.h>
#include <drivers/lfb.h>

#define KLOG_BUF_MAX 4096
#define KLOG_BUF_MASK (KLOG_BUF_MAX - 1)

typedef struct {
    char c;
    volatile uint8_t ready;
} log_entry_t;

void init_klog(void);
void klog_write(const char *s, size_t len);
void klog_flush_to_screen(void);