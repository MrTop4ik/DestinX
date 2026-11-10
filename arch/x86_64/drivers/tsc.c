#include <arch/x86_64/drivers/tsc.h>

void init_tsc(uint64_t *boot_tsc, uint64_t *ticks_per_ms){
    *boot_tsc = rdtsc();
    
    pit_wait_ms(10);

    uint64_t current = rdtsc();
    *ticks_per_ms = (current - *boot_tsc) / 10;
}