#include <arch/x86_64/drivers/tsc.h>

uint64_t boot_tsc = 0;
uint64_t ticks_per_ms = 0;

void init_tsc(void){
    boot_tsc = rdtsc();
    
    pit_wait_ms(10);

    uint64_t current = rdtsc();
    ticks_per_ms = current / 10;
}