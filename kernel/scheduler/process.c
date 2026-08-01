#include <kernel/scheduler/process.h>

uint64_t next_process_id = 1;

process_t *create_user_process(void (*entry_point)(void), size_t kstack_size, size_t ustack_size){
    process_t *p = (process_t*)kmalloc(sizeof(process_t));
    if (!p) return NULL;
    memset(p, 0, sizeof(process_t));

    uint64_t pml4_phys = pmm_alloc_page();

    uint64_t *pml4 = (uint64_t*)(pml4_phys + DIRECT_OFFSET);
    memset(pml4, 0, PAGE_SIZE_4KB);

    uint64_t old_pml4_phys = read_cr3();
    uint64_t *old_pml4 = (uint64_t*)(old_pml4_phys + DIRECT_OFFSET);

    for (uint64_t i = 0; i < 512; i++) pml4[i] = old_pml4[i];
    
    write_cr3(pml4_phys);

    us_list_head = p->ustacks_infos;
    thread_t *t = create_user_thread(entry_point, kstack_size, ustack_size);
    p->ustacks_infos = us_list_head;

    p->threads = t;
    
    p->pid = next_process_id++;
}

