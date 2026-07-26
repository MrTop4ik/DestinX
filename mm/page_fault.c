#include <mm/page_fault.h>

void page_fault_handler(struct InterruptRegisters *regs){
    uint64_t fault_address;
    __asm__ volatile ("mov %%cr2, %0" : "=r"(fault_address) : : "memory");

    if (current_thread->page_guard_max > fault_address && current_thread->page_guard_min <= fault_address){
        if (current_thread->page_guard_max != current_thread->user_stack.bottom){
            uint64_t paddr = pmm_alloc_page();
            vmm_map_page(read_cr3(), paddr, current_thread->page_guard_min, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));

            current_thread->page_guard_max -= PAGE_SIZE_4KB;
            current_thread->page_guard_min -= PAGE_SIZE_4KB;

            serial_print("[ISR] User Stack Expand");
            serial_print("\n");

            return;
        } else {
            serial_print("[ISR] User Stack Overflow");
            serial_print("\n");
            thread_exit();
        }
    }
    for (;;);
}