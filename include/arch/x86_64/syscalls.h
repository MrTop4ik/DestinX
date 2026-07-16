#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/drivers/video/serial.h>
#include <drivers/lfb.h>
#include <arch/x86_64/inlineasm.h>

#define IA32_EFER   0xC0000080
#define IA32_STAR   0xC0000081
#define IA32_LSTAR  0xC0000082
#define IA32_FMASK  0xC0000084
#define IA32_KERNEL_GS_BASE 0xC0000102


typedef struct {
    uint64_t user_rsp;
    uint64_t kernel_rsp;
}__attribute__((packed)) syscalls_stacks_t;