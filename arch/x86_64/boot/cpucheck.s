%include "video.s"

section .boot_text
bits 32
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
    mov edi, msg_no_cpuid
    call print_string_32_vga
    hlt

check_long_mode:
    push ebx
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    pop ebx
    ret
.no_long_mode:
    mov edi, msg_no_long_mode
    call print_string_32_vga
    hlt

check_invariant_tsc:
    push ebx
    mov eax, 0x80000007
    cpuid
    test edx, 1 << 8
    jz .no_invariant_tsc
    pop ebx
    ret
.no_invariant_tsc:
    mov edi, msg_no_invariant_tsc
    call print_string_32_vga
    hlt

check_lfb:
    push esi
    push edx
    push ecx
    push eax
    mov ecx, [edi]
    lea edx, [edi + 8]
    add ecx, edi

.loop_tags:
    mov eax, [edx]
    mov esi, [edx + 4]

    cmp eax, 8
    je .done

    cmp eax, 0
    je .no_lfb

.next_tag:
    add esi, 7
    and esi, 0xFFFFFFF8

    add edx, esi
    jmp .loop_tags

.no_lfb:
    mov edi, msg_no_lfb
    call print_string_32_vga
    hlt

.done:
    pop eax
    pop ecx

    mov esi, [edx + 8]
    mov [eax], esi

    mov esi, [edx + 16]
    mov [ecx], esi

    pop edx
    pop esi

    ret