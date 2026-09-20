#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/inlineasm.h>
#include <arch/x86_64/drivers/pit.h>

void init_tsc(uint64_t *boot_tsc, uint64_t *ticks_per_ms);