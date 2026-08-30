#include <mm/stack.h>

uint64_t user_next_top = USER_STACK_MAX;
vm_area_t *us_list_head = NULL;

void *kernel_alloc_stack(size_t size){
    if (size == 0) return 0;

    void *stack_bottom = vmalloc(size);
    if (!stack_bottom) return NULL;

    return stack_bottom;
}

void *user_alloc_stack(size_t size){
    if (size == 0) return NULL;

    vm_area_t *i = (vm_area_t *)kmalloc(sizeof(vm_area_t));
    if (!i) return NULL;

    i->size = size;
    
    us_add_to_list(i);
    
    vmm_map_page(read_cr3(), pmm_alloc_page(), (uint64_t)i->base - PAGE_SIZE_4KB, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));

    return i->base;
}

void user_free_stack(void *ptr){
    if (!us_list_head) return; 

    vm_area_t *current = us_list_head;
    while (current){
        if ((uint64_t)current->base == (uint64_t)ptr){
            for (int i = 0; i < current->size; i += PAGE_SIZE_4KB){
                uint64_t paddr = vmm_unmap_page(read_cr3(), (uint64_t)current->base - current->size + i);
                pmm_free_page(paddr);
            }
            us_remove_from_list(current);
            return;
        }
        current = current->next;
    }
}

void us_add_to_list(vm_area_t *i){
    if (!us_list_head){
        i->base = (void*)USER_STACK_MAX;
        i->prev = NULL;
        i->next = NULL;
        us_list_head = i;
    } else {
        vm_area_t *current = us_list_head;
        vm_area_t *next = us_list_head->next;
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

void us_remove_from_list(vm_area_t *i){
    if (!i) return;

    if (us_list_head == i) us_list_head = i->next;
    if (i->next) i->next->prev = i->prev;
    if (i->prev) i->prev->next = i->next;

    i->next = NULL;
    i->prev = NULL;
}