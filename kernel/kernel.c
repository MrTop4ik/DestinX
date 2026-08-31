#include <arch/x86_64/apic/acpi.h>
#include <arch/x86_64/apic/ioapic.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/drivers/lapic_timer.h>
#include <arch/x86_64/drivers/pit.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/inlineasm.h>
#include <arch/x86_64/syscalls.h>
#include <drivers/ahci.h>
#include <drivers/lfb.h>
#include <fs/dfs.h>
#include <kernel/mutex.h>
#include <kernel/scheduler/scheduler.h>
#include <mm/kmalloc.h>
#include <mm/vmalloc.h>
#include <multiboot2.h>
#include <stdint.h>

void kernel_main(uint64_t magic, unsigned int physBootInfo){
    serial_init();
    init_GDT();
    init_IDT();

    init_PMM(physBootInfo);
    init_VMM(physBootInfo);

    init_kheap();

    init_LFB(physBootInfo);

    parse_acpi(physBootInfo);
    init_lapic();
    init_ioapic();

    init_ahci();

    init_scheduler();

    init_PIT(10);
    init_lapic_timer(0x30, 1);

    init_syscalls();

    inode_t *root_inode = dfs_mount_root();

    sti();

    inode_t *inode = dfs_get_inode("/usr/txt/test.txt");
    
    uint8_t *buffer = kmalloc(inode->size);
    char buf[13] = "!dlroW ,olleH";

    dfs_write("/usr/txt/test.txt", buf, 0, inode->size);

    dfs_read("/usr/txt/test.txt", buffer, 0, inode->size);

    kfree(buffer);

    create_user_process("/usr/bin/test.elf");

    for (;;);
}