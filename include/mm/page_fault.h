#pragma once
#include <stdint.h>
#include <mm/vmalloc.h>
#include <arch/x86_64/drivers/video/serial.h>
#include <drivers/lfb.h>
#include <arch/x86_64/idt.h>
#include <kernel/scheduler/thread.h>

void page_fault_handler(struct InterruptRegisters *regs);