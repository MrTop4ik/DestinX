#pragma once
#include <stdint.h>
#include <stddef.h>
#include <drivers/lfb.h>
#include <arch/x86_64/inlineasm.h>
#include <libc/string.h>
#include <mm/kmalloc.h>
#include <multiboot2.h>

#define MAX_ISO 16

struct RSDPDescriptor {
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
}__attribute__((packed));

struct RSDPDescriptor20 {
    struct RSDPDescriptor Base;
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t Reserved[3];
}__attribute__((packed));

struct ACPISDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableD[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
}__attribute__((packed));

struct MADT {
    struct ACPISDTHeader Header;
    uint32_t LocalAPICAddress;
    uint32_t Flags;
}__attribute__((packed));

struct MADTEntryHeader {
    uint8_t Type;
    uint8_t Length;
}__attribute__((packed));

struct MADT_LocalAPIC {
    struct MADTEntryHeader Header;
    uint8_t ProcessorID;
    uint8_t ApicID;
    uint32_t Flags;
}__attribute__((packed));

struct MADT_IOAPIC {
    struct MADTEntryHeader Header;
    uint8_t IOApicID;
    uint8_t Reserved;
    uint32_t IOApicAddress;
    uint32_t GlobalSystemInterruptBase;
}__attribute__((packed));

struct MADT_ISO {
    struct MADTEntryHeader Header;
    uint8_t BusSource;
    uint8_t IRQSource;
    uint32_t GSI;
    uint16_t Flags;
}__attribute__((packed));

extern uint64_t lapic_paddr;
extern uint64_t ioapic_paddr;
extern int iso_count;
extern struct MADT_ISO iso_list[MAX_ISO];

void parse_acpi(unsigned int physBootInfo);
struct MADT *find_madt(uint64_t sdt_phys, uint8_t is_xsdt);
void parse_madt(struct MADT *madt);
void save_iso(struct MADT_ISO *iso);