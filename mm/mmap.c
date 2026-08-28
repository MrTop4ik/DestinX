#include <mm/mmap.h>

vm_info_t *mmap_list_head = NULL;

uint64_t mmap(size_t size){
    if (size == 0) return 0;

    vm_info_t *i = kmalloc(sizeof(vm_info_t));
    if (!i) return 0;

    i->size = size;

    mmap_add_to_list(i);

    for (int j = 0; j < size; j += PAGE_SIZE_4KB) vmm_map_page(read_cr3(), pmm_alloc_page(), (uint64_t)i->base - size + j, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));

    return ((uint64_t)i->base - size);
}

void munmap(void *ptr){
    if (!mmap_list_head) return; 

    vm_info_t *current = mmap_list_head;
    while (current){
        if (((uint64_t)current->base - current->size) == (uint64_t)ptr){
            munmap_by_info(current, current_thread->process->pml4);
            mmap_remove_from_list(current);
            return;
        }
        current = current->next;
    }
}

void munmap_by_info(vm_info_t *i, uint64_t pml4){
    for (int j = 0; j < i->size; j += PAGE_SIZE_4KB){
        uint64_t paddr = vmm_unmap_page(pml4, (uint64_t)i->base - i->size + j);
        pmm_free_page(paddr);
    }
    mmap_remove_from_list(i);
}

void mmap_add_to_list(vm_info_t *i){
    if (!mmap_list_head){
        i->base = (void*)MMAP_START;
        i->prev = NULL;
        i->next = NULL;
        mmap_list_head = i;
    } else {
        vm_info_t *current = mmap_list_head;
        vm_info_t *next = mmap_list_head->next;
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

void mmap_remove_from_list(vm_info_t *i){
    if (!i) return;

    if (mmap_list_head == i) mmap_list_head = i->next;
    if (i->next) i->next->prev = i->prev;
    if (i->prev) i->prev->next = i->next;

    i->next = NULL;
    i->prev = NULL;
}