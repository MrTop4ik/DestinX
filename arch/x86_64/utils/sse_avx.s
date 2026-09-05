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

global avx_lfb_memcpy
align 32
avx_lfb_memcpy:
    shr rdx, 8
    jz .exit

.loop_avx:
    vmovdqu ymm0, [rsi]
    vmovdqu ymm1, [rsi + 32]
    vmovdqu ymm2, [rsi + 64]
    vmovdqu ymm3, [rsi + 96]
    vmovdqu ymm4, [rsi + 128]
    vmovdqu ymm5, [rsi + 160]
    vmovdqu ymm6, [rsi + 192]
    vmovdqu ymm7, [rsi + 224]

    vmovntdq [rdi], ymm0
    vmovntdq [rdi + 32], ymm1
    vmovntdq [rdi + 64], ymm2
    vmovntdq [rdi + 96], ymm3
    vmovntdq [rdi + 128], ymm4
    vmovntdq [rdi + 160], ymm5
    vmovntdq [rdi + 192], ymm6
    vmovntdq [rdi + 224], ymm7

    add rdi, 256
    add rsi, 256
    
    dec rdx
    jnz .loop_avx

    sfence

.exit:
    vzeroupper
    ret


global sse_lfb_memcpy
sse_lfb_memcpy:
    ret