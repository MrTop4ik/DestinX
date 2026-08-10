#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/drivers/pci.h>

typedef struct {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    uint64_t bar5;
} ahci_controller_t;

void init_ahci(void);