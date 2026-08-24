#include <arch/x86_64/apic/lapic.h>

void init_lapic(void){
    vmm_map_page(read_cr3(), lapic_paddr, LAPIC_VIRT, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_CD));

    uint32_t low, high;

    read_msr(IA32_APIC_MSR, &low, &high);
    low |= IA32_APIC_ENABLE;
    write_msr(IA32_APIC_MSR, low, high);

    write_lapic(LAPIC_SVR_OFFSET, SPURIOUS_VECTOR | LAPIC_SVR_ENABLE);
    write_lapic(LAPIC_TPR_OFFSET, 0);

    write_lapic(LAPIC_LVT_LINTO_OFFSET, 0x700);
}

void lapic_eoi(void){
    *(volatile uint32_t *)(LAPIC_VIRT + LAPIC_EOI_OFFSET) = 0;
}

uint32_t read_lapic(uint32_t reg){
    return *(volatile uint32_t *)(LAPIC_VIRT + reg);
}

void write_lapic(uint32_t reg, uint32_t val){
    *(volatile uint32_t *)(LAPIC_VIRT + reg) = val;
}
