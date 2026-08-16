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

#define KRING_BUF_MAX 4096
#define KRING_BUF_MASK (KRING_BUF_MAX - 1)

typedef struct {
    char c;
    volatile uint8_t ready;
} log_entry_t;

void init_kring(void);
void kring_write(const char *s, size_t len);
void kring_flush_to_screen(void);