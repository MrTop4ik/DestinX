#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/stack.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/inlineasm.h>
#include <arch/x86_64/elf.h>
#include <mm/vmalloc.h>
#include <fs/vfs.h>
#include <drivers/console.h>

#define MAX_FD 128

struct thread;

typedef struct process {
    uint64_t pml4;
    uint64_t pid;
    struct thread *threads;
    struct vm_area *ustacks_infos;
    struct vm_area *mmap_infos;
    struct FILE *fd_table[MAX_FD];
    uint64_t heap_start;
    uint64_t current_heap_end;
    uint64_t pages_alloced;
} process_t;

process_t *create_user_process(const char *fp);