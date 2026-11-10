#pragma once
#include <stdint.h>
#include <mm/vmalloc.h>
#include <arch/x86_64/drivers/serial.h>
#include <drivers/video/lfb.h>
#include <arch/x86_64/tables/idt.h>
#include <kernel/scheduler/thread.h>
#include <kernel/scheduler/process.h>
#include <mm/mmap.h>
#include <fs/dfs.h>

void page_fault_handler(struct InterruptRegisters *regs);