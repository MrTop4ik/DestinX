#include <mm/buddy.h>

free_block_t *free_areas[MAX_ORDERS + 1];
page_metadata_t *metadata = (page_metadata_t *)METADATA_VIRT;
uint64_t metadata_size;
uint64_t current_heap_end;

void init_buddy(void){
    uint64_t pages_needed = (START_PAGES * sizeof(page_metadata_t) + PAGE_SIZE_4KB - 1) / PAGE_SIZE_4KB;
    uint64_t pages = pmm_alloc_pages(pages_needed);
    for (int i = 0; i < pages_needed; i++) vmm_map_page(read_cr3(), pages + (i * PAGE_SIZE_4KB), METADATA_VIRT + (i * PAGE_SIZE_4KB), PAGE_SIZE_4KB, PTE_WRITABLE);

    current_heap_end = HEAP_START + (START_PAGES * PAGE_SIZE_4KB);
    metadata_size = START_PAGES;

    for (int i = 0; i <= MAX_ORDERS; i++) free_areas[i] = NULL;
    
    for (int i = 0; i < START_PAGES; i++){
        metadata[i].is_free = 0;
        metadata[i].order = 0;
        metadata[i].is_slab = 0;
    }

    buddy_list_add(HEAP_START, MAX_ORDERS);
}

void *buddy_alloc(int order){
    for (int i = order; i <= MAX_ORDERS; i++){
        if (free_areas[i]){
            uint64_t block_addr = (uint64_t)free_areas[i];

            buddy_list_remove(block_addr, i);

            while (i > order){
                i--;
                uint64_t buddy_addr = block_addr ^ ((1ULL << i) * PAGE_SIZE_4KB);
                buddy_list_add(buddy_addr, i);
            }

            int index = (block_addr - HEAP_START) / PAGE_SIZE_4KB;
            metadata[index].order = order;
            metadata[index].is_free = 0;
            metadata[index].is_slab = 0;

            return (void *)block_addr;
        }
    }
    
    uint64_t pages_needed = (START_PAGES * sizeof(page_metadata_t) + PAGE_SIZE_4KB - 1) / PAGE_SIZE_4KB;
    uint64_t pages = pmm_alloc_pages(pages_needed);
    for (int i = 0; i < pages_needed; i++) vmm_map_page(read_cr3(), pages + (i * PAGE_SIZE_4KB), METADATA_VIRT + metadata_size + (i * PAGE_SIZE_4KB), PAGE_SIZE_4KB, PTE_WRITABLE);
    metadata_size += pages_needed * PAGE_SIZE_4KB;
    
    pages = pmm_alloc_pages(START_PAGES);
    for (int i = 0; i < START_PAGES; i++){
        vmm_map_page(read_cr3(), pages + (i * PAGE_SIZE_4KB), current_heap_end + (i * PAGE_SIZE_4KB), PAGE_SIZE_4KB, PTE_WRITABLE);
        uint64_t indx = (current_heap_end + (i * PAGE_SIZE_4KB) - HEAP_START) / PAGE_SIZE_4KB;
        metadata[indx].order = MAX_ORDERS;
        metadata[indx].is_free = 0;
        metadata[indx].is_slab = 0;
    }

    buddy_list_add(current_heap_end, MAX_ORDERS);
    current_heap_end += START_PAGES;

    void *addr = buddy_alloc(order);
    if (!addr) return NULL;

    return addr;
}

void buddy_free(void *ptr){
    uint64_t addr = (uint64_t)ptr;
    int index = (addr - HEAP_START) / PAGE_SIZE_4KB;
    
    int i = metadata[index].order;
    for(; i < MAX_ORDERS; i++){
        uint64_t buddy_addr = addr ^ ((1ULL << i) * PAGE_SIZE_4KB);
        int buddy_index = (buddy_addr - HEAP_START) / PAGE_SIZE_4KB;

        if (metadata[buddy_index].is_free && metadata[buddy_index].order == i){
            buddy_list_remove(buddy_addr, i);

            if (buddy_addr < addr){
                addr = buddy_addr;
            }
        } else {
            break;
        }
    }

    buddy_list_add(addr, i);
}

int is_buddy_free(uint64_t buddy_addr, int order){
    int index = (buddy_addr - HEAP_START) / PAGE_SIZE_4KB;
    return metadata[index].order == order && metadata[index].is_free;
}

void buddy_list_remove(uint64_t addr, int order){
    free_block_t *block = (free_block_t *)addr;
    if (block->prev) block->prev->next = block->next;
    else free_areas[order] = block->next;
    if (block->next) block->next->prev = block->prev;

    int index = (addr - HEAP_START) / PAGE_SIZE_4KB;
    metadata[index].is_free = 0;
}

void buddy_list_add(uint64_t addr, int order){
    free_block_t *block = (free_block_t *)addr;
    block->prev = NULL;
    block->next = free_areas[order];
    if (free_areas[order]) free_areas[order]->prev = block;

    free_areas[order] = block;

    int index = (addr - HEAP_START) / PAGE_SIZE_4KB;
    metadata[index].is_free = 1;
    metadata[index].is_slab = 0;
    metadata[index].order = order;
}