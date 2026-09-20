#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/drivers/rtc.h>
#include <arch/x86_64/drivers/tsc.h>

extern rtc_time_t boot_rtc;
extern uint64_t boot_tsc;
extern uint64_t tsc_ticks_per_ms;
extern uint64_t unix_boot_timestamp;

void init_time(void);