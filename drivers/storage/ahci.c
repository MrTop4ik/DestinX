#include <drivers/ahci.h>

ahci_controller_t main_ahci;
int ahci_found = 0;

void init_ahci(void){
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t reg_id = pci_read_dword((uint8_t)bus, dev, func, 0x00);
                uint16_t vendor_id = reg_id & 0xFFFF;

                if (vendor_id == 0xFFFF) {
                    if (func == 0) break;
                    continue;
                }

                uint32_t class_reg = pci_read_dword((uint8_t)bus, dev, func, 0x08);
                uint8_t class    = (class_reg >> 24) & 0xFF;
                uint8_t subclass = (class_reg >> 16) & 0xFF;
                uint8_t prog_if  = (class_reg >> 8)  & 0xFF;

                if (class == 0x01 && subclass == 0x06 && prog_if == 0x01) {
                    uint32_t bar5 = pci_read_dword((uint8_t)bus, dev, func, 0x24);

                    main_ahci.bus = (uint8_t)bus;
                    main_ahci.dev = dev;
                    main_ahci.func = func;
                    main_ahci.bar5 = bar5 & 0xFFFFF000;

                    uint32_t command = pci_read_dword((uint8_t)bus, dev, func, 0x04);
                    command |= (1 << 1) | (1 << 2);
                    pci_write_dword((uint8_t)bus, dev, func, 0x04, command);

                    serial_print("[AHCI] Found AHCI Controller\n");
                    ahci_found = 1;                
                    return;
                }
            }
        }
    }
    serial_print("[AHCI] AHCI Controller wasnt found\n");
}