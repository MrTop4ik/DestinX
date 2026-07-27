#include <mm/vmm.h>

#define MSR_IA32_PAT 0x277

void init_VMM(unsigned int bootInfoAddr){
    struct multiboot_info* virtBootInfo = (struct multiboot_info *)(bootInfoAddr + KERNEL_OFFSET);
    uint64_t maxAddr = 0;

    struct multiboot_tag* tag = (struct multiboot_tag *)((uint8_t *)virtBootInfo + 8);

    while (tag->type != MULTIBOOT_TAG_TYPE_END){
        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP){
            struct multiboot_tag_mmap* mmap = (struct multiboot_tag_mmap *)tag;
            uint64_t num_entries = (mmap->size - sizeof(struct multiboot_tag_mmap)) / mmap->entry_size;
            for (uint64_t i = 0; i < num_entries; i++){
                uint64_t end = mmap->entries[i].addr + mmap->entries[i].len;
                if (end > maxAddr){
                    maxAddr = end;
                }
            }
        }
        tag = (struct multiboot_tag *)((uintptr_t)((uint8_t*)tag + tag->size + 7) & ~7);
    }

    vmm_init_direct_mapping(maxAddr);
    init_pat();
}

void vmm_init_direct_mapping(uint64_t maxAddr){
    uint64_t pml4_phys = pmm_alloc_page();
    uint64_t *pml4 = (uint64_t *)(pml4_phys + KERNEL_OFFSET);
    memset(pml4, 0, PAGE_SIZE_4KB);

    uint64_t* old_pml4 = (uint64_t *)(read_cr3() + KERNEL_OFFSET);
    pml4[511] = old_pml4[511];

    for (uint64_t paddr = 0; paddr < maxAddr; paddr += PAGE_SIZE_2MB){
        uint64_t vaddr = paddr + DIRECT_OFFSET;

        uint64_t pml4_indx = (vaddr >> 39) & 0x1FF;
        uint64_t pdpt_indx = (vaddr >> 30) & 0x1FF;
        uint64_t pd_indx = (vaddr >> 21) & 0x1FF;

        if (!(pml4[pml4_indx] & PTE_PRESENT)){
            uint64_t new_table_phys = pmm_alloc_page();
            uint64_t *new_table = (uint64_t *)(new_table_phys + KERNEL_OFFSET);
            memset(new_table, 0, PAGE_SIZE_4KB);
            pml4[pml4_indx] = new_table_phys | (PTE_PRESENT | PTE_WRITABLE);
        }

        uint64_t *pdpt = (uint64_t *)((pml4[pml4_indx] & PAGE_MASK_4KB) + KERNEL_OFFSET);

        if (!(pdpt[pdpt_indx] & PTE_PRESENT)){
            uint64_t new_table_phys = pmm_alloc_page();
            uint64_t *new_table = (uint64_t *)(new_table_phys + KERNEL_OFFSET);
            memset(new_table, 0, PAGE_SIZE_4KB);
            pdpt[pdpt_indx] = new_table_phys | (PTE_PRESENT | PTE_WRITABLE);
        }

        uint64_t *pd = (uint64_t *)((pdpt[pdpt_indx] & PAGE_MASK_4KB) + KERNEL_OFFSET);

        pd[pd_indx] = paddr | (PTE_PRESENT | PTE_WRITABLE | PTE_PAGE_SIZE);
    }

    write_cr3(pml4_phys);
}

void vmm_map_page(uint64_t pml4_phys, uint64_t paddr, uint64_t vaddr, uint64_t ps, uint64_t flags){
    uint64_t pml4_indx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_indx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_indx = (vaddr >> 21) & 0x1FF;
    

    uint64_t *pml4 = (uint64_t *)(pml4_phys + DIRECT_OFFSET);

    if (!(pml4[pml4_indx] & PTE_PRESENT)){
        uint64_t new_table_phys = pmm_alloc_page();
        uint64_t *new_table = (uint64_t *)(new_table_phys + DIRECT_OFFSET);
        memset(new_table, 0, PAGE_SIZE_4KB);
        if (flags & PTE_USER) pml4[pml4_indx] = new_table_phys | (PTE_PRESENT | PTE_WRITABLE | PTE_USER);
        else pml4[pml4_indx] = new_table_phys | (PTE_PRESENT | PTE_WRITABLE);
    } else {
        if (flags & PTE_USER) pml4[pml4_indx] |= PTE_USER;
    }

    uint64_t *pdpt = (uint64_t *)((pml4[pml4_indx] & PAGE_MASK_4KB) + DIRECT_OFFSET);

    if (!(pdpt[pdpt_indx] & PTE_PRESENT)){
        uint64_t new_table_phys = pmm_alloc_page();
        uint64_t *new_table = (uint64_t *)(new_table_phys + DIRECT_OFFSET);
        memset(new_table, 0, PAGE_SIZE_4KB);
        if (flags & PTE_USER) pdpt[pdpt_indx] = new_table_phys | (PTE_PRESENT | PTE_WRITABLE | PTE_USER);
        else pdpt[pdpt_indx] = new_table_phys | (PTE_PRESENT | PTE_WRITABLE);
    } else {
        if (flags & PTE_USER) pdpt[pdpt_indx] |= PTE_USER;
    }

    uint64_t *pd = (uint64_t *)((pdpt[pdpt_indx] & PAGE_MASK_4KB) + DIRECT_OFFSET);

    if (ps == PAGE_SIZE_4KB){
        uint64_t pt_indx = (vaddr >> 12) & 0x1FF;

        if (!(pd[pd_indx] & PTE_PRESENT)){
            uint64_t new_table_phys = pmm_alloc_page();
            uint64_t *new_table = (uint64_t *)(new_table_phys + DIRECT_OFFSET);
            memset(new_table, 0, PAGE_SIZE_4KB);
            if (flags & PTE_USER) pd[pd_indx] = new_table_phys | (PTE_PRESENT | PTE_WRITABLE | PTE_USER);
            else pd[pd_indx] = new_table_phys | (PTE_PRESENT | PTE_WRITABLE);
        } else {
            if (flags & PTE_USER) pd[pd_indx] |= PTE_USER;
        }

        uint64_t *pt = (uint64_t *)((pd[pd_indx] & PAGE_MASK_4KB) + DIRECT_OFFSET);

        pt[pt_indx] = paddr | (PTE_PRESENT | flags);

    } else if (ps == PAGE_SIZE_2MB){

        pd[pd_indx] = paddr | (PTE_PRESENT | PTE_PAGE_SIZE | flags);

    } else {

        serial_print("Wrong Page Size.\n");
        return;

    }

    invalidate(vaddr);
}

uint64_t vmm_unmap_page(uint64_t pml4_phys, uint64_t vaddr){
    int level;
    void *entry = vmm_get_entry(pml4_phys, vaddr, &level);

    if (entry){
        uint64_t old_entry = *(uint64_t*)entry;
        uint64_t paddr = old_entry & PAGE_MASK_4KB;

        *(uint64_t*)entry = 0;
        invalidate(vaddr);

        return paddr;  
    }

    return 0;
}

void *vmm_get_entry(uint64_t pml4_phys, uint64_t vaddr, int *level){
    uint64_t pml4_indx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_indx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_indx = (vaddr >> 21) & 0x1FF;
    uint64_t pt_indx = (vaddr >> 12) & 0x1FF;

    uint64_t *pml4 = (uint64_t *)(pml4_phys + DIRECT_OFFSET);

    if (!(pml4[pml4_indx] & PTE_PRESENT)) return NULL;
    if (pml4[pml4_indx] & PTE_PAGE_SIZE){
        *level = 3;
        return &pml4[pml4_indx];
    }

    uint64_t *pdpt = (uint64_t *)((pml4[pml4_indx] & PAGE_MASK_4KB) + DIRECT_OFFSET);

    if (!(pdpt[pdpt_indx] & PTE_PRESENT)) return NULL;
    if (pdpt[pdpt_indx] & PTE_PAGE_SIZE){
        *level = 2;
        return &pdpt[pdpt_indx];
    }

    uint64_t *pd = (uint64_t *)((pdpt[pdpt_indx] & PAGE_MASK_4KB) + DIRECT_OFFSET);

    if (!(pd[pd_indx] & PTE_PRESENT)) return NULL;
    if (pd[pd_indx] & PTE_PAGE_SIZE){
        *level = 2;
        return &pd[pd_indx];
    }

    uint64_t *pt = (uint64_t *)((pd[pd_indx] & PAGE_MASK_4KB) + DIRECT_OFFSET);

    *level = 0;
    return &pt[pt_indx];
}

void init_pat(void){
    uint32_t low, high;

    read_msr(MSR_IA32_PAT, &low, &high);

    high = (high & ~0xFF) | 0x06;

    write_msr(MSR_IA32_PAT, low, high);
}