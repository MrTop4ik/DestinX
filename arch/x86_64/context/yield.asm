bits 64
section .text

extern scheduler_handler
extern lapic_timer_reload_oneshot
global yield_handler
yield_handler:
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov rdi, rsp
    
    call scheduler_handler

    mov rsp, rax

    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15

    push rdi

    mov rdi, 1
    call lapic_timer_reload_oneshot

    pop rdi

    iretq