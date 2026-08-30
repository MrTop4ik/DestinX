#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/drivers/serial.h>
#include <drivers/lfb.h>
#include <arch/x86_64/inlineasm.h>
#include <kernel/scheduler/thread.h>
#include <mm/brk.h>
#include <mm/mmap.h>
#include <drivers/console.h>

#define IA32_EFER   0xC0000080
#define IA32_STAR   0xC0000081
#define IA32_LSTAR  0xC0000082
#define IA32_FMASK  0xC0000084
#define IA32_KERNEL_GS_BASE 0xC0000102

#define FLAGS_IF 0x200
#define FLAGS_DF 0x400
#define FLAGS_TF 0x100

#define SYS_READ        0
#define SYS_WRITE       1
#define SYS_OPEN        2
#define SYS_CLOSE       3
#define SYS_MMAP        9
#define SYS_MUNMAP      11
#define SYS_BRK         12
#define SYS_EXIT        60
#define SYS_EXIT_GROUP  231

typedef struct {
    uint64_t user_rsp;
    uint64_t kernel_rsp;
}__attribute__((packed)) syscalls_stacks_t;

struct SyscallRegisters {
    uint64_t rax, rbx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip, rflags;
}__attribute__((packed));

extern syscalls_stacks_t sstacks;

void init_syscalls(void);
void init_kernel_gs_base(void);
void syscall_handler(struct SyscallRegisters *regs);