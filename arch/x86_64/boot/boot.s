MB2_MAGIC       equ   0xE85250D6
MB2_ARCH        equ   0
MB2_HEADER_LEN  equ   header_end - header_start
MB2_MAGIC_SUM   equ   -(MB2_MAGIC + MB2_ARCH + MB2_HEADER_LEN)

section .multiboot_header
align 8
header_start:
    dd MB2_MAGIC
    dd MB2_ARCH
    dd MB2_HEADER_LEN
    dd MB2_MAGIC_SUM

    align 8
    dw 5
    dw 0
    dd 20
    dd 1024
    dd 768
    dd 32

    align 8
    dw 0
    dw 0
    dw 8
header_end:

section .boot_bss alloc nobits
align 4096
pml4: resb 4096
pdpt_ident: resb 4096
pd_indent: resb 4096
pdpt_kernel: resb 4096
pd_kernel: resb 4096
align 16
stack_bottom:
    resb 32768
stack_top:

global stack_top

section .boot_rodata
msg_no_cpuid: db "ERROR: CPUID NOT SUPPORTED", 0
msg_no_long_mode: db "ERROR: LONG MODE (X64) NOT SUPPORTED", 0
msg_no_invariant_tsc: db "ERROR: INVARIANT TSC NOT SUPPORTED", 0

gdt64:
    dq 0
.code_segment: equ $ - gdt64
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
.data_segment: equ $ - gdt64 
    dq (1 << 41) | (1 << 44) | (1 << 47)
.pointer:
    dw $ - gdt64 - 1
    dq gdt64
.virt_pointer:
    dw $ - gdt64 - 1
    dq gdt64 + 0xFFFFFFFF80000000

section .boot_text
bits 32
print_string_32:
    mov edi, 0xB8000
    mov ah, 0x0F
.loop:
    lodsb
    test al, al
    jz .done
    stosw
    jmp .loop
.done:
    ret

check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .no_cpuid
    ret
.no_cpuid:
    push esi

    mov esi, msg_no_cpuid
    call print_string_32

    pop esi
    hlt

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
.no_long_mode:
    push esi
    
    mov esi, msg_no_long_mode
    call print_string_32

    pop esi
    hlt

check_invariant_tsc:
    mov eax, 0x80000007
    cpuid
    test edx, 1 << 8
    jz .no_invariant_tsc
    ret
.no_invariant_tsc:
    push esi

    mov esi, msg_no_invariant_tsc
    call print_string_32

    pop esi
    hlt

global _start
_start:
    mov esp, stack_top

    mov ebp, eax
    mov esi, ebx

    call check_cpuid
    call check_long_mode
    call check_invariant_tsc

    mov edi, pml4
    xor eax, eax
    mov ecx, 4096 * 5 / 4
    rep stosd

    mov eax, pdpt_ident
    or eax, 0b11
    mov [pml4], eax

    mov eax, pd_indent
    or eax, 0b11
    mov [pdpt_ident], eax
    
    xor eax, eax
    or eax, 0b10000011
    mov [pd_indent], eax

    mov eax, pdpt_kernel
    or eax, 0b11
    mov [pml4 + 511 * 8], eax

    mov eax, pd_kernel
    or eax, 0b11
    mov [pdpt_kernel + 510 * 8], eax
    
    xor ecx, ecx
.map_kernel_loop:
    mov eax, 0x200000
    mul ecx
    or eax, 0b10000011
    mov [pd_kernel + ecx * 8], eax

    inc ecx
    cmp ecx, 64
    jne .map_kernel_loop

    mov eax, pml4
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    lgdt [gdt64.pointer]

    jmp gdt64.code_segment:.long_mode_entry

bits 64
.long_mode_entry:
    mov rax, higher_half
    jmp rax

section .text
higher_half:
    mov rax, gdt64.virt_pointer
    lgdt [rax]

    mov ax, gdt64.data_segment
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rax, stack_top
    add rax, 0xFFFFFFFF80000000
    mov rsp, rax

    mov rax, pml4
    add rax, 0xFFFFFFFF80000000
    mov qword [rax], 0

    mov rax, cr3
    mov cr3, rax

    mov rdi, rbp
    mov rsi, rsi

    extern kernel_main
    call kernel_main
     
    hlt