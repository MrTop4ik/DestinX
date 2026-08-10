#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/drivers/serial.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/buddy.h>

#define NUM_CACHES 8

struct slab;
struct kmem_cache;

typedef struct slab {
    void *free_list;
    uint32_t free_count;
    uint32_t total_count;
    struct kmem_cache *parent_cache;
    struct slab *prev;
    struct slab *next;    
} slab_t;

typedef struct kmem_cache {
    uint32_t obj_size;
    struct slab *slabs_partial;
    struct slab *slabs_full;
} kmem_cache_t;

extern kmem_cache_t kernel_caches[NUM_CACHES];

void init_slab(void);
void *slab_alloc(kmem_cache_t *cahce);
slab_t *slab_create(kmem_cache_t *cache);
void slab_free(void *ptr);
void slab_list_remove(slab_t **head, slab_t *slab);
void slab_list_add(slab_t **head, slab_t *slab);