#include <mm/stack.h>

uint64_t user_next_top = USER_STACK_MAX;
us_info_t *us_list_head = NULL;

void *kernel_alloc_stack(size_t size){
    if (size == 0) return 0;

    void *stack_bottom = kmalloc(size);
    if (!stack_bottom) return NULL;

    return stack_bottom;
}

void *user_alloc_stack(size_t size){
    if (size == 0) return NULL;

    us_info_t *i = (us_info_t *)vmalloc(sizeof(us_info_t));
    if (!i) return NULL;

    i->size = size;

    if (!us_list_head){
        i->base = (void*)USER_STACK_MAX;
        us_add_to_list(i);
    } else {
        us_info_t *current = us_list_head;
        us_info_t *next = us_list_head->next;
        while (1){
            if ((!next) || ((uint64_t)current->base - current->size - size >= (uint64_t)next->base)){
                i->base = (void*)((uint64_t)current->base + current->size);
                us_add_to_list(i);
                break;
            }

            us_info_t *old_next = next;
            current = old_next;
            next = current->next;
        }
    }

    

    for (int j = 0; j < size; j += PAGE_SIZE_4KB) vmm_map_page(read_cr3(), pmm_alloc_page(), (uint64_t)i->base - j, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));

    return i->base;
}

void user_free_stack(void *ptr){
    if (!us_list_head) return; 

    us_info_t *current = us_list_head;
    while (current){
        if ((uint64_t)current->base == (uint64_t)ptr){
            for (int i = 0; i < current->size; i += PAGE_SIZE_4KB){
                uint64_t paddr = vmm_unmap_page(read_cr3(), (uint64_t)current->base - i);
                pmm_free_page(paddr);
            }
            us_remove_from_list(current);
            return;
        }
        current = current->next;
    }
}

void us_add_to_list(us_info_t *i){
    if (us_list_head){
        if (!us_list_head->next){
            if ((uint64_t)i->base < (uint64_t)us_list_head->base){ 
                us_list_head->next = i;
                i->next = NULL;
                i->prev = us_list_head;
                return;
            } else {
                i->next = us_list_head;
                i->prev = NULL;
                us_list_head->prev = i;
                us_list_head = i;
                return;
            }
        }
        us_info_t *current = us_list_head;
        us_info_t *next = us_list_head->next;
        if ((uint64_t)i->base > (uint64_t)us_list_head->base){
            i->next = us_list_head;
            i->prev = NULL;
            us_list_head->prev = i;
            us_list_head = i;
            return;
        }
        while (1){
            if (!next){
                current->next = i;
                i->prev = current;
                i->next = NULL;
                return;
            }
            if (((uint64_t)current->base > (uint64_t)i->base) && ((uint64_t)next->base < (uint64_t)i->base)){
                current->next = i;
                i->next = next;
                i->prev = current;
                next->prev = i;
                return;
            }
            us_info_t *old_next = next;
            current = old_next;
            next = current->next;
        }
    } else {
        us_list_head = i;
        i->next = NULL;
        i->prev = NULL;
    }
}

void us_remove_from_list(us_info_t *i){
    if (!i) return;

    if (us_list_head == i) {
        us_list_head = i->next;
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