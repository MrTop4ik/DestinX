#include <arch/x86_64/drivers/rtc.h>

int get_update_in_progress_flag(void){
    outb(CMOS_ADDRESS_PORT, 0x0A);
    return (inb(CMOS_DATA_PORT) & 0x80);
}

uint8_t read_rtc_register(int reg){
    outb(CMOS_ADDRESS_PORT, reg);
    return inb(CMOS_DATA_PORT);
}

rtc_time_t read_rtc(void){
    rtc_time_t time;

    while (get_update_in_progress_flag());

    time.second = read_rtc_register(0x00);
    time.minute = read_rtc_register(0x02);
    time.hour   = read_rtc_register(0x04);
    time.day    = read_rtc_register(0x07);
    time.month  = read_rtc_register(0x08);
    time.year   = read_rtc_register(0x09);

    uint8_t registerB = read_rtc_register(0x0B);

    if (!(registerB & 0x04)){
        time.second = (time.second & 0x0F) + ((time.second / 16) * 10);
        time.minute = (time.minute & 0x0F) + ((time.minute / 16) * 10);
        time.hour   = ((time.hour & 0x0F) + (((time.hour & 0x70) / 16) * 10)) | (time.hour & 0x80);
        time.day    = (time.day & 0x0F) + ((time.day / 16) * 10);
        time.month  = (time.month & 0x0F) + ((time.month / 16) * 10);
        time.year   = (time.year & 0x0F) + ((time.year / 16) * 10);
    }

    if (!(registerB & 0x02) && (time.hour & 0x80)){
        time.hour = ((time.hour & 0x7F) + 12) % 24;
    }

    time.year += 2000; 

    return time;
}