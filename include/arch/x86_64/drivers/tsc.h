#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/inlineasm.h>
#include <arch/x86_64/drivers/pit.h>

extern uint64_t boot_tsc;
extern uint64_t ticks_per_ms;

void init_tsc(void);