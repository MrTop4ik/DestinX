#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/stack.h>
#include <arch/x86_64/idt.h>
#include <drivers/lfb.h>
#include <arch/x86_64/inlineasm.h>
#include <drivers/kring.h>
#include <kernel/scheduler/process.h>

typedef enum {
    READY = 0,
    RUNNING,
    BLOCKED,
    DEAD
} thread_state_t;

typedef struct thread {
    uint64_t rsp;
    uint64_t tid;
    thread_state_t state;
    stack_t kernel_stack;
    stack_t user_stack;
    uint64_t page_guard_min;
    uint64_t page_guard_max;
    struct process *process;
    struct thread *prev;
    struct thread *next;
    struct thread *next_blocked;
    struct thread *next_pthread;
    struct thread *prev_pthread;
} thread_t;

extern uint64_t next_thread_id;
extern thread_t *current_thread;
extern thread_t *ready_list_head;
extern thread_t *dead_list_head;

thread_t *create_thread(void (*entry_pointer)(void), size_t stack_size);
thread_t *create_user_thread(struct process *proc, void (*entry_point)(void), size_t kstack_t, size_t ustack_size);
void destroy_thread(thread_t *t);
void thread_exit(void);
void enqueue_thread(thread_t *t);
void dequeue_thread(thread_t *t);
void yield(void);
void cleanup_dead_threads(void);
void idle_thread_entry(void);
void third_thread(void);