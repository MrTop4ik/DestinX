#include <drivers/kring.h>

static log_entry_t *kring_buffer = NULL;
static uint64_t kring_head = 0;
static uint64_t kring_tail = 0;

static spinlock_t kring_write_lock = {0};
static spinlock_t kring_read_lock = {0};

void init_kring(void){
    kring_buffer = (log_entry_t *)kmalloc(KRING_BUF_MAX * sizeof(log_entry_t));
    if (!kring_buffer) for(;;);

    for (int i = 0; i < 0; i ++){
        kring_buffer[i].c = 0;
        kring_buffer[i].ready = 0;
    }

    kring_head = 0;
    kring_tail = 0;

    serial_print("[KRING] KRING buffer was initialized\n");
}

void kring_write(const char *s, size_t len){
    if (!kring_buffer || len == 0) return;

    if (len > KRING_BUF_MAX) len = KRING_BUF_MAX;

    uint64_t rflags = spin_lock_irqsave(&kring_write_lock);
    uint64_t start_head = kring_head;
    kring_head += len;
    spin_lock_irqrestore(&kring_write_lock, rflags);

    for (size_t i = 0; i < len; i++){
        uint64_t pos = (start_head + i) & KRING_BUF_MASK;

        kring_buffer[pos].c = s[i];
        __atomic_store_n(&kring_buffer[pos].ready, 1, __ATOMIC_RELEASE);
    }
}

void kring_flush_to_screen(void){
    if (!kring_buffer) return;

    uint64_t rflags = spin_lock_irqsave(&kring_read_lock);

    int printed = 0;

    while (1){
        uint64_t current_head = __atomic_load_n(&kring_head, __ATOMIC_ACQUIRE);

        if ((current_head - kring_tail) > KRING_BUF_MAX){
            kring_tail = current_head - KRING_BUF_MAX;
            kputchar_direct('['); kputchar_direct('d'); kputchar_direct('r'); kputchar_direct('o'); kputchar_direct('p'); kputchar_direct(']'); 
            printed = 1;
        }

        if (kring_tail == kring_head) break;

        uint64_t pos = kring_tail & (KRING_BUF_MASK);

        uint8_t ready = __atomic_load_n(&kring_buffer[pos].ready, __ATOMIC_ACQUIRE);
        if (!ready) break;

        kputchar_direct(kring_buffer[pos].c);

        __atomic_store_n(&kring_buffer[pos].ready, 0, __ATOMIC_RELEASE);

        uint64_t rflags0 = spin_lock_irqsave(&kring_write_lock);
        kring_tail++;
        spin_lock_irqrestore(&kring_write_lock, rflags0);
        printed = 1;
    }

    if (printed) lfb_swap();

    spin_lock_irqrestore(&kring_read_lock, rflags);
}