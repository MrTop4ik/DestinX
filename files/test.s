bits 64

section .text
global _start

_start:
    mov rax, 12
    mov rdi, 0x402000
    syscall
    mov qword [rax], 24
    jmp $