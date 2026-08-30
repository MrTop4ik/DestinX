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

void syscall_handler(struct SyscallRegisters *regs){
    switch (regs->rax){
        case SYS_READ:
            struct FILE *file = current_thread->process->fd_table[regs->rdi];
            int bytes_read = file->ops->read(file, (const char *)regs->rsi, regs->rdx);
            regs->rax = bytes_read;
            break;

        case SYS_WRITE:
            file = current_thread->process->fd_table[regs->rdi];
            uint64_t count = file->ops->write(file, (const char *)regs->rsi, regs->rdx);
            regs->rax = count;
            break;
        
        case SYS_OPEN:
            uint64_t fd = open((const char *)regs->rdi);
            regs->rax = fd;
            break;
        
        case SYS_CLOSE:
            int status = close(regs->rdi);
            regs->rax = status;
            break;

        case SYS_MMAP:
            uint64_t maddr = mmap(regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9);
            regs->rax = maddr;
            break;
        
        case SYS_MUNMAP:
            munmap((void*)regs->rdi);
            break;

        case SYS_BRK:
            uint64_t baddr = brk(regs->rdi);
            regs->rax = baddr;
            break;

        case SYS_EXIT:
            serial_print("[THREAD %d] SYS EXIT (Exit code: %d)\n", current_thread->tid, regs->rdi);
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
            break;

        default:
            serial_print("[SYSCALLS] No Such Syscall (%d)\n", regs->rax);
            regs->rax = (uint64_t)-38;
            break;
    }
}