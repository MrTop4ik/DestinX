#include <arch/x86_64/drivers/lapic_timer.h>
#include <kernel/scheduler/thread.h>

extern void* lapic_timer_handler();

uint32_t lapic_timer_ticks_per_ms = 0;

void init_lapic_timer(uint8_t vector){
    write_lapic(LAPIC_TIMER_DIV, 0x3);
    write_lapic(LAPIC_TIMER_LVT, LAPIC_TIMER_MASK);
    
    write_lapic(LAPIC_TIMER_INITCNT, 0xFFFFFFFF);
    pit_wait_ms(10);
    
    uint32_t current = read_lapic(LAPIC_TIMER_CURRCNT);
    uint32_t ticks_in_10ms = 0xFFFFFFFF - current;
    lapic_timer_ticks_per_ms = ticks_in_10ms / 10;

    if (lapic_timer_ticks_per_ms> 0xFFFFFFFF) serial_print("[LAPIC TIMER] Ticks per ms > 0xFFFFFFFF\n");

    write_lapic(LAPIC_TIMER_LVT, vector | LAPIC_TIMER_ONESHOT);
    write_lapic(LAPIC_TIMER_INITCNT, lapic_timer_ticks_per_ms);

    setIDTGate(vector, (uint64_t)lapic_timer_handler, 0x8E, 0);
}

void lapic_timer_reload_oneshot(uint64_t ms){
    write_lapic(LAPIC_TIMER_INITCNT, lapic_timer_ticks_per_ms * ms);
}