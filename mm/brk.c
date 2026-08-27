#include <mm/brk.h>

uint64_t brk(uint64_t addr){
    if (addr < current_thread->process->heap_start) return current_thread->process->current_heap_end;
    uint64_t saddr = current_thread->process->current_heap_end;
    current_thread->process->current_heap_end = addr;
    return saddr;
}