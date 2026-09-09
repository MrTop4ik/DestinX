#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>

#define DIRTY_FLAG          (1 << 0)
#define WRITEBACK_FLAG      (1 << 1)

typedef struct page_cache {
    uint64_t id;
    uint64_t indx;
    uint64_t addr;
    uint64_t flags;
    struct page_cache *next;
} page_cache_t;

extern page_cache_t *cache_list;

void add_page_to_cache(uint64_t id, uint64_t indx, uint64_t addr);
void remove_page_from_cache(uint64_t id, uint64_t indx);
page_cache_t *get_page_cache(uint64_t id, uint64_t indx);