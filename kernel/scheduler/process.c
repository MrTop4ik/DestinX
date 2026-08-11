#include <kernel/scheduler/process.h>

uint64_t next_process_id = 1;

process_t *create_user_process(){
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

    serial_print("[PROCESS] User Process was created\n");

    return p;
}

