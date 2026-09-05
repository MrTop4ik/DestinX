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
                return;
            } else {
                serial_print("[THREAD %d] User Stack Overflow\n", current_thread->tid);
                thread_exit();
            }
        } else if (current_thread->process->heap_start <= fault_address && fault_address < current_thread->process->current_heap_end){
            vmm_map_page(read_cr3(), pmm_alloc_page(), fault_address & PAGE_MASK_4KB, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));
            current_thread->process->pages_alloced++;
            serial_print("[THREAD %d] User Heap Expand (Pages Allocated: %d)\n", current_thread->tid, current_thread->process->pages_alloced);
            return;
        }

        vm_area_t *cur = current_thread->process->mmap_infos;
        while (cur){
            if ((fault_address >= (uint64_t)cur->base - cur->size) && (fault_address < (uint64_t)cur->base)){
                uint64_t flags = 0;
                flags |= PTE_USER;
                if (cur->flags & PROT_WRITE) flags |= PTE_WRITABLE;
                if (cur->flags & MAP_ANONYMOUS) vmm_map_page(read_cr3(), pmm_alloc_page(), fault_address & PAGE_MASK_4KB, PAGE_SIZE_4KB, flags);
                else if (cur->flags & MAP_PRIVATE){
                    uint64_t paddr = pmm_alloc_page();
                    uint64_t bytes_read = dfs_read(cur->file->fp, (uint8_t*)(paddr + DIRECT_OFFSET), cur->file_pgoff + (fault_address & PAGE_MASK_4KB) - (uint64_t)cur->base + cur->size, PAGE_SIZE_4KB);
                    vmm_map_page(read_cr3(), paddr, fault_address & PAGE_MASK_4KB, PAGE_SIZE_4KB, flags);
                } else if (cur->flags & MAP_SHARED){
                    inode_t *inode = (inode_t *)cur->file->private_data;
                    uint64_t paddr = get_page_addr(inode->inode_num, (cur->file_pgoff + (fault_address & PAGE_MASK_4KB) - (uint64_t)cur->base + cur->size) / PAGE_SIZE_4KB);
                    if (!paddr){
                        paddr = pmm_alloc_page();
                        int status = ahci_read(&ahci_regs->ports[0], (cur->file_pgoff + (fault_address & PAGE_MASK_4KB) - (uint64_t)cur->base + cur->size) / 512 + inode->extent.start_block * 8, 8, &paddr, 1);
                        if (status != 0){
                            pmm_free_page(paddr);
                            serial_print("[THREAD %d] Couldn't Expand MMAP Area\n", current_thread->tid);
                            return;
                        }
                        add_page_to_cache(inode->inode_num, (cur->file_pgoff + (fault_address & PAGE_MASK_4KB) - (uint64_t)cur->base + cur->size) / PAGE_SIZE_4KB, paddr);
                    }
                    vmm_map_page(read_cr3(), paddr, fault_address & PAGE_MASK_4KB, PAGE_SIZE_4KB, flags);
                }
                serial_print("[THREAD %d] MMAP Area Expand\n", current_thread->tid);
                return;
            }
            cur = cur->next;
        }
    }
    serial_print("[ISR] ");
    serial_print(exceptions[regs->int_no]);
    serial_print("\n");
    for (;;);
}