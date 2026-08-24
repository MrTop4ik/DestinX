#include <kernel/scheduler/process.h>

void dfs_read(const char *fp, uint8_t *buffer, uint64_t offset, uint64_t size);
size_t dfs_get_size(const char *path);
struct thread *create_user_thread(struct process *proc, void (*entry_point)(void), size_t kstack_size, size_t ustack_size);

const char user_exit_trampoline[10] = {
    0xB8, 0x3C, 0x00, 0x00, 0x00, 
    0x48, 0x31, 0xFF, 
    0x0F, 0x05
};

uint64_t next_process_id = 1;

process_t *create_user_process(const char *fp){
    process_t *p = (process_t *)kmalloc(sizeof(process_t));
    if (!p) return NULL;

    memset(p, 0, sizeof(process_t));

    uint64_t pml4_phys = pmm_alloc_page();
    if (!pml4_phys){ kfree(p); return NULL; }

    uint64_t *old_pml4 = (uint64_t *)(read_cr3() + DIRECT_OFFSET);
    uint64_t *pml4 = (uint64_t *)(pml4_phys + DIRECT_OFFSET);

    for (int i = 0; i < 256; i++) pml4[i] = 0;
    for (int i = 256; i < 512; i++) pml4[i] = old_pml4[i];

    p->pml4 = pml4_phys;
    p->pid = next_process_id++;

    uint64_t old_pml4_phys = read_cr3();
    
    size_t size = dfs_get_size(fp);
    uint64_t pages_needed = (size + PAGE_SIZE_4KB - 1) / PAGE_SIZE_4KB;

    uint64_t phys_buf = pmm_alloc_pages(pages_needed);
    uint8_t *virt_buf = (uint8_t *)(phys_buf + DIRECT_OFFSET);

    dfs_read(fp, virt_buf, 0, size);
    
    write_cr3(p->pml4);

    vmm_map_page(read_cr3(), pmm_alloc_page(), 0xffffffffffff0000, PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));
    memcpy((void*)0xffffffffffff0000, user_exit_trampoline, sizeof(user_exit_trampoline));

    uint64_t entry = load_elf(virt_buf);
    if (!entry){
        entry = 0x400000;
        uint64_t pages_needed = (size + PAGE_SIZE_4KB - 1) / PAGE_SIZE_4KB;
        uint64_t first_phys = pmm_alloc_pages(pages_needed);
        for (int i = 0; i < pages_needed; i++) vmm_map_page(read_cr3(), first_phys + (i * PAGE_SIZE_4KB), entry + (i * PAGE_SIZE_4KB), PAGE_SIZE_4KB, (PTE_WRITABLE | PTE_USER));
        memcpy((void*)entry, virt_buf, size);
    }

    write_cr3(old_pml4_phys);

    create_user_thread(p, (void*)entry, PAGE_SIZE_2MB, PAGE_SIZE_2MB);

    serial_print("[PROCESS] User Process was created\n");

    return p;
}

