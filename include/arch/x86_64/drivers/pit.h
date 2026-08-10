#pragma once
#include <stdint.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/inlineasm.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/apic/ioapic.h>

void init_PIT(uint32_t freq);
void PIT_handler(struct InterruptRegisters *regs);
void pit_wait_ms(uint32_t ms);