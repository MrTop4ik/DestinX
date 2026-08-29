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

    mov rbx, rax

    mov rax, 11
    mov rdi, rbx
    syscall

    mov rax, 1
    mov rdi, 2
    mov rsi, msg
    mov rdx, 15
    syscall

    mov rax, 2
    mov rdi, fp
    syscall

    mov rbx, rax

    mov rax, 3
    mov rdi, rbx
    syscall

    jmp $

section .rodata
msg: db "Hello, World!", 10, 0
fp: db "/usr/txt/test.txt", 0