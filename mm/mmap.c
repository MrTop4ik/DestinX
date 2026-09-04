#include <mm/mmap.h>

vm_area_t *mmap_list_head = NULL;

uint64_t mmap(uint64_t addr, size_t size, uint32_t prot, uint32_t flags, uint64_t fd, uint64_t offset){
    if ((flags & MAP_FIXED) || (flags & MAP_SHARED) || size == 0 || (offset & 0xFFF)) return -EINVAL;

    vm_area_t *i = (vm_area_t *)kmalloc(sizeof(vm_area_t));
    if (!i) return -ENOMEM;

    i->size = (size + PAGE_SIZE_4KB - 1) & PAGE_MASK_4KB;
    i->prot = prot;
    i->flags = flags;
    if (!(flags & MAP_ANONYMOUS) && current_thread->process->fd_table[fd]){
        i->file = current_thread->process->fd_table[fd];
        i->file_pgoff = offset;
    }

    mmap_add_to_list(i);

    current_thread->process->mmap_infos = mmap_list_head;

    return (uint64_t)i->base - i->size;
}

void munmap(void *ptr){
    if (!mmap_list_head) return; 

    vm_area_t *current = mmap_list_head;
    while (current){
        if (((uint64_t)current->base - current->size) == (uint64_t)ptr){
            munmap_by_info(current, current_thread->process->pml4);
            current_thread->process->mmap_infos = mmap_list_head;
            return;
        }
        current = current->next;
    }
}

void munmap_by_info(vm_area_t *i, uint64_t pml4){
    for (int j = 0; j < i->size; j += PAGE_SIZE_4KB){
        uint64_t paddr = vmm_unmap_page(pml4, (uint64_t)i->base - i->size + j);
        pmm_free_page(paddr);
    }
    mmap_remove_from_list(i);
}

void mmap_add_to_list(vm_area_t *i){
    if (!mmap_list_head){
        i->base = (void*)MMAP_START;
        i->prev = NULL;
        i->next = NULL;
        mmap_list_head = i;
    } else {
        vm_area_t *current = mmap_list_head;
        vm_area_t *next = mmap_list_head->next;
        while (next){
            if ((uint64_t)current->base - current->size - i->size >= (uint64_t)next->base){
                i->base = (void *)((uint64_t)current->base - current->size);
                i->next = next;
                i->prev = current;
                current->next = i;
                next->prev = i;
                return;
            }
            current = next;
            next = current->next;
        }
        i->base = (void *)((uint64_t)current->base - current->size);
        i->prev = current;
        i->next = NULL;
        current->next = i;
    }
}

void mmap_remove_from_list(vm_area_t *i){
    if (!i) return;

    if (mmap_list_head == i) mmap_list_head = i->next;
    if (i->next) i->next->prev = i->prev;
    if (i->prev) i->prev->next = i->next;

    i->next = NULL;
    i->prev = NULL;
}