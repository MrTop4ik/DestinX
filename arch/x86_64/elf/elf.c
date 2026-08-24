#include <arch/x86_64/elf.h>

Elf64_Addr load_elf(uint8_t *elf_file_data){
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf_file_data;

    if (ehdr->e_ident[0] != ELFMAG0 ||
        ehdr->e_ident[1] != ELFMAG1 ||
        ehdr->e_ident[2] != ELFMAG2 ||
        ehdr->e_ident[3] != ELFMAG3 ||
        ehdr->e_ident[4] != ELFCLASS64
    ) return 0;
    
    Elf64_Phdr * phdr = (Elf64_Phdr *)(elf_file_data + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++){
        if (phdr[i].p_type == PT_LOAD){
            uint64_t pages_needed = (phdr[i].p_memsz + PAGE_SIZE_4KB - 1) / PAGE_SIZE_4KB;
            uint64_t paddr = pmm_alloc_pages(pages_needed);
            for (int j = 0; j < pages_needed; j++) vmm_map_page(read_cr3(), paddr + (j * PAGE_SIZE_4KB), phdr[i].p_vaddr + (j * PAGE_SIZE_4KB), PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));

            if (phdr[i].p_filesz > 0) memcpy((void*)phdr[i].p_vaddr, (void*)(elf_file_data + phdr[i].p_offset), phdr[i].p_filesz);
            if (phdr[i].p_memsz > phdr[i].p_filesz) memset((void*)(phdr[i].p_vaddr + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
        }
    }

    return ehdr->e_entry;
}