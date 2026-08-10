#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>
#include <arch/x86_64/drivers/serial.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

void pci_write_dword(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value);
uint32_t pci_read_dword(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);