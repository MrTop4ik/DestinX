#include <arch/x86_64/utils.h>

extern uint64_t sse_avx_check(void);
extern void init_sse_avx(void);
extern uint64_t check_pcid(void);
extern void enable_pcid(void);

uint64_t pcid;
uint64_t sse_avx;

void init_utils(void){
    sse_avx = sse_avx_check();
    init_sse_avx();

    if (sse_avx == 1) serial_print("[SSE] SSE Was Initialized\n");
    else if (sse_avx = 2) serial_print("[AVX] AVX Was Initialized\n");
    else serial_print("[SSE] SSE Is Not Supprted\n");

    pcid = check_pcid();
    if (pcid){
        enable_pcid();
        serial_print("[PCID] PCID Enabled\n");
    } else serial_print("[PCID] PCID Is Not Supported\n");
    
}