bits 64

section .text
global _start

_start:
    mov rax, 12
    mov rdi, 0x402000
    syscall
    mov qword [rax], 24

    mov rax, 9
    mov rdi, 0x10000
    syscall
    mov qword [rax], 42

    jmp $