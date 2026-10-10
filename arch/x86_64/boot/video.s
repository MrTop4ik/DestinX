section .boot_text
bits 32
print_string_32_vga:
    mov edi, 0xB8000
    mov ah, 0x0F
.loop_print:
    cmp byte [esi], 0
    je .done

    mov al, [esi]
    mov [edi], ax

    inc esi
    add edi, 2
    
    jmp .loop_print

.done:
    ret