#include <arch/x86_64/syscalls.h>

extern void syscall_entry(void);

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
}