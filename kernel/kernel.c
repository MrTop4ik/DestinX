#include <stdint.h>
#include <multiboot2.h>
#include <arch/x86_64/drivers/serial.h>
#include <drivers/lfb.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/drivers/pit.h>
#include <arch/x86_64/inlineasm.h>
#include <mm/kmalloc.h>
#include <arch/x86_64/apic/acpi.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/apic/ioapic.h>
#include <arch/x86_64/drivers/lapic_timer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/mutex.h>
#include <mm/vmalloc.h>
#include <arch/x86_64/syscalls.h>
#include <drivers/ahci.h>

void kernel_main(uint64_t magic, unsigned int physBootInfo){
    serial_init();
    init_GDT();
    init_IDT();

    init_PMM(physBootInfo);
    init_VMM(physBootInfo);
    
    init_kheap();

    init_LFB(physBootInfo);

    parse_acpi(physBootInfo);
    init_LAPIC();

    init_ahci();

    init_scheduler();

    init_PIT(10);
    init_lapic_timer(0x20, 1);

    init_syscalls();

    sti();

    for (;;);
}