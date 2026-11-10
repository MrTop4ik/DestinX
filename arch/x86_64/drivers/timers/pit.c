#include <arch/x86_64/drivers/timers/pit.h>

volatile uint64_t ticks;

uint32_t division = 0;

void init_PIT(uint32_t freq){
    ticks = 0;
    division = 1193180 / freq;

    setIRQHandler(0, &PIT_handler);
    
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(division & 0xFF));
    outb(0x40, (uint8_t)((division >> 8) & 0xFF));

    ioapic_set_irq(0, 32, 0);
}

void PIT_handler(struct InterruptRegisters *regs){
    ticks++;
}

void pit_wait_ms(uint32_t ms){
    uint32_t total_ticks = ms * 1193;

    while (total_ticks > 0){
        uint16_t wait = (total_ticks > 0xFFFF) ? 0xFFFF : (uint16_t)total_ticks;

        outb(0x43, 0x30);
        outb(0x40, (uint8_t)wait);
        outb(0x40, (uint8_t)(wait >> 8));

        uint16_t last = wait;
        while (1){
            outb(0x43, 0x00);
            uint16_t count = inb(0x40) | inb(0x40) << 8;
            if (count > last || count == 0) break;
            last = count;
        }

        total_ticks -= wait;
    }

    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(division & 0xFF));
    outb(0x40, (uint8_t)((division >> 8) & 0xFF));
}