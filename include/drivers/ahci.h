#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/drivers/pci.h>
#include <kernel/mutex.h>
#include <kernel/scheduler/thread.h>

#define VIRT_ABAR 0xFFFFF00000000000

#define AHCI_DEV_SATA   0x00000101
#define AHCI_DEV_SATAPI 0xEB140101
#define AHCI_DEV_SEMB   0xC33C0101
#define AHCI_DEV_PM     0x96690101

#define AHCI_MAX_PRDT 16

#define ATA_CMD_READ_DMA_EXT 0x25

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
    uint8_t rsv[116];  
    uint8_t vendor[96];
    hba_port_t ports[32];
} __attribute__((packed)) hba_mem_t;

typedef struct {
    uint8_t cfl:5;
    uint8_t a:1;
    uint8_t w:1;
    uint8_t p:1;
    uint8_t r:1;
    uint8_t b:1;
    uint8_t c:1;
    uint8_t rsv0:1;
    uint8_t pmp:4;
    uint16_t prdtl;
    volatile uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t rsv1[4];
}__attribute__((packed)) ahci_cmd_header_t;

typedef struct {
    uint32_t dba;
    uint32_t dbau;
    uint32_t rsv0;
    uint32_t dbc:22;
    uint32_t rsv1:9;
    uint32_t i:1;
}__attribute__((packed)) ahci_prdt_entry_t;

typedef struct {
    uint8_t fis_type;
    uint8_t pmport:4;
    uint8_t rsv0:3;
    uint8_t c:1;
    uint8_t command;
    uint8_t featurel;
    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;
    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureh;
    uint8_t countl;
    uint8_t counth;
    uint8_t icc;
    uint8_t control;
    uint32_t rsv1;
}__attribute__((packed)) fis_reg_h2d_t;

typedef struct {
    uint8_t cfis[64];
    uint8_t acmd[16];
    uint8_t rsv[48];
    ahci_prdt_entry_t prdt[AHCI_MAX_PRDT];
}__attribute__((packed)) ahci_cmd_table_t;

typedef struct {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    uint64_t abar;

    mutex_t mutex;
    thread_t *blcoked_thread;
} ahci_controller_t;

extern ahci_controller_t main_ahci;
extern volatile hba_mem_t *ahci_regs;

void init_ahci(void);
void map_ahci(void);
void probe_ahci_ports(void);
void ahci_stop_port(volatile hba_port_t *port);
void ahci_start_port(volatile hba_port_t *port);
void ahci_init_port(volatile hba_port_t *port);
int ahci_read(volatile hba_port_t *port, uint64_t lba, uint32_t seccount, uint64_t *pages, uint32_t page_count);
void ahci_handler(struct InterruptRegisters *regs);