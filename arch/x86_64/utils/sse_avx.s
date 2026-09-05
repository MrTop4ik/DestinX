bits 64
section .text

global sse_avx_check
sse_avx_check:
    push rbx

    mov eax, 1
    cpuid

    bt ecx, 27
    jne .sse_check

    bt ecx, 28
    jne .sse_check

    mov rax, 2
    jmp .exit
.sse_check:
    bt edx, 26
    jnc .no_support

    mov rax, 1
    jmp .exit
.no_support:
    mov rax, 0
.exit:
    pop rbx
    ret

global init_sse_avx
init_sse_avx:
    mov rax, cr0
    or rax, (1 << 1)
    and rax, ~(1 << 2)
    mov cr0, rax

    mov rax, cr4
    or rax, (1 << 9)
    or rax, (1 << 10)
    or rax, (1 << 18)
    mov cr4, rax

    mov ecx, 0    
    xgetbv
    or eax, 0x7
    xsetbv

    ret