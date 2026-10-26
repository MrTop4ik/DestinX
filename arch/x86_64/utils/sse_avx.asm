bits 64
section .text

extern sse_avx

global sse_avx_check
sse_avx_check:
    push rbx

    mov eax, 1
    cpuid

    bt ecx, 26
    jnc .sse_check

    bt ecx, 28
    jnc .sse_check

    mov rax, 2
    jmp .exit
.sse_check:
    bt edx, 26
    jnc .no_support

    mov rax, 1
    jmp .exit
.no_support:
    xor rax, rax
.exit:
    pop rbx
    ret

global init_sse_avx
init_sse_avx:
    mov r8b, [rel sse_avx]
    cmp r8b, 0
    je .exit

    mov rax, cr0
    or rax, (1 << 1)
    and rax, ~(1 << 2)
    mov cr0, rax

    mov rax, cr4
    or rax, (1 << 9)
    or rax, (1 << 10)
    mov cr4, rax

    cmp r8b, 2
    jne .exit

    mov rax, cr4
    or rax, (1 << 18)
    mov cr4, rax

    mov ecx, 0
    xgetbv
    or eax, 0x7
    xsetbv

.exit:
    ret

global get_sse_avx_buf_size
get_sse_avx_buf_size:
    push rbx

    mov r8b, [rel sse_avx]
    cmp r8b, 0
    je .no_support

    cmp r8b, 2
    je .avx_size

    mov rax, 512
    jmp .exit

.avx_size:
    mov eax, 0xD
    mov ecx, 0
    cpuid

    mov ebx, ebx
    jmp .exit

.no_support:
    xor rax, rax

.exit:
    pop rbx
    ret

global save_avx_ctx
save_avx_ctx:
    mov eax, 0x7
    xor edx, edx
    xsave [rdi]
    ret

global restore_avx_ctx
restore_avx_ctx:
    mov eax, 0x7
    xor edx, edx
    xrstor [rdi]
    ret

global save_sse_ctx
save_sse_ctx:
    fxsave [rdi]
    ret

global restore_sse_ctx
restore_sse_ctx:
    fxrstor [rdi]
    ret

global avx_lfb_memcpy
avx_lfb_memcpy:
    shr rdx, 8
    jz .exit

align 32
.loop_avx:
    vmovdqa ymm0, [rsi]
    vmovdqa ymm1, [rsi + 32]
    vmovdqa ymm2, [rsi + 64]
    vmovdqa ymm3, [rsi + 96]
    vmovdqa ymm4, [rsi + 128]
    vmovdqa ymm5, [rsi + 160]
    vmovdqa ymm6, [rsi + 192]
    vmovdqa ymm7, [rsi + 224]

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
    shr rdx, 7
    jz .exit

align 16
.loop_avx:
    movdqa xmm0, [rsi]
    movdqa xmm1, [rsi + 16]
    movdqa xmm2, [rsi + 32]
    movdqa xmm3, [rsi + 48]
    movdqa xmm4, [rsi + 64]
    movdqa xmm5, [rsi + 80]
    movdqa xmm6, [rsi + 96]
    movdqa xmm7, [rsi + 112]

    movntdq [rdi], xmm0
    movntdq [rdi + 16], xmm1
    movntdq [rdi + 32], xmm2
    movntdq [rdi + 48], xmm3
    movntdq [rdi + 64], xmm4
    movntdq [rdi + 80], xmm5
    movntdq [rdi + 96], xmm6
    movntdq [rdi + 112], xmm7

    add rdi, 128
    add rsi, 128
    
    dec rdx
    jnz .loop_avx

    sfence

.exit:
    ret