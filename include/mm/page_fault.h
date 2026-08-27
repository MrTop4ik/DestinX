#pragma once
#include <stdint.h>
#include <mm/vmalloc.h>
#include <arch/x86_64/drivers/serial.h>
#include <drivers/lfb.h>
#include <arch/x86_64/idt.h>
#include <kernel/scheduler/thread.h>
#include <kernel/scheduler/process.h>

void page_fault_handler(struct InterruptRegisters *regs);