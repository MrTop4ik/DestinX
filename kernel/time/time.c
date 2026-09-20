#include <kernel/time/time.h>

rtc_time_t boot_rtc = {0};
uint64_t boot_tsc = 0;
uint64_t tsc_ticks_per_ms = 0;
uint64_t unix_boot_timestamp = 0;

void init_time(void){
    boot_rtc = read_rtc();
    init_tsc(&boot_tsc, &tsc_ticks_per_ms);

    uint64_t a = (14 - boot_rtc.month) / 12;
    uint64_t y = boot_rtc.year + 4800 - a;
    uint64_t m = boot_rtc.month + 12 * a - 3;

    uint64_t days = boot_rtc.day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
    uint64_t days_since_unix = days - 2440588;

    unix_boot_timestamp = (days_since_unix * 86400) + (boot_rtc.hour * 3600) + (boot_rtc.minute * 60) + boot_rtc.second;
}

uint64_t time(void){
    uint64_t cur_rdtsc = rdtsc();
    uint64_t seconds_passed = (cur_rdtsc - boot_tsc) / tsc_ticks_per_ms / 1000;
    return seconds_passed + unix_boot_timestamp;
}