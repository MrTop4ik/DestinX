bits 64
section .text

global check_pcid
check_pcid:
    push rbx

    mov rax, 1
    cpuid

    bt rcx, 17
    jnc .no_support

    mov rax, 1
    jmp .exit

.no_support:
    mov rax, 0

.exit:
    pop rbx
    ret

global enable_pcid
enable_pcid:
    mov rax, cr4
    or rax, (1 << 17)
    mov cr4, rax
    ret