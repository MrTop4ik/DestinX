#include <drivers/kring.h>

extern int backspace;

log_entry_t *write_kring = NULL;
log_entry_t *read_kring = NULL;
uint64_t write_kring_head = 0;
uint64_t write_kring_tail = 0;
uint64_t read_kring_head = 0;
uint64_t read_kring_tail = 0;

static spinlock_t kring_write_lock = {0};
static spinlock_t kring_read_lock = {0};

void init_kring(void){
    write_kring = (log_entry_t *)kmalloc(KRING_BUF_MAX * sizeof(log_entry_t));
    read_kring = (log_entry_t *)kmalloc(KRING_BUF_MAX * sizeof(log_entry_t));
    if (!write_kring) for(;;);

    for (int i = 0; i < 0; i ++){
        write_kring[i].c = 0;
        write_kring[i].ready = 0;

        read_kring[i].c = 0;
        read_kring[i].ready = 0;
    }

    serial_print("[KRING] KRING buffer was initialized\n");
}

void kring_write(const char *s, size_t len, uint8_t mode){
    if (len == 0) return;

    if (mode && write_kring){
        if (len > KRING_BUF_MAX) len = KRING_BUF_MAX;

        uint64_t rflags = spin_lock_irqsave(&kring_write_lock);
        uint64_t start_head = write_kring_head;
        write_kring_head += len;
        spin_lock_irqrestore(&kring_write_lock, rflags);

        for (size_t i = 0; i < len; i++){
            uint64_t pos = (start_head + i) & KRING_BUF_MASK;

            write_kring[pos].c = s[i];
            __atomic_store_n(&write_kring[pos].ready, 1, __ATOMIC_RELEASE);
        }
    } else if (!mode && read_kring){
        if (len > KRING_BUF_MAX) len = KRING_BUF_MAX;

        uint64_t rflags = spin_lock_irqsave(&kring_write_lock);
        uint64_t start_head = read_kring_head;
        read_kring_head += len;
        spin_lock_irqrestore(&kring_write_lock, rflags);

        for (size_t i = 0; i < len; i++){
            uint64_t pos = (start_head + i) & KRING_BUF_MASK;

            read_kring[pos].c = s[i];
            __atomic_store_n(&read_kring[pos].ready, 1, __ATOMIC_RELEASE);
        }
    }
}

void kring_flush_to_screen(void){
    if (!write_kring) return;

    uint64_t rflags = spin_lock_irqsave(&kring_read_lock);

    int printed = 0;

    while (1){
        uint64_t current_head = __atomic_load_n(&write_kring_head, __ATOMIC_ACQUIRE);

        if ((current_head - write_kring_tail) > KRING_BUF_MAX){
            write_kring_tail = current_head - KRING_BUF_MAX;
            kputchar_direct('['); kputchar_direct('d'); kputchar_direct('r'); kputchar_direct('o'); kputchar_direct('p'); kputchar_direct(']'); 
            printed = 1;
        }

        if (write_kring_tail == write_kring_head) break;

        uint64_t pos = write_kring_tail & (KRING_BUF_MASK);

        uint8_t ready = __atomic_load_n(&write_kring[pos].ready, __ATOMIC_ACQUIRE);
        if (!ready) break;

        kputchar_direct(write_kring[pos].c);

        __atomic_store_n(&write_kring[pos].ready, 0, __ATOMIC_RELEASE);

        uint64_t rflags0 = spin_lock_irqsave(&kring_write_lock);
        write_kring_tail++;
        spin_lock_irqrestore(&kring_write_lock, rflags0);
        printed = 1;
    }

    if (printed || backspace) { backspace = 0; lfb_swap(); }

    spin_lock_irqrestore(&kring_read_lock, rflags);
}

int kring_flush_to_rbuf(char *buf, size_t count){
    int read = 0;
    for (size_t i = 0; i < count; i++){
        uint16_t pos = read_kring_tail++ & KRING_BUF_MASK;
        char c = read_kring[pos].c;
        read_kring[pos].ready = 0;
        buf[i] = c;
        read++;
        if (c == '\n') break;
    }
    return read;
}

int check_for_n(void){
    for (int i = 0; i < KRING_BUF_MAX; i++){
        if (read_kring[i].c == '\n' && read_kring[i].ready == 1) return 1;
    }
    return 0;
}