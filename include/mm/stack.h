#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/kmalloc.h>
#include <drivers/lfb.h>

#define DEFAULT_STACK_SIZE 0x100000
#define USER_STACK_MAX 0x00007FFFFFFFFFFF

typedef struct {
    void *top;
    void *bottom;
    size_t size;
} stack_t;

typedef struct us_info {
    void *base;
    size_t size;
    struct us_info *next;
    struct us_info *prev;
} us_info_t;

extern us_info_t *us_list_head;

void *kernel_alloc_stack(size_t size);
void *user_alloc_stack(size_t size);
void user_free_stack(void *stack_bottom);
void us_add_to_list(us_info_t *i);
void us_remove_from_list(us_info_t *i);