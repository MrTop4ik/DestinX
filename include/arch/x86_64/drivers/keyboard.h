#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/utils.h>
#include <arch/x86_64/apic/ioapic.h>
#include <drivers/video/kring.h>
#include <arch/x86_64/tables/idt.h>

void keyboard_handler(struct InterruptRegisters *regs);
void init_keyboard(void);