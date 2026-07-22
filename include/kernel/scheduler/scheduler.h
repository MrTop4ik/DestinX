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

void init_scheduler(void);
uint64_t scheduler_handler(uint64_t old_rsp);