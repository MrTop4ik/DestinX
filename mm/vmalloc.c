#include <mm/vmalloc.h>

vm_area_t *vmalloc_list_head = NULL;

void *vmalloc(size_t size){
    if (size == 0) return NULL;

    vm_area_t *i = kmalloc(sizeof(vm_area_t));
    if (!i) return NULL;

    i->size = size;

    vm_add_to_list(i);

    for (int j = 0; j < size; j += PAGE_SIZE_4KB) vmm_map_page(read_cr3(), pmm_alloc_page(), (uint64_t)i->base + j, PAGE_SIZE_4KB, PTE_WRITABLE);

    return i->base;
}

void vfree(void *ptr){
    if (!vmalloc_list_head) return; 

    vm_area_t *current = vmalloc_list_head;
    while (current){
        if ((uint64_t)current->base == (uint64_t)ptr){
            for (int i = 0; i < current->size; i += PAGE_SIZE_4KB){
                uint64_t paddr = vmm_unmap_page(read_cr3(), (uint64_t)current->base + i);
                pmm_free_page(paddr);
            }
            vm_remove_from_list(current);
            return;
        }
        current = current->next;
    }
}

void vm_add_to_list(vm_area_t *i){
    if (!vmalloc_list_head){
        i->base = (void*)VMALLOC_START;
        i->prev = NULL;
        i->next = NULL;
        vmalloc_list_head = i;
    } else {
        vm_area_t *current = vmalloc_list_head;
        vm_area_t *next = vmalloc_list_head->next;
        while (next){
            if ((uint64_t)current->base + current->size + i->size <= (uint64_t)next->base){
                i->base = (void *)((uint64_t)current->base + current->size);
                i->next = next;
                i->prev = current;
                current->next = i;
                next->prev = i;
                return;
            }
            current = next;
            next = current->next;
        }
        i->base = (void *)((uint64_t)current->base + current->size);
        i->prev = current;
        i->next = NULL;
        current->next = i;
    }
}

void vm_remove_from_list(vm_area_t *i){
    if (!i) return;

    if (vmalloc_list_head == i) vmalloc_list_head = i->next;
    if (i->next) i->next->prev = i->prev;
    if (i->prev) i->prev->next = i->next;

    i->next = NULL;
    i->prev = NULL;
}