#include <mm/vmalloc.h>

vm_info_t *vmalloc_list_head = NULL;

void *vmalloc(size_t size){
    if (size == 0) return NULL;

    vm_info_t *i = kmalloc(sizeof(vm_info_t));
    if (!i) return NULL;

    i->size = size;

    if (!vmalloc_list_head){
        i->base = (void*)VMALLOC_START;
        vm_add_to_list(i);
    } else {
        vm_info_t *current = vmalloc_list_head;
        vm_info_t *next = vmalloc_list_head->next;
        while (1){
            if ((!next) || ((uint64_t)current->base + current->size + size <= (uint64_t)next->base)){
                i->base = (void*)((uint64_t)current->base + current->size);
                vm_add_to_list(i);
                break;
            }

            vm_info_t *old_next = next;
            current = old_next;
            next = current->next;
        }
    }

    for (int j = 0; j < size; j += PAGE_SIZE_4KB) vmm_map_page(read_cr3(), pmm_alloc_page(), (uint64_t)i->base + j, PAGE_SIZE_4KB, PTE_WRITABLE);

    return i->base;
}

void vfree(void *ptr){
    if (!vmalloc_list_head) return; 

    vm_info_t *current = vmalloc_list_head;
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

void vm_add_to_list(vm_info_t *i){
    if (vmalloc_list_head){
        if (!vmalloc_list_head->next){
            if ((uint64_t)i->base > (uint64_t)vmalloc_list_head->base){ 
                vmalloc_list_head->next = i;
                i->next = NULL;
                i->prev = vmalloc_list_head;
                return;
            } else {
                i->next = vmalloc_list_head;
                i->prev = NULL;
                vmalloc_list_head->prev = i;
                vmalloc_list_head = i;
                return;
            }
        }
        vm_info_t *current = vmalloc_list_head;
        vm_info_t *next = vmalloc_list_head->next;
        if ((uint64_t)i->base < (uint64_t)vmalloc_list_head->base){
            i->next = vmalloc_list_head;
            i->prev = NULL;
            vmalloc_list_head->prev = i;
            vmalloc_list_head = i;
            return;
        }
        while (1){
            if (!next){
                current->next = i;
                i->prev = current;
                i->next = NULL;
                return;
            }
            if (((uint64_t)current->base < (uint64_t)i->base) && ((uint64_t)next->base > (uint64_t)i->base)){
                current->next = i;
                i->next = next;
                i->prev = current;
                next->prev = i;
                return;
            }
            vm_info_t *old_next = next;
            current = old_next;
            next = current->next;
        }
    } else {
        vmalloc_list_head = i;
        vmalloc_list_head->next = NULL;
        vmalloc_list_head->prev = NULL;
    }
}

void vm_remove_from_list(vm_info_t *i){
    if (!i) return;

    if (vmalloc_list_head == i) {
        vmalloc_list_head = i->next;
    }

    if (i->next) {
        i->next->prev = i->prev;
    }

    if (i->prev) {
        i->prev->next = i->next;
    }

    i->next = NULL;
    i->prev = NULL;
}