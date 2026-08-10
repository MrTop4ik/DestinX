#pragma once
#include <stdint.h>

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);
void outl(uint16_t port, uint32_t val);
uint8_t inl(uint16_t port);
void invalidate(uint64_t vaddr);
uint64_t read_cr3(void);
void write_cr3(uint64_t paddr);
void read_msr(uint32_t index, uint32_t *l, uint32_t *h);
void write_msr(uint32_t index, uint32_t low, uint32_t high);
uint64_t read_rsp(void);
uint64_t read_rflags(void);
void write_rflags(uint64_t rflags);
void yield(void);
void sti(void);
void cli(void);
void hlt(void);
