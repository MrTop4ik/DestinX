#include <arch/x86_64/apic/ioapic.h>

void init_ioapic(void){
    vmm_map_page(read_cr3(), ioapic_paddr, IOAPIC_VIRT, PAGE_SIZE_4KB, (PTE_WRITABLE));
}

uint32_t get_gsi_for_irq(uint8_t irq){
    for (int i = 0; i < iso_count; i++){
        if (iso_list[i].IRQSource == irq){
            return iso_list[i].GSI;
        }
    }
    return (uint32_t)irq;
}

void ioapic_set_irq(uint8_t irq, uint8_t vector, uint8_t cpu_id){
    uint32_t gsi = irq;
    uint16_t flags = 0;

    for (int i = 0; i < iso_count; i++){
        if (iso_list[i].IRQSource == irq){
            gsi = iso_list[i].GSI;
            flags = iso_list[i].Flags;
            break;
        }
    }

    uint32_t low = vector;

    if (flags & 0x2) low |= (1 << 13);
    if (flags & 0x8) low |= (1 << 15);

    write_ioapic(IOAPIC_REDTBL_OFFSET + gsi * 2, low);
    write_ioapic(IOAPIC_REDTBL_OFFSET + gsi * 2 + 1, (uint32_t)(cpu_id << 24));
}

uint32_t read_ioapic(uint32_t reg){
    *(volatile uint32_t *)(IOAPIC_VIRT + IOAPIC_REGSEL_OFFSET) = reg;
    return *(volatile uint32_t *)(IOAPIC_VIRT + IOAPIC_IOWIN_OFFSET);
}

void write_ioapic(uint32_t reg, uint32_t data){
    *(volatile uint32_t *)(IOAPIC_VIRT + IOAPIC_REGSEL_OFFSET) = reg;
    *(volatile uint32_t *)(IOAPIC_VIRT + IOAPIC_IOWIN_OFFSET) = data;
}