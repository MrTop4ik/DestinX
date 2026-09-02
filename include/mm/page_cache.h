#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>

typedef struct page_cache {
    uint64_t id;
    uint64_t indx;
    uint64_t addr;
    struct page_cache *next;
} page_cache_t;

void add_page_to_cache(uint64_t id, uint64_t indx, uint64_t addr);
void remove_page_from_cache(uint64_t id, uint64_t indx);
uint64_t get_page_addr(uint64_t id, uint64_t indx);