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

    pcid = check_pcid();
    if (pcid){
        enable_pcid();
        serial_print("[PCID] PCID Enabled\n");
    } else serial_print("[PCID] PCID Enabled\n");
    
}