#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/drivers/serial.h>
#include <drivers/lfb.h>
#include <arch/x86_64/inlineasm.h>
#include <kernel/scheduler/thread.h>

#define IA32_EFER   0xC0000080
#define IA32_STAR   0xC0000081
#define IA32_LSTAR  0xC0000082
#define IA32_FMASK  0xC0000084
#define IA32_KERNEL_GS_BASE 0xC0000102

#define FLAGS_IF 0x200
#define FLAGS_DF 0x40
#define FLAGS_TF 0x100

#define SYS_EXIT 60

typedef struct {
    uint64_t user_rsp;
    uint64_t kernel_rsp;
}__attribute__((packed)) syscalls_stacks_t;

struct SyscallRegisters {
    uint64_t rbx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip, rflags;
}__attribute__((packed));

extern syscalls_stacks_t sstacks;

void init_syscalls(void);
void init_kernel_gs_base(void);
void syscall_handler(uint64_t sys_num, struct SyscallRegisters *regs);