#pragma once
#include <stdint.h>
#include <stddef.h>

struct gdt_entry_struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_high_flags;
    uint8_t base_high;
}__attribute__((packed));

struct gdt_ptr_struct {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));

struct tss_entry_struct {
    struct gdt_entry_struct common;
    uint32_t base_upper;
    uint32_t reserved;
}__attribute__((packed));

struct TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb;
}__attribute__((packed));

extern struct TSS tss;

void init_GDT(void);
void setGDTGate(uint8_t vector, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void setTSSGate(uint8_t vector, uint64_t base, uint32_t limit);