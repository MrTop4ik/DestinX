#pragma once
#include <stdint.h>
#include <arch/x86_64/drivers/serial.h>

#define CR3_NOFLUSH (1 << 63)

extern uint64_t pcid;
extern uint64_t sse_avx;

void init_utils(void);