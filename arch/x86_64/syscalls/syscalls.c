#include <arch/x86_64/syscalls.h>

extern void syscall_entry(void);

syscalls_stacks_t sstacks;

void init_syscalls(void){
    uint32_t low;
    uint32_t high;
    read_msr(IA32_EFER, &low, &high);
    uint64_t efer = (uint64_t)high << 32 | low;
    efer |= 1;
    write_msr(IA32_EFER, (uint32_t)efer, (uint32_t)(efer >> 32));

    uint64_t star = ((uint64_t)0x10 << 48) | ((uint64_t)0x08 << 32);
    write_msr(IA32_STAR, (uint32_t)star, (uint32_t)(star >> 32));

    write_msr(IA32_LSTAR, (uint32_t)&syscall_entry, (uint32_t)((uint64_t)&syscall_entry >> 32));

    uint64_t mflags = FLAGS_IF | FLAGS_DF | FLAGS_TF;
    write_msr(IA32_FMASK, (uint32_t)mflags, (uint32_t)(mflags >> 32));

    init_kernel_gs_base();
}

void init_kernel_gs_base(void){
    write_msr(IA32_KERNEL_GS_BASE, (uint32_t)&sstacks, (uint32_t)((uint64_t)&sstacks >> 32));
}

void syscall_handler(uint64_t sys_num, struct SyscallRegisters *regs){
    switch (sys_num){
        case SYS_EXIT:
            serial_print("[THREAD %d] SYS EXIT\n", current_thread->tid);
            thread_exit();
            break;

        case SYS_EXIT_GROUP:
            serial_print("[THREAD %d] SYS EXIT GROUP\n", current_thread->tid);
            process_t *p = current_thread->process;
            thread_t *t = p->threads;
            while (t){
                t->state = DEAD;
                t->next = dead_list_head;
                dead_list_head = t;
                if (t == current_thread) continue;
                dequeue_thread(t);
                t = t->next_pthread;
            }
            yield();
    }
}