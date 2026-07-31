#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/stack.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/inlineasm.h>
#include <kernel/scheduler/thread.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/syscalls.h>

typedef struct process {
    uint64_t pml4;
    uint64_t pid;
    thread_t *threads;
    us_info_t *ustacks_infos;
} process_t;

process_t *create_user_process(void (*entry_point)(void), size_t kstack_size, size_t ustack_size);