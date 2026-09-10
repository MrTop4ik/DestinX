#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/inlineasm.h>

#define CMOS_ADDRESS_PORT 0x70
#define CMOS_DATA_PORT    0x71

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint32_t year;
} rtc_time_t;

int get_update_in_progress_flag(void);
uint8_t read_rtc_register(int reg);
rtc_time_t read_rtc(void);
