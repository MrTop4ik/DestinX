#pragma once
#include <stdint.h>
#include <arch/x86_64/inlineasm.h>
#include <arch/x86_64/apic/lapic.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/drivers/lapic_timer.h>

struct idt_entry_struct {
    uint16_t offset_1;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t reserved;
}__attribute__((packed));

struct idt_ptr_struct {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));

struct InterruptRegisters {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss; 
}__attribute__((packed));

extern char *exceptions[];

void remap_PIC(void);
void init_IDT(void);
void setIDTGate(uint8_t vector, uint64_t handler, uint8_t flags, uint8_t ist);
void exception_handler(struct InterruptRegisters *regs);
void isr_handler(struct InterruptRegisters *regs);
void irq_handler(struct InterruptRegisters *regs);
void setIRQHandler(uint8_t num, void (*handler)(struct InterruptRegisters *regs));
void deleteIRQHandler(uint8_t num);