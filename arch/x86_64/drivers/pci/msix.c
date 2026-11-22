#include <arch/x86_64/drivers/pci.h>

void pci_enable_bus_master(uint8_t bus, uint8_t dev, uint8_t func){
    uint32_t command = pci_read_dword(bus, dev, func, 0x04);
    pci_write_dword(bus, dev, func, 0x04, command | (1 << 2));
}

void pci_enable_msix(uint8_t bus, uint8_t dev, uint8_t func){
    uint32_t command = pci_read_dword(bus, dev, func, 0x04);
    command |= (1 << 10);
    pci_write_dword(bus, dev, func, 0x04, command);

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

void pci_set_msix_vector(uint8_t bus, uint8_t dev, uint8_t func, uint8_t idt_vector, uint16_t msix_vector, uint8_t lapic_id){
    uint32_t cap_reg = pci_read_dword(bus, dev, func, 0x34);
    uint8_t cap_ptr = cap_reg & 0xFF;

    while (cap_ptr != 0){
        uint32_t cap_header = pci_read_dword(bus, dev, func, cap_ptr);
        uint8_t cap_id = (uint8_t)(cap_header & 0xFF);
        uint8_t next_ptr = (uint8_t)((cap_header >> 8) & 0xFF);

        if (cap_id == 0x11){
            uint16_t msg_ctrl = (cap_header >> 16) & 0xFFFF;
            uint32_t table_entries = (msg_ctrl & 0x7FF) + 1;
            uint32_t table_size = table_entries * 16;

            uint32_t tbloff_bir = pci_read_dword(bus, dev, func, cap_ptr + 4);
            uint32_t tbl_off = tbloff_bir & ~7;
            uint8_t bar_num = tbloff_bir & 7;

            uint32_t bar_info = pci_read_dword(bus, dev, func, 0x10 + bar_num * 4);
            uint8_t bar_flags = bar_info & 0x0F;
            uint8_t is_64bit = bar_flags & (1 << 2);

            uint64_t bar_address = bar_info & 0xFFFFFFF0;
            if (is_64bit) bar_address |= (uint64_t)(pci_read_dword(bus, dev, func, 0x10 + bar_num * 4 + 4)) << 32;

            uint64_t phys_start = (bar_address + tbl_off) & PAGE_MASK_4KB;
            uint64_t page_offset = (bar_address + tbl_off) & (PAGE_SIZE_4KB - 1);
            uint64_t map_size = (page_offset + table_size + PAGE_SIZE_4KB - 1) & PAGE_MASK_4KB;

            for (uint64_t i = 0; i < map_size; i += PAGE_SIZE_4KB) vmm_map_page(read_cr3(), phys_start + i, MSIX_TABLE_VIRT + i, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_CD | PTE_WT));

            volatile msix_entry_t *table = (volatile msix_entry_t *)(MSIX_TABLE_VIRT + page_offset);
            
            table[msix_vector].msg_addr_lo = lapic_paddr | (lapic_id << 12);
            table[msix_vector].msg_addr_hi = 0x0;
            table[msix_vector].msg_data = idt_vector;

            mfence();
            table[msix_vector].vector_ctrl = 0x0;

            serial_print("[PCI] Enabled MSI-X (Vector: %x)\n", msix_vector);
            return;
        }
        cap_ptr = next_ptr;
    }
    serial_print("[PCI] MSI-X not supported\n");
}