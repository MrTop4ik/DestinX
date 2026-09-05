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