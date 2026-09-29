#pragma once
#include <stdint.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/drivers/pit.h>
#include <drivers/lfb.h>
#include <arch/x86_64/inlineasm.h>
#include <mm/kmalloc.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/apic/ioapic.h>

#define LAPIC_TIMER_LVT 0x320
#define LAPIC_TIMER_INITCNT 0x380
#define LAPIC_TIMER_CURRCNT 0x390
#define LAPIC_TIMER_DIV 0x3E0

#define LAPIC_TIMER_MASK            (1 << 16)
#define LAPIC_TIMER_PERIODIC        (1 << 17)
#define LAPIC_TIMER_TSC_DEADLINE    (1 << 18)
#define LAPIC_TIMER_ONESHOT         0x0

void init_lapic_timer(uint8_t vector);
void lapic_timer_reload_oneshot(uint64_t ms);