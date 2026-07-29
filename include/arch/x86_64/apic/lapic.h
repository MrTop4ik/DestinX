#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/apic/acpi.h>
#include <mm/kmalloc.h>
#include <arch/x86_64/inlineasm.h>

#define LAPIC_VER_OFFSET 0x30
#define LAPIC_ID_OFFSET 0x20
#define LAPIC_ICR_LOW_OFFSET 0x300
#define LAPIC_ICR_HIGH_OFFSET 0x310
#define LAPIC_EOI_OFFSET 0xB0
#define LAPIC_SVR_OFFSET 0xF0
#define LAPIC_TPR_OFFSET 0x80

#define IA32_APIC_MSR 0x1B

#define LAPIC_SVR_ENABLE (1ULL << 8)
#define IA32_APIC_ENABLE (1ULL << 11)

#define SPURIOUS_VECTOR 0xFF

void init_LAPIC(void);
void lapic_eoi(void);
uint32_t read_lapic(uint32_t reg);
void write_lapic(uint32_t reg, uint32_t val);