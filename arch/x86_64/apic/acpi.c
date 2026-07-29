#include <arch/x86_64/apic/acpi.h>

uint64_t lapic_paddr, ioapic_paddr;
int iso_count = 0;
struct MADT_ISO iso_list[MAX_ISO];

void parse_acpi(unsigned int physBootInfo){
    struct multiboot_info* virtBootInfo = (struct multiboot_info *)(physBootInfo + DIRECT_OFFSET);

    struct multiboot_tag* tag = (struct multiboot_tag *)((uint8_t *)virtBootInfo + 8);

    struct MADT *madt;
    while (tag->type != MULTIBOOT_TAG_TYPE_END){
        if (tag->type == MULTIBOOT_TAG_TYPE_ACPI_NEW){
            struct multiboot_tag_new_acpi *acpi = (struct multiboot_tag_new_acpi *)tag;
            struct RSDPDescriptor20 *rsdp = (struct RSDPDescriptor20 *)(acpi->rsdp);
            madt = find_madt(rsdp->XsdtAddress, 1);
            serial_print("[ACPI] ACPI TAG NEW was found\n");
            break;
        } else if (tag->type == MULTIBOOT_TAG_TYPE_ACPI_OLD){
            struct multiboot_tag_old_acpi *acpi = (struct multiboot_tag_old_acpi *)tag;
            struct RSDPDescriptor *rsdp = (struct RSDPDescriptor *)(acpi->rsdp);
            madt = find_madt(rsdp->RsdtAddress, 0);
            serial_print("[ACPI] ACPI TAG OLD was found\n");
            break;
        }
        tag = (struct multiboot_tag *)((uintptr_t)((uint8_t*)tag + tag->size + 7) & ~7);
    }
    parse_madt(madt);
}

struct MADT *find_madt(uint64_t sdt_phys, uint8_t is_xsdt){
    struct ACPISDTHeader *sdt = (struct ACPISDTHeader *)(sdt_phys + DIRECT_OFFSET);

    int entry_size = is_xsdt ? 8 : 4;
    int entries = (sdt->Length - sizeof(struct ACPISDTHeader)) / entry_size;
    uint8_t *ptrs = (uint8_t *)sdt + sizeof(struct ACPISDTHeader);

    for (int i = 0; i < entries; i++){
        uint64_t paddr;

        if (is_xsdt) paddr = ((uint64_t *)ptrs)[i];
        else paddr = ((uint32_t*)ptrs)[i];

        struct ACPISDTHeader *h = (struct ACPISDTHeader *)(paddr + DIRECT_OFFSET);
        if (memcmp(h->Signature, "APIC", 4) == 0){
            return (struct MADT *)h;
        }
    }

    return NULL;
}

void parse_madt(struct MADT *madt){
    uint8_t *ptr = (uint8_t *)madt + sizeof(struct MADT);
    uint8_t *end = (uint8_t *)madt + madt->Header.Length;

    lapic_paddr = madt->LocalAPICAddress;
    
    while (ptr < end){
        struct MADTEntryHeader *entry = (struct MADTEntryHeader *)ptr;

        switch (entry->Type){
            case 0:
                break;
            case 1:
                struct MADT_IOAPIC *ioapic = (struct MADT_IOAPIC *)entry;
                ioapic_paddr = ioapic->IOApicAddress;
                break;
            case 2:
                struct MADT_ISO *iso = (struct MADT_ISO *)entry;
                save_iso(iso);
                break;
        }

        ptr += entry->Length;
    }
}

void save_iso(struct MADT_ISO *iso){
    if (iso_count < MAX_ISO) iso_list[iso_count++] = *iso;
    else kprintf("To many ISO.\n");
}