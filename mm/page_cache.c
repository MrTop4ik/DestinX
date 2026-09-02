#include <mm/page_cache.h>

page_cache_t *cache_list = NULL;

void add_page_to_cache(uint64_t id, uint64_t indx, uint64_t addr){
    page_cache_t *cp = (page_cache_t *)kmalloc(sizeof(page_cache_t));

    cp->id = id;
    cp->indx = indx;
    cp->addr = addr;

    cp->next = cache_list;
    cache_list = cp;
}

void remove_page_from_cache(uint64_t id, uint64_t indx){
    page_cache_t *prev = NULL;
    page_cache_t *cur = cache_list;

    while (cur){
        if (cur->id == id && cur->indx == indx){
            if (prev) prev->next = cur;
            if (cur == cache_list) cache_list = cur->next;
            pmm_free_page(cur->addr);
            kfree(cur);
            return;
        }

        prev = cur;
        cur = cur->next;
    }
}

uint64_t get_page_addr(uint64_t id, uint64_t indx){
    page_cache_t *prev = NULL;
    page_cache_t *cur = cache_list;

    while (cur){
        if (cur->id == id && cur->indx == indx) return cur->addr;

        prev = cur;
        cur = cur->next;
    }
}