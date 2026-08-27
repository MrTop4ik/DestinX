#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/kmalloc.h>
#include <drivers/lfb.h>
#include <kernel/scheduler/process.h>
#include <mm/vmalloc.h>

#define DEFAULT_STACK_SIZE 0x100000
#define USER_STACK_MAX 0x00007FFFFFFFF000

typedef struct {
    void *top;
    void *bottom;
    size_t size;
} stack_t;

extern vm_info_t *us_list_head;

void *kernel_alloc_stack(size_t size);
void *user_alloc_stack(size_t size);
void user_free_stack(void *stack_bottom);
void us_add_to_list(vm_info_t *i);
void us_remove_from_list(vm_info_t *i);