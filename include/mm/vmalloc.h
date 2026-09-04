#pragma once 
#include <stdint.h>
#include <mm/kmalloc.h>

#define VMALLOC_START 0xFFFFC00000000000

typedef struct vm_area {
    void *base;
    size_t size;
    uint32_t prot;
    uint32_t flags;
    struct FILE *file;
    uint64_t file_pgoff;
    struct vm_area *next;
    struct vm_area *prev;
} vm_area_t;

extern vm_area_t *vmalloc_list_head;

void *vmalloc(size_t size);
void vfree(void *ptr);
void vm_add_to_list(vm_area_t *i);
void vm_remove_from_list(vm_area_t *i);