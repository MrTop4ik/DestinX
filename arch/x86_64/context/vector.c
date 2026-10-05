#include <stdint.h>
#include <arch/x86_64/idt.h>
#include <kernel/scheduler/thread.h>

thread_t *ctx_thread = NULL;

void vector_ctx_switch(struct InterruptRegisters *regs){
    clts();
    if (current_thread == ctx_thread) return;

    if (sse_avx == 1){
        if (ctx_thread) save_sse_ctx(ctx_thread->sse_avx_buffer);
        ctx_thread = current_thread;
        restore_sse_ctx(ctx_thread->sse_avx_buffer);
    } else if (sse_avx == 2){
        if (ctx_thread) save_avx_ctx(ctx_thread->sse_avx_buffer);
        ctx_thread = current_thread;
        restore_avx_ctx(ctx_thread->sse_avx_buffer);
    }

}