#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/inlineasm.h>
#include <arch/x86_64/apic/ioapic.h>
#include <drivers/kring.h>
#include <arch/x86_64/idt.h>

void keyboard_handler(struct InterruptRegisters *regs);
void init_keyboard(void);