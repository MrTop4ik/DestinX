bits 64

section .text
global _start

_start:
    mov rax, 12
    mov rdi, 0x402000
    syscall
    mov qword [rax], 24

    mov rax, 0x00007fffffffd060
    mov qword [rax], 24

    mov rax, 2
    mov rdi, fp
    syscall

    mov rbx, rax

    mov rax, 0
    mov rdi, rbx
    mov rsi, 0x401000
    mov rdx, 13
    push rbx
    syscall

    mov rax, 1
    mov rdi, 2
    mov rsi, 0x401000
    mov rdx, 13
    syscall

    pop rbx

    mov rax, 3
    mov rdi, rbx
    syscall

    jmp $

section .rodata
fp: db "/usr/txt/test.txt", 0