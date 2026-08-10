#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <arch/x86_64/inlineasm.h>

#define COM1_PORT 0x3F8

char * itoa(int value, char * str, int base);
void serial_init(void);
void serial_putchar(char a);
void serial_put_number(uint32_t num, uint32_t base);
void serial_print(const char* format, ...);