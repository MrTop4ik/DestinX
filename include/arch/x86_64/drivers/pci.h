#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mm/vmalloc.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/apic/acpi.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define MSIX_TABLE_VIRT 0xFFFFFFFFFFFB1000

typedef struct {
    uint32_t msg_addr_lo;
    uint32_t msg_addr_hi;
    uint32_t msg_data;
    uint32_t vector_ctrl;
} msix_entry_t;

void pci_write_dword(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value);
uint32_t pci_read_dword(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset);
void pci_enable_msi(uint8_t bus, uint8_t dev, uint8_t func, uint8_t vector, uint8_t lapic_id);
void pci_enable_bus_master(uint8_t bus, uint8_t dev, uint8_t func);
void pci_enable_msix(uint8_t bus, uint8_t dev, uint8_t func);
void pci_set_msix_vector(uint8_t bus, uint8_t dev, uint8_t func, uint8_t idt_vector, uint16_t msix_vector, uint8_t lapic_id);