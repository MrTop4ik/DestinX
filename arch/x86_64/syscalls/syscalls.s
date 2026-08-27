bits 64
section .text

global syscall_entry
syscall_entry:
    swapgs

    mov [gs:00], rsp
    mov rsp, [gs:08]

	push r11
    push rcx

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
    push rbx
    push rax

    mov rdi, rsp
    
    extern syscall_handler
    call syscall_handler

    pop rax
    pop rbx
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
    
    pop rcx
    pop r11

    mov rbx, [gs:00]

    push 0x1B
    push rbx
    push r11
    push 0x23
    push rcx

    swapgs

    iretq
