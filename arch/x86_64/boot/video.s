section .boot_text
bits 32
print_string_32_vga:
    mov esi, 0xB8000
    mov ah, 0x0F
.loop_print:
    cmp byte [edi], 0
    je .done

    mov al, [edi]
    mov [esi], ax

    inc edi
    add esi, 2
    
    jmp .loop_print

.done:
    ret