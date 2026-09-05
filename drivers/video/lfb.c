#include <drivers/lfb.h>

extern uint64_t sse_avx_check();

uint64_t instr_supported = 0;

framebuffer_t lfb;

void init_LFB(unsigned int physBootInfo){
    struct multiboot_info *virtBootInfo = (struct multiboot_info *)(physBootInfo + KERNEL_OFFSET);

    struct multiboot_tag* tag = (struct multiboot_tag *)((uint8_t *)virtBootInfo + 8);

    while (tag->type != MULTIBOOT_TAG_TYPE_END){
        if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER){
            struct multiboot_tag_framebuffer *fb = (struct multiboot_tag_framebuffer *)tag;
            lfb.paddr = fb->framebuffer_addr;
            lfb.width = fb->framebuffer_width;
            lfb.height = fb->framebuffer_height;
            lfb.pitch = fb->framebuffer_pitch;
            lfb.size = lfb.pitch * lfb.height;
        }
        tag = (struct multiboot_tag *)((uintptr_t)((uint8_t*)tag + tag->size + 7) & ~7);
    }

    for (int size = 0; size < lfb.size; size += PAGE_SIZE_2MB) vmm_map_page(read_cr3(), lfb.paddr + size, size + LFB_ADDR, PAGE_SIZE_2MB, (PTE_WRITABLE | PTE_WC_HUGE));

    lfb.vram = (uint32_t *)LFB_ADDR;
    lfb.buffer = vmalloc(lfb.size);

    lfb.font = (uint8_t *)font8x16;
    lfb.char_width = 8;
    lfb.char_height = 16;
    lfb.cursor_x = 0;
    lfb.cursor_y = 0;

    memset(lfb.buffer, 0, lfb.size);

    instr_supported = sse_avx_check();
    init_kring();

    lfb_swap();

    serial_print("[LFB] LFB was initialized\n");
}

void lfb_swap(void){
    memcpy(lfb.vram, lfb.buffer, lfb.size);
}

void lfb_scroll(void){
    uint32_t line_size = lfb.pitch * lfb.char_height;
    uint32_t move_size = lfb.size - line_size;

    memcpy(lfb.buffer, (uint8_t *)lfb.buffer + line_size, move_size);
    memset((uint8_t *)lfb.buffer + move_size, 0, line_size);
}

void putpixel(int x, int y, uint32_t color){
    if (x >= lfb.width || y >= lfb.height) return;
    lfb.buffer[y * (lfb.pitch / 4) + x] = color;
}

void draw_char_at(int x, int y, char c, uint32_t fg, uint32_t bg){
    uint8_t *glyph = lfb.font  + (uint8_t)c * lfb.char_height;

    for (int i = 0; i < lfb.char_height; i++){
        for (int j = 0; j < lfb.char_width; j++){
            if ((glyph[i] >> (7 - j)) & 1) putpixel(x + j, y + i, fg);
            else putpixel(x + j, y + i, bg);
        }
    }
}

void kputchar_direct(char c){
    if (c == '\n'){
        lfb.cursor_x = 0;
        lfb.cursor_y += lfb.char_height;
    } else {
        draw_char_at(lfb.cursor_x, lfb.cursor_y, c, 0xFFFFFFFF, 0x0);
        lfb.cursor_x += lfb.char_width;
    }

    if (lfb.cursor_x + lfb.char_width > lfb.width){
        lfb.cursor_x = 0;
        lfb.cursor_y += lfb.char_height;
    }

    if (lfb.cursor_y + lfb.char_height > lfb.height){
        lfb_scroll();
        lfb.cursor_y -= lfb.char_height;
    }
}

void kputchar(char c){
    kputchar_direct(c);
    lfb_swap();
}

void kputnum_direct(uint32_t num, uint32_t base){
    char buf[32];
    int i = 0;
    const char digits[] = "0123456789abcdef";

    if (num == 0){
        kputchar_direct('0');
        return;
    }

    while (num > 0){
        buf[i++] = digits[num % base];
        num /= base;
    }

    while (i > 0) kputchar_direct(buf[--i]);
}

static int local_putnum(char *buf, uint32_t max_len, uint64_t num, uint32_t base){
    int i = 0;
    const char digits[] = "0123456789abcdef";

    if (num == 0 && max_len > 0){
        buf[i++] = '0';
        return i;
    }

    char temp[64];
    int j = 0;

    while (num > 0 && j < 64){
        temp[j++] = digits[num % base];
        num /= base;
    }

    while (j > 0 && i < max_len) buf[i++] = temp[--j];

    return i;
}

void kprintf(const char* format, ...){
    char local_buf[KPRINTF_BUF_MAX];
    int idx = 0;

    va_list args;
    va_start(args, format);

    for (const char* p = format; *p != '\0'; p++){
        CHECK_AND_FLUSH();

        if (*p != '%') {
            local_buf[idx++] = *p;
            continue;
        }

        p++;
        switch (*p) {
            case 's':
                char *s = va_arg(args, char *);
                while (*s){
                    CHECK_AND_FLUSH();
                    local_buf[idx++] = *s++;
                }
                break;
            case 'd': {
                int i = va_arg(args, int);

                if (i < 0) {
                    CHECK_AND_FLUSH();
                    local_buf[idx++] = '-';
                    i = -i;
                }

                if (idx > KPRINTF_BUF_MAX - 35){
                    kring_write(local_buf, idx);
                    idx = 0;
                }

                idx += local_putnum(&local_buf[idx], KPRINTF_BUF_MAX - 1, (uint64_t)i, 10);
                break;
            }
            case 'x': {
                uint32_t i = va_arg(args, uint32_t);

                CHECK_AND_FLUSH(); local_buf[idx++] = '0';
                CHECK_AND_FLUSH(); local_buf[idx++] = 'x';

                if (idx > KPRINTF_BUF_MAX - 35){
                    kring_write(local_buf, idx);
                    idx = 0;
                }

                idx += local_putnum(&local_buf[idx], KPRINTF_BUF_MAX - 1, (uint64_t)i, 16);
                break;
            }
            case 'l':
                if (*(p+1) == 'l' && *(p+2) == 'x') {
                    p += 2;
                    uint64_t i = va_arg(args, uint64_t);

                    CHECK_AND_FLUSH(); local_buf[idx++] = '0';
                    CHECK_AND_FLUSH(); local_buf[idx++] = 'x';
                    
                    if (idx > KPRINTF_BUF_MAX - 35){
                        kring_write(local_buf, idx);
                        idx = 0;
                    }

                    idx += local_putnum(&local_buf[idx], KPRINTF_BUF_MAX - 1, (uint64_t)i, 16);
                }
                break;
            case 'p':
                uint64_t i = va_arg(args, uint64_t);

                CHECK_AND_FLUSH(); local_buf[idx++] = '0';
                CHECK_AND_FLUSH(); local_buf[idx++] = 'x';

                if (idx > KPRINTF_BUF_MAX - 35){
                    kring_write(local_buf, idx);
                    idx = 0;
                }

                idx += local_putnum(&local_buf[idx], KPRINTF_BUF_MAX - 1, (uint64_t)i, 16);
                break;
            case '%':
                local_buf[idx++] = '%';
                break;
        }
    }
    va_end(args);

    if (idx > 0) kring_write(local_buf, idx);
}