#include <arch/x86_64/apic/acpi.h>
#include <arch/x86_64/apic/ioapic.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/drivers/timers/lapic_timer.h>
#include <arch/x86_64/drivers/timers/pit.h>
#include <arch/x86_64/drivers/keyboard.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/tables/gdt.h>
#include <arch/x86_64/tables/idt.h>
#include <arch/x86_64/utils.h>
#include <arch/x86_64/syscalls.h>
#include <arch/x86_64/utils.h>
#include <drivers/storage/ahci.h>
#include <drivers/video/lfb.h>
#include <drivers/virtio/virtio.h>
#include <drivers/virtio/virtgpu.h>
#include <fs/dfs.h>
#include <kernel/sync/mutex.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/time/time.h>
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

    init_utils();
    init_LFB(physBootInfo);

    parse_acpi(physBootInfo);
    init_lapic();
    init_ioapic();

    init_ahci();

    find_virtio_gpu();
    parse_virtio_gpu_capabilities();
    map_virtio_gpu_capablities();
    init_virtio_gpu_handshake();
    init_virtio_gpu_features();
    configure_and_start_queue();

    init_keyboard();
    init_time();

    init_scheduler();

    init_PIT(1000);
    init_lapic_timer(0x30);

    init_syscalls();

    inode_t *root_inode = dfs_mount_root();

    sti();

    create_user_process("/usr/bin/test.elf");

    for (;;);
}