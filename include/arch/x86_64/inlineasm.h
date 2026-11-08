#pragma once
#include <stdint.h>
#include <arch/x86_64/drivers/serial.h>

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);
void invalidate(uint64_t vaddr);
uint64_t read_cr3(void);
void write_cr3(uint64_t paddr);
void read_msr(uint32_t index, uint32_t *l, uint32_t *h);
void write_msr(uint32_t index, uint32_t low, uint32_t high);
uint64_t read_rsp(void);
uint64_t read_rflags(void);
void write_rflags(uint64_t rflags);
uint64_t rdtsc(void);
__attribute__((naked)) void naked_yield(uint8_t _sti);
void yield(uint8_t _sti);
void sti(void);
void cli(void);
void hlt(void);
void clts(void);
void mfence(void);