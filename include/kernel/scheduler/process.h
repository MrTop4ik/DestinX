#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/stack.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/inlineasm.h>
#include <arch/x86_64/elf.h>
#include <mm/vmalloc.h>

struct thread;

typedef struct process {
    uint64_t pml4;
    uint64_t pid;
    struct thread *threads;
    struct vm_info *ustacks_infos;
    struct vm_info *mmap_infos;
    uint64_t heap_start;
    uint64_t current_heap_end;
    uint64_t pages_alloced;
} process_t;

process_t *create_user_process(const char *fp);