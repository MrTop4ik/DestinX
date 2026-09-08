#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>

#define DIRTY_FLAG 0x1

typedef struct page_cache {
    uint64_t id;
    uint64_t indx;
    uint64_t addr;
    uint64_t flags;
    struct page_cache *next;
} page_cache_t;

void add_page_to_cache(uint64_t id, uint64_t indx, uint64_t addr);
void remove_page_from_cache(uint64_t id, uint64_t indx);
page_cache_t *get_page_cache(uint64_t id, uint64_t indx);