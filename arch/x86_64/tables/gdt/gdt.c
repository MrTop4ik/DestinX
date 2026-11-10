#include <arch/x86_64/tables/gdt.h>

extern uint8_t ist1_stack_top;
extern void gdt_flush(uint64_t);

struct gdt_entry_struct gdt_entries[7];
struct gdt_ptr_struct gdt_ptr;
struct TSS tss;

void init_GDT(void){
    setGDTGate(0, 0, 0, 0, 0);
    setGDTGate(1, 0, 0xFFFFFFFF, 0x9A, 0x20);
    setGDTGate(2, 0, 0xFFFFFFFF, 0x92, 0x00);
    setGDTGate(3, 0, 0xFFFFFFFF, 0xF2, 0x00);
    setGDTGate(4, 0, 0xFFFFFFFF, 0xFA, 0x20);

    for (size_t i = 0; i < sizeof(struct TSS); i++){
        ((uint8_t*)&tss)[i] = 0;
    }
    
    tss.ist[0] = (uint64_t)&ist1_stack_top;

    tss.iopb = sizeof(struct TSS);
    
    setTSSGate(5, (uint64_t)&tss, sizeof(struct TSS) - 1);

    gdt_ptr.limit = (sizeof(gdt_entries)) - 1;
    gdt_ptr.base = (uint64_t)&gdt_entries;

    gdt_flush((uint64_t)&gdt_ptr);
}

void setGDTGate(uint8_t vector, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags){
    gdt_entries[vector].limit_low = (limit & 0xFFFF);
    gdt_entries[vector].base_low = (base & 0xFFFF);
    gdt_entries[vector].base_mid = (base >> 16) & 0xFF;
    gdt_entries[vector].access = access;
    gdt_entries[vector].limit_high_flags = ((limit >> 16) & 0x0F) | (flags & 0xF0);
    gdt_entries[vector].base_high = (base >> 24) & 0xFF;
}

void setTSSGate(uint8_t vector, uint64_t base, uint32_t limit){
    struct tss_entry_struct *tss_entry = (struct tss_entry_struct*)&gdt_entries[vector];

    setGDTGate(vector, (uint32_t)base, limit, 0x89, 0x40);
    tss_entry->base_upper = (uint32_t)(base >> 32);
    tss_entry->reserved = 0;
}