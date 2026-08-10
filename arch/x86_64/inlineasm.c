#include <arch/x86_64/inlineasm.h>

void outb(uint16_t port, uint8_t val){
    __asm__ volatile ("outb %0, %1" : : "a" (val), "dN"(port));
}

uint8_t inb(uint16_t port){
    uint8_t res;
    __asm__ volatile("inb %1, %0" : "=a" (res) : "dN" (port));
    return res;
}

void outl(uint16_t port, uint32_t val){
    __asm__ volatile ("outl %0, %1" : : "a" (val), "dN"(port));
}

uint8_t inl(uint16_t port){
    uint32_t res;
    __asm__ volatile("inl %1, %0" : "=a" (res) : "dN" (port));
    return res;
}

void invalidate(uint64_t vaddr){
    __asm__ volatile ("invlpg (%0)" : : "r" (vaddr) : "memory");
}

uint64_t read_cr3(void){
    uint64_t val;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(val) : : "memory"); 
    return val;
}

void write_cr3(uint64_t paddr){
    __asm__ volatile ("mov %0, %%cr3" : : "r" (paddr) : "memory");
}

void read_msr(uint32_t index, uint32_t *l, uint32_t *h){
    uint32_t low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(index));
    *l = low;
    *h = high;
}

void write_msr(uint32_t index, uint32_t low, uint32_t high){
    __asm__ volatile ("wrmsr" : : "a"(low), "d"(high), "c"(index));
}

uint64_t read_rsp(void){
    uint64_t rsp;
    __asm__ volatile ("mov %%rsp, %0" : "=r"(rsp) : : "memory");
    return rsp; 
}

uint64_t read_rflags(void){
    uint64_t rflags;
    __asm__ volatile ("pushf\n\tpop %0" : "=rm"(rflags) : : "memory");
    return rflags;
}

void write_rflags(uint64_t rflags){
    __asm__ volatile ("push %0\n\tpopf" : : "rm"(rflags) : "memory", "cc");
}

void yield(void){
    __asm__ volatile ("int $0x81" : : : "memory");
}

void sti(void){
    __asm__ volatile ("sti" : : : "memory");
}

void cli(void){
    __asm__ volatile ("cli" : : : "memory");
}

void hlt(void){
    __asm__ volatile ("hlt" : : : "memory");
}
