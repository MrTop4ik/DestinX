#include <arch/x86_64/drivers/pci.h>

void pci_enable_bus_master(uint8_t bus, uint8_t dev, uint8_t func){
    uint32_t command = pci_read_dword(bus, dev, func, 0x04);
    command |= (1 << 2);
    pci_write_dword(bus, dev, func, 0x04, command);
}

void pci_enable_msix(uint8_t bus, uint8_t dev, uint8_t func){
    uint32_t cap_reg = pci_read_dword(bus, dev, func, 0x34);
    uint8_t cap_ptr = cap_reg & 0xFF;

    while (cap_ptr != 0){
        uint32_t cap_header = pci_read_dword(bus, dev, func, cap_ptr);
        uint8_t cap_id = (uint8_t)(cap_header & 0xFF);
        uint8_t next_ptr = (uint8_t)((cap_header >> 8) & 0xFF);

        if (cap_id == 0x11){
            cap_header |= (1 << 31);
            cap_header &= ~(1 << 30);
            pci_write_dword(bus, dev, func, cap_ptr, cap_header);
            return;
        }
        cap_ptr = next_ptr;
    }
}

void set_msix_vector(uint8_t bus, uint8_t dev, uint8_t func, uint8_t idt_vector, uint16_t msix_vector, uint8_t lapic_id){
    uint32_t cap_reg = pci_read_dword(bus, dev, func, 0x34);
    uint8_t cap_ptr = cap_reg & 0xFF;

    while (cap_ptr != 0){
        uint32_t cap_header = pci_read_dword(bus, dev, func, cap_ptr);
        uint8_t cap_id = (uint8_t)(cap_header & 0xFF);
        uint8_t next_ptr = (uint8_t)((cap_header >> 8) & 0xFF);

        if (cap_id == 0x11){
            uint32_t tbloff_bir = pci_read_dword(bus, dev, func, cap_ptr + 4);
            uint32_t tbl_off = tbloff_bir & ~7;
            uint8_t bar_num = tbloff_bir & 7;

            uint32_t bar_info = pci_read_dword(bus, dev, func, 0x10 + bar_num * 4);
            uint8_t bar_flags = bar_info & 0x0F;
            uint64_t bar_address = bar_info & 0xFFFFFFF0;
            if (bar_flags & (1 << 2)) bar_address |= (uint64_t)(pci_read_dword(bus, dev, func, 0x10 + bar_num * 4 + 4)) << 32;

            msix_entry_t *table = (msix_entry_t *)(bar_address + DIRECT_OFFSET);
            
            table[msix_vector].msg_addr_lo = lapic_paddr;
            table[msix_vector].msg_addr_hi = 0x0;
            table[msix_vector].msg_data = idt_vector;
            table[msix_vector].vector_ctrl = 0x0;

            return;
        }
        cap_ptr = next_ptr;
    }
}