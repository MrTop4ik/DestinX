CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel -Iinclude
AFLAGS = -f elf64
QEMUFLAGS = -d int -D logs/qemu.log -m 4096 -machine q35,acpi=on,smm=off -serial stdio -display gtk -drive id=disk,file=disk.img,format=raw,if=none -device ide-hd,drive=disk,bus=ide.0

iso:
	nasm 				$(AFLAGS) 		   arch/x86_64/boot/boot.s						-o boot.o
	nasm				$(AFLAGS)		   arch/x86_64/gdt/gdt.s						-o gdt.o
	nasm				$(AFLAGS)		   arch/x86_64/idt/idt.s						-o idt.o
	nasm				$(AFLAGS)		   arch/x86_64/drivers/timers/lapic_timer.s		-o lapic_timer.o
	nasm 				$(AFLAGS) 		   arch/x86_64/context/yield.s					-o yield.o
	nasm				$(AFLAGS)		   arch/x86_64/syscalls/syscalls.s				-o syscall.o
	x86_64-elf-gcc 		$(CFLAGS)		-c kernel/kernel.c								-o kernel.o
	x86_64-elf-gcc		$(CFLAGS)		-c arch/x86_64/gdt/gdt.c						-o gdts.o
	x86_64-elf-gcc		$(CFLAGS)		-c arch/x86_64/idt/idt.c						-o idts.o
	x86_64-elf-gcc 		$(CFLAGS)		-c arch/x86_64/inlineasm.c						-o inlineasm.o
	x86_64-elf-gcc		$(CFLAGS)		-c arch/x86_64/drivers/timers/pit.c				-o pit.o
	x86_64-elf-gcc		$(CFLAGS)		-c libc/string.c								-o string.o
	x86_64-elf-gcc		$(CFLAGS)		-c arch/x86_64/drivers/video//serial.c			-o serial.o
	x86_64-elf-gcc		$(CFLAGS)		-c drivers/lfb/lfb.c							-o lfb.o
	x86_64-elf-gcc		$(CFLAGS)		-c mm/pmm.c 									-o pmm.o
	x86_64-elf-gcc		$(CFLAGS)		-c mm/vmm.c										-o vmm.o
	x86_64-elf-gcc		$(CFLAGS)		-c mm/buddy.c									-o buddy.o
	x86_64-elf-gcc		$(CFLAGS)		-c mm/slab.c									-o slab.o
	x86_64-elf-gcc		$(CFLAGS)		-c mm/kmalloc.c									-o kmalloc.o
	x86_64-elf-gcc		$(CFLAGS)		-c mm/vmalloc.c									-o vmalloc.o
	x86_64-elf-gcc		$(CFLAGS)		-c mm/stack.c									-o stack.o
	x86_64-elf-gcc		$(CFLAGS)		-c arch/x86_64/apic/acpi.c						-o acpi.o
	x86_64-elf-gcc 		$(CFLAGS)		-c arch/x86_64/apic/lapic.c						-o lapic.o
	x86_64-elf-gcc 		$(CFLAGS)		-c arch/x86_64/apic/ioapic.c					-o ioapic.o
	x86_64-elf-gcc		$(CFLAGS)		-c arch/x86_64/drivers/timers/lapic_timer.c		-o lapic_timers.o
	x86_64-elf-gcc		$(CFLAGS)		-c kernel/scheduler/thread.c					-o thread.o
	x86_64-elf-gcc		$(CFLAGS)		-c kernel/scheduler/scheduler.c					-o scheduler.o
	x86_64-elf-gcc		$(CFLAGS)		-c kernel/spinlock.c							-o spinlock.o
	x86_64-elf-gcc		$(CFLAGS)		-c drivers/lfb/klog.c							-o klog.o
	x86_64-elf-gcc		$(CFLAGS)		-c kernel/mutex.c								-o mutex.o
	x86_64-elf-gcc		$(CFLAGS)		-c arch/x86_64/syscalls/syscalls.c				-o syscalls.o
	x86_64-elf-gcc		$(CFLAGS)		-c mm/page_fault.c								-o page_fault.o
	x86_64-elf-gcc		$(CFLAGS)		-c kernel/scheduler/process.c					-o process.o
	x86_64-elf-gcc		$(CFLAGS)		-c arch/x86_64/drivers/pci/pci.c				-o pci.o
	x86_64-elf-gcc		$(CFLAGS)		-c drivers/storage/ahci.c						-o ahci.o
	x86_64-elf-gcc -T arch/x86_64/boot/linker.ld -o kernel.bin -ffreestanding -O2 -nostdlib -lgcc boot.o kernel.o inlineasm.o serial.o gdt.o gdts.o idt.o idts.o pit.o string.o pmm.o vmm.o lfb.o kmalloc.o buddy.o slab.o acpi.o lapic.o ioapic.o lapic_timer.o lapic_timers.o thread.o yield.o scheduler.o stack.o spinlock.o klog.o mutex.o vmalloc.o syscalls.o syscall.o page_fault.o process.o pci.o ahci.o
	grub-file --is-x86-multiboot2 kernel.bin
	mv kernel.bin isodir/boot/kernel.bin
	grub-mkrescue -o kernel.iso isodir
	rm *.o

disk:
	mkfs.dfs disk.img 32
	dfs-tools disk.img mkdir /usr
	dfs-tools disk.img mkdir /usr/bin
	dfs-tools disk.img add files/test.txt /test.txt


run: kernel.iso
	qemu-system-x86_64 -cdrom kernel.iso $(QEMUFLAGS)