#include <arch/x86_64/drivers/pci.h>

void pci_enable_msi(uint8_t bus, uint8_t dev, uint8_t func, uint8_t vector, uint8_t lapic_id){
    uint32_t cap_reg = pci_read_dword(bus, dev, func, 0x34);
    uint8_t cap_ptr = cap_reg & 0xFF;

    while (cap_ptr != 0){
        uint32_t cap_header = pci_read_dword(bus, dev, func, cap_ptr);
        uint8_t cap_id = (uint8_t)(cap_header & 0xFF);
        uint8_t next_ptr = (uint8_t)((cap_header >> 8) & 0xFF);

        if (cap_id == 0x05){
            serial_print("[PCI] Found MSI Capability\n");

            uint16_t msg_ctrl_reg = (uint16_t)((cap_header >> 16) & 0xFFFF);

            int is_64bit = (msg_ctrl_reg & (1 << 7)) ? 1 : 0;

            uint32_t msg_addr = lapic_paddr | ((uint32_t)lapic_id << 12);

            pci_write_dword(bus, dev, func, cap_ptr + 4, msg_addr);

            if (is_64bit){
                pci_write_dword(bus, dev, func, cap_ptr + 8, 0);
                pci_write_dword(bus, dev, func, cap_ptr + 12, (uint32_t)vector);
            } else {
                pci_write_dword(bus, dev, func, cap_ptr + 8, (uint32_t)vector);
            }

            cap_header |= (1 << 16);
            pci_write_dword(bus, dev, func, cap_ptr, cap_header);

            serial_print("[PCI] Enabled MSI\n");
            return;
        }
        cap_ptr = next_ptr;
    }
    serial_print("[PCI] MSI not supported\n");
}