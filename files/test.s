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
    mov rsi, 0x2
    syscall

    mov rbx, rax

    mov rax, 1
    mov rdi, rbx
    mov rsi, reversed_hw
    mov rdx, 13
    push rbx
    syscall

    pop rbx

    mov rax, 8
    mov rdi, rbx
    mov rsi, 0
    mov rdx, 0
    push rbx
    syscall

    pop rbx

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

    mov rax, 9
    mov rdi, 0
    mov rsi, 0x10000
    mov rdx, (0x1 | 0x2)
    mov r10, 0x2 | 0x20
    mov r8, -1
    mov r9, 0
    syscall

    mov qword [rax], 24

    mov rbx, rax

    mov rax, 11
    mov rdi, rbx
    syscall

    jmp $

section .rodata
fp: db "/usr/txt/test.txt", 0
reversed_hw: db "!dlroW ,olleH", 0