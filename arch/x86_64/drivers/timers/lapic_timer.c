#include <arch/x86_64/drivers/lapic_timer.h>
#include <kernel/scheduler/thread.h>

extern void* lapic_timer_handler();

uint64_t lapic_timer_ticks = 0;
extern thread_t *sleeping_list_head;

void init_lapic_timer(uint8_t vector, uint32_t ms){
    write_lapic(LAPIC_TIMER_DIV, 0x3);
    write_lapic(LAPIC_TIMER_LVT, LAPIC_TIMER_MASK);
    
    write_lapic(LAPIC_TIMER_INITCNT, 0xFFFFFFFF);
    pit_wait_ms(10);
    
    uint32_t current = read_lapic(LAPIC_TIMER_CURRCNT);
    uint32_t ticks_in_10ms = 0xFFFFFFFF - current;
    uint32_t ticks_per_ms = ticks_in_10ms / 10;

    if (ticks_per_ms * ms > 0xFFFFFFFF) kprintf("Ticks per %dms > 0xFFFFFFFF\n");

    write_lapic(LAPIC_TIMER_LVT, vector | LAPIC_TIMER_PERIODIC);
    write_lapic(LAPIC_TIMER_INITCNT, (uint32_t)(ticks_per_ms * ms));

    setIDTGate(vector, (uint64_t)lapic_timer_handler, 0x8E, 0);
}

void lapic_timer_centry(void){
    lapic_timer_ticks++;

    thread_t *prev = NULL;
    thread_t* cur = sleeping_list_head;

    while(cur){
        if (lapic_timer_ticks >= cur->sleep_time){
            cur->state = READY;
            enqueue_thread(cur);

            if (prev) prev->next = cur;
            if (cur == sleeping_list_head) sleeping_list_head = cur->next;
        }

        prev = cur;
        cur = cur->next_sleeping;
    }
}