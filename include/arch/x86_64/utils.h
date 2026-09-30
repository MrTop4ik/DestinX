#pragma once
#include <stdint.h>
#include <arch/x86_64/drivers/serial.h>

#define CR3_NOFLUSH (1 << 63)

extern uint64_t pcid;
extern uint64_t sse_avx;
extern uint64_t sse_avx_buf_size;

extern uint64_t sse_avx_check(void);
extern void init_sse_avx(void);
extern uint64_t check_pcid(void);
extern void enable_pcid(void);
extern uint64_t get_sse_avx_buf_size(void);
extern void save_avx_ctx(void *buf);
extern void restore_avx_ctx(void *buf);
extern void save_sse_ctx(void *buf);
extern void restore_sse_ctx(void *buf);

void init_utils(void);