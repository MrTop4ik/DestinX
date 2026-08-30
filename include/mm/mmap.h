#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>
#include <kernel/scheduler/thread.h>
#include <kernel/scheduler/process.h>

#define MMAP_START      0x0000700000000000

#define PROT_NONE       0x0
#define PROT_READ       0x1
#define PROT_WRITE      0x2
#define PROT_EXEC       0x4

#define MAP_SHARED      0x1
#define MAP_PRIVATE     0x2
#define MAP_FIXED       0x10
#define MAP_ANONYMOUS   0x20

#define EINVAL          22
#define ENOMEM          12

uint64_t mmap(uint64_t addr, size_t size, uint32_t prot, uint32_t flags, uint64_t fd, uint64_t offset);
void mmap_add_to_list(vm_area_t *i);
void mmap_remove_from_list(vm_area_t *i);
void munmap(void *ptr);
void munmap_by_info(vm_area_t *i, uint64_t pml4);