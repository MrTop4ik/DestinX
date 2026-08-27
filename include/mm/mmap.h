#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>
#include <kernel/scheduler/thread.h>
#include <kernel/scheduler/process.h>

#define MMAP_START 0x0000700000000000

uint64_t mmap(size_t size);
void mmap_add_to_list(vm_info_t *i);
void mmap_remove_from_list(vm_info_t *i);