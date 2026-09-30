#include <arch/x86_64/utils.h>

uint64_t pcid;
uint64_t sse_avx;
uint64_t sse_avx_buf_size;

void init_utils(void){
    sse_avx = sse_avx_check();
    init_sse_avx();
    sse_avx_buf_size = get_sse_avx_buf_size();

    if (sse_avx == 1) serial_print("[SSE] SSE Was Initialized\n");
    else if (sse_avx = 2) serial_print("[AVX] AVX Was Initialized\n");
    else serial_print("[SSE] SSE Is Not Supprted\n");

    pcid = check_pcid();
    if (pcid){
        enable_pcid();
        serial_print("[PCID] PCID Enabled\n");
    } else serial_print("[PCID] PCID Is Not Supported\n");
    
}