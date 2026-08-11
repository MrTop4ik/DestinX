#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/drivers/pci.h>

#define VIRT_ABAR 0xFFFFF00000000000

#define AHCI_DEV_SATA   0x00000101
#define AHCI_DEV_SATAPI 0xEB140101
#define AHCI_DEV_SEMB   0xC33C0101
#define AHCI_DEV_PM     0x96690101

typedef struct {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    uint64_t abar;
} ahci_controller_t;

typedef struct {
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t rsv0;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sctl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t fbs;
    uint32_t rsv1[11];
    uint32_t vendor[4];
} __attribute__((packed)) hba_port_t;

typedef struct {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
    uint32_t ccc_ctl;
    uint32_t ccc_pts;
    uint32_t em_loc;
    uint32_t em_ctl;
    uint32_t cap2;
    uint32_t bohc;
    uint8_t  rsv[116];  
    uint8_t  vendor[96];
    hba_port_t ports[32];
} __attribute__((packed)) hba_mem_t;

void init_ahci(void);
void map_ahci(void);
void probe_ahci_ports(void);