#pragma once
#include <stdint.h>
#include <multiboot2.h>
#include <libc/string.h>
#include <arch/x86_64/drivers/serial.h>
#include <arch/x86_64/inlineasm.h>
#include <mm/pmm.h>

#define KERNEL_OFFSET 0xFFFFFFFF80000000
#define DIRECT_OFFSET 0xFFFF800000000000

#define PAGE_SIZE_4KB 0x1000
#define PAGE_MASK_4KB (~(PAGE_SIZE_4KB - 1))
#define PAGE_SIZE_2MB 0x200000
#define PAGE_MASK_2MB (~(PAGE_SIZE_2MB - 1))

#define PTE_PRESENT   (1ULL << 0)
#define PTE_WRITABLE  (1ULL << 1)
#define PTE_USER      (1ULL << 2)
#define PTE_PAGE_SIZE (1ULL << 7)
#define PTE_WC_4KB    (1ULL << 7)
#define PTE_WC_HUGE   (1ULL << 12)
#define PTE_CD        (1ULL << 4)

void init_VMM(unsigned int bootInfoAddr);
void vmm_init_direct_mapping(uint64_t maxAddr);
void vmm_map_page(uint64_t pml4_phys, uint64_t paddr, uint64_t vaddr, uint64_t ps, uint64_t flags);
uint64_t vmm_unmap_page(uint64_t pml4_phys, uint64_t vaddr);
void *vmm_get_entry(uint64_t pml4_phys, uint64_t vaddr, int *level);
void init_pat(void);