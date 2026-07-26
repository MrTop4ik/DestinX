#include <arch/x86_64/idt.h>

extern void* isr_stub_table[];
extern void* irq_stub_table[];

void page_fault_handler(struct InterruptRegisters *regs);
extern void* lapic_timer_handler();
extern void* yield_handler();
extern void* isr255();

char *exceptions[] = {
    "Division By Zero.",
    "Debug.",
    "Non Maskable Interrupt.",
    "Breakpoint.",
    "Into Detected Overflow.",
    "Out Of Bounds.",
    "Invalid Opcode.",
    "No Coprocessor.",
    "Double Fault.",
    "Coprocessor Segment Overrun.",
    "Bad TSS.",
    "Segment not present.",
    "Stack fault.",
    "General Protection Fault.",
    "Page Fault.",
    "Unknown Interrupt.",
    "Coprocessor Fault.",
    "Aligment Fault.",
    "Machine Check.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
    "Reserved.",
};

void* irq_handlers[16] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

struct idt_entry_struct idt_entries[256];
struct idt_ptr_struct idt_ptr;

void remap_PIC(void){
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void init_IDT(void){
    idt_ptr.limit = (uint16_t)(sizeof(struct idt_entry_struct) * 256) - 1;
    idt_ptr.base = (uint64_t)&idt_entries;

    remap_PIC();

    for (uint16_t i = 0; i < 256; i++){
        setIDTGate(i, 0, 0, 0);
    }

    for (uint8_t vector = 0; vector < 8; vector++){
        setIDTGate(vector, (uint64_t)isr_stub_table[vector], 0x8E, 0);
    }

    setIDTGate(8, (uint64_t)isr_stub_table[8], 0x8E, 1);
    
    for (uint8_t vector = 9; vector < 32; vector++){
        setIDTGate(vector, (uint64_t)isr_stub_table[vector], 0x8E, 0);
    }

    for (uint8_t vector = 32; vector < 48; vector++){
        setIDTGate(vector, (uint64_t)irq_stub_table[vector - 32], 0x8E, 0);
    }

    setIDTGate(0x81, (uint64_t)yield_handler, 0x8E, 0);
    setIDTGate(0xEF, (uint64_t)lapic_timer_handler, 0x8E, 0);
    setIDTGate(255, (uint64_t)isr255, 0x8E, 0);

    __asm__ volatile ("lidt %0" : : "m" (idt_ptr));
}

void setIDTGate(uint8_t vector, uint64_t handler, uint8_t flags, uint8_t ist){
    idt_entries[vector].offset_1 = (uint16_t)((uint64_t)handler & 0xFFFF);
    idt_entries[vector].selector = 0x08;
    idt_entries[vector].ist = ist & 0x07;
    idt_entries[vector].flags = flags;
    idt_entries[vector].offset_2 = (uint16_t)(((uint64_t)handler >> 16) & 0xFFFF);
    idt_entries[vector].offset_3 = (uint32_t)(((uint64_t)handler >> 32) & 0xFFFFFFFF);
    idt_entries[vector].reserved = 0;
}

void exception_handler(struct InterruptRegisters *regs){
    if (regs->int_no < 32){
        isr_handler(regs);
    }

    if (regs->int_no > 31 && regs->int_no < 48){
        irq_handler(regs);
    }

    if (regs->int_no == 255){
        lapic_eoi();
    }
}

void isr_handler(struct InterruptRegisters *regs){
    serial_print("[ISR] ");
    serial_print(exceptions[regs->int_no]);
    serial_print("\n");

    if (regs->int_no == 0x0e){
        page_fault_handler(regs);
    }
}

void setIRQHandler(uint8_t num, void (*handler)(struct InterruptRegisters *regs)){
    irq_handlers[num] = handler;
}

void deleteIRQHandler(uint8_t num){
    irq_handlers[num] = 0;
}

void irq_handler(struct InterruptRegisters *regs){
    void (*handler)(struct InterruptRegisters *regs);

    handler = irq_handlers[regs->int_no - 32];

    handler(regs);

    lapic_eoi();
}