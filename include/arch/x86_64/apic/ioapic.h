#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/apic/acpi.h>
#include <mm/kmalloc.h>
#include <arch/x86_64/inlineasm.h>

#define IOAPIC_VIRT 0xffffffffffffb000

#define IOAPIC_REGSEL_OFFSET 0x0
#define IOAPIC_IOWIN_OFFSET 0x10
#define IOAPIC_REDTBL_OFFSET 0x10

void init_ioapic(void);
uint32_t get_gsi_for_irq(uint8_t irq);
void ioapic_set_irq(uint8_t irq, uint8_t vector, uint8_t cpu_id);
uint32_t read_ioapic(uint32_t reg);
void write_ioapic(uint32_t reg, uint32_t data);