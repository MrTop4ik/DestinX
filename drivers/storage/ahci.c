#include <drivers/ahci.h>

ahci_controller_t main_ahci;
int ahci_found = 0;

volatile hba_mem_t *ahci_regs = (hba_mem_t *)VIRT_ABAR;

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
                    uint32_t abar = pci_read_dword((uint8_t)bus, dev, func, 0x24);

                    main_ahci.bus = (uint8_t)bus;
                    main_ahci.dev = dev;
                    main_ahci.func = func;
                    main_ahci.abar = abar & 0xFFFFF000;

                    uint32_t command = pci_read_dword((uint8_t)bus, dev, func, 0x04);
                    command |= (1 << 1) | (1 << 2);
                    pci_write_dword((uint8_t)bus, dev, func, 0x04, command);

                    serial_print("[AHCI] Found AHCI Controller\n");
                    ahci_found = 1;

                    map_ahci();
                    probe_ahci_ports();

                    return;
                }
            }
        }
    }
    serial_print("[AHCI] AHCI Controller wasnt found\n");
}

void map_ahci(void){
    vmm_map_page(read_cr3(), main_ahci.abar, VIRT_ABAR, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_WT | PTE_CD));
}

void probe_ahci_ports(void){
    for (int i = 0; i < 32; i++){
        if (ahci_regs->pi & (1 << i)){
            volatile hba_port_t *port = &ahci_regs->ports[i];

            uint8_t det = port->ssts & 0x0F;
            if (det != 0x03) continue;

            switch (port->sig){
                case AHCI_DEV_SATA:
                    serial_print("[AHCI] Found SATA Hard Disk / SDD\n");
                    ahci_init_port(port);
                    main_ahci.main_port = port;
                    break;
                case AHCI_DEV_SATAPI:
                    serial_print("[AHCI] Found SATA CD-ROM\n");
                    break;
                default:
                    serial_print("[AHCI] Found unknown device type\n");
                    break;
            }
        }
    }
}

void ahci_stop_port(volatile hba_port_t *port){
    port->cmd &= ~(1 << 0);
    port->cmd &= ~(1 << 4);

    for (;;){
        if (port->cmd & (1 << 15)) continue;
        if (port->cmd & (1 << 14)) continue;
        break;
    }
}

void ahci_start_port(volatile hba_port_t *port){
    for (;;){
        if (port->tfd & ((1 << 7) | (1 << 3))) continue;
        break;
    }

    port->cmd |= (1 << 0);
    port->cmd |= (1 << 4);
}

void ahci_init_port(volatile hba_port_t *port){
    ahci_stop_port(port);

    uint64_t phys_page = pmm_alloc_page();

    uint64_t phys_cmd_list = phys_page;
    uint64_t phys_rcvd_fis = phys_page + 1024;

    uint64_t virt_page = phys_page + DIRECT_OFFSET;

    memset((void*)virt_page, 0, PAGE_SIZE_4KB);

    port->clb = (phys_cmd_list & 0xFFFFFFFF);
    port->clbu = (phys_cmd_list >> 32);

    port->fb = (phys_rcvd_fis & 0xFFFFFFFF);
    port->fbu = (phys_rcvd_fis >> 32);
    
    ahci_start_port(port);
}

int ahci_read(volatile hba_port_t *port, uint64_t lba, uint32_t seccount, uint64_t *pages, uint32_t page_count){
    if (page_count > AHCI_MAX_PRDT) return -1;

    port->is = 0xFFFFFFFF;

    uint64_t phys_cmd_list = ((uint64_t)port->clbu << 32) | port->clb;
    ahci_cmd_header_t *cmd_header_list = (ahci_cmd_header_t *)(phys_cmd_list  + DIRECT_OFFSET);

    ahci_cmd_header_t *head = &cmd_header_list[0];

    uint64_t phys_cmd_table = pmm_alloc_page();
    ahci_cmd_table_t *cmd_table = (ahci_cmd_table_t *)(phys_cmd_table + DIRECT_OFFSET);
    memset(cmd_table, 0, PAGE_SIZE_4KB);

    head->ctba = (uint32_t)(phys_cmd_table & 0xFFFFFFFF);
    head->ctbau = (uint32_t)(phys_cmd_table >> 32);

    head->cfl = 5;
    head->w = 0;
    head->prdtl = page_count;

    uint64_t bytes_left = seccount * 512;

    for (int i = 0; i < page_count; i++){
        uint32_t chunk_size = (bytes_left > 4096) ? 4096 : bytes_left;

        cmd_table->prdt[i].dba = (uint32_t)(pages[i] & 0xFFFFFFFF);
        cmd_table->prdt[i].dbau = (uint32_t)(pages[i] >> 32);
        cmd_table->prdt[i].dbc = chunk_size - 1;

        if (i == page_count - 1) cmd_table->prdt[i].i = 1;
        else cmd_table->prdt[i].i = 0;
    }

    fis_reg_h2d_t *fis = (fis_reg_h2d_t *)(cmd_table->cfis);
    fis->fis_type = 0x27;
    fis->c = 1;
    fis->command = ATA_CMD_READ_DMA_EXT;
    fis->device = (1 << 6);

    fis->lba0 = (uint8_t)(lba & 0xFF);
    fis->lba1 = (uint8_t)((lba >> 8) & 0xFF);
    fis->lba2 = (uint8_t)((lba >> 16) & 0xFF);
    fis->lba3 = (uint8_t)((lba >> 24) & 0xFF);
    fis->lba4 = (uint8_t)((lba >> 32) & 0xFF);
    fis->lba5 = (uint8_t)((lba >> 40) & 0xFF);

    fis->countl = (uint8_t)(seccount & 0xFF);
    fis->counth = (uint8_t)((seccount >> 8) & 0xFF);

    port->ci = (1 << 0);

    int status = 0;

    for (;;){
        if ((port->ci & (1 << 0)) == 0) break;

        if (port->is & (1 << 30)){
            serial_print("[AHCI] DMA read error");
            status = -1;
            break;
        }
    }

    pmm_free_page(phys_cmd_table);

    return status;
}