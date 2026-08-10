#include <arch/x86_64/drivers/serial.h>

char * itoa(int value, char * str, int base){
    char * rc;
    char * ptr;
    char * low;
    if ( base < 2 || base > 36 ){
        *str = '\0';
        return str;
    }
    rc = ptr = str;
    if ( value < 0 && base == 10 ){
        *ptr++ = '-';
    }
    low = ptr;
    do{
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + value % base];
        value /= base;
    } while ( value );
    *ptr-- = '\0';
    while ( low < ptr ){
        char tmp = *low;
        *low++ = *ptr;
        *ptr-- = tmp;
    }
    return rc;
}

void serial_init(){
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x80);
    outb(COM1_PORT + 0, 0x03);
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + 3, 0x03);
    outb(COM1_PORT + 2, 0xC7);
    outb(COM1_PORT + 4, 0x0B);
}

void serial_putchar(char a){
    while ((inb(COM1_PORT + 5) & 0x20) == 0);
    outb(COM1_PORT, a);
}

void serial_put_number(uint32_t num, uint32_t base){
    char buf[32];
    int i = 0;
    const char digits[] = "0123456789abcdef";

    if (num == 0) {
        serial_putchar('0');
        return;
    }

    while (num > 0) {
        buf[i++] = digits[num % base];
        num /= base;
    }

    while (i > 0) {
        serial_putchar(buf[--i]);
    }
}

void serial_print(const char* format, ...){
    va_list args;
    va_start(args, format);

    for (const char* p = format; *p != '\0'; p++) {
        if (*p != '%') {
            serial_putchar(*p);
            continue;
        }

        p++;
        switch (*p) {
            case 's':
                serial_print(va_arg(args, char*));
                break;
            case 'd': {
                int i = va_arg(args, int);
                if (i < 0) {
                    serial_putchar('-');
                    i = -i;
                }
                serial_put_number((uint32_t)i, 10);
                break;
            }
            case 'x': {
                uint32_t x = va_arg(args, uint32_t);
                serial_put_number(x, 16);
                break;
            }
            case 'l':
                if (*(p+1) == 'l' && *(p+2) == 'x') {
                    p += 2;
                    uint64_t val = va_arg(args, uint64_t);
                    
                    if (val == 0) {
                        serial_putchar('0');
                    } else {
                        char buf[16];
                        int i = 0;
                        while (val > 0) {
                            uint32_t rem = val % 16;
                            buf[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
                            val /= 16;
                        }
                        while (i > 0) {
                            serial_putchar(buf[--i]);
                        }
                    }
                }
                break;
            case 'p':
                serial_print("0x");
                serial_put_number(va_arg(args, uint32_t), 16);
                break;
            case '%':
                serial_putchar('%');
                break;
        }
    }
    va_end(args);
}