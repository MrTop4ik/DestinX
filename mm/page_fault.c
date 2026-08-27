#include <mm/page_fault.h>

extern volatile int scheduler;

void page_fault_handler(struct InterruptRegisters *regs){
    if (scheduler){
        uint64_t fault_address;
        __asm__ volatile ("mov %%cr2, %0" : "=r"(fault_address) : : "memory");

        if (current_thread->page_guard_max > fault_address && current_thread->page_guard_min <= fault_address){
            if (current_thread->page_guard_max != current_thread->user_stack.bottom){
                uint64_t paddr = pmm_alloc_page();
                vmm_map_page(read_cr3(), paddr, current_thread->page_guard_min, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));

                current_thread->page_guard_max -= PAGE_SIZE_4KB;
                current_thread->page_guard_min -= PAGE_SIZE_4KB;

                serial_print("[THREAD %d] User Stack Expand\n", current_thread->tid);
                serial_print("\n");
                return;
            } else {
                serial_print("[THREAD %d] User Stack Overflow\n", current_thread->tid);
                serial_print("\n");
                thread_exit();
            }
        } else if (current_thread->process->heap_start <= fault_address && fault_address < current_thread->process->current_heap_end){
            vmm_map_page(read_cr3(), pmm_alloc_page(), fault_address & PAGE_MASK_4KB, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));
            serial_print("[THREAD %d] User Heap Expand\n", current_thread->tid);
            return;
        }
    }
    for (;;);
}