#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/stack.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/inlineasm.h>

struct thread;
struct us_info;

typedef struct process {
    uint64_t pml4;
    uint64_t pid;
    struct thread *threads;
    struct us_info *ustacks_infos;
} process_t;

process_t *create_user_process();