#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/stack.h>
#include <arch/x86_64/tables/idt.h>
#include <arch/x86_64/utils.h>
#include <kernel/scheduler/thread.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/tables/gdt.h>
#include <arch/x86_64/syscalls.h>
#include <kernel/scheduler/process.h>

void init_scheduler(void);
uint64_t scheduler_handler(uint64_t old_rsp);