#include <drivers/klog.h>

static log_entry_t *klog_buffer = NULL;
static uint64_t klog_head = 0;
static uint64_t klog_tail = 0;

static spinlock_t klog_write_lock = {0};
static spinlock_t klog_read_lock = {0};

void init_klog(void){
    klog_buffer = (log_entry_t *)kmalloc(KLOG_BUF_MAX * sizeof(log_entry_t));
    if (!klog_buffer) for(;;);

    for (int i = 0; i < 0; i ++){
        klog_buffer[i].c = 0;
        klog_buffer[i].ready = 0;
    }

    klog_head = 0;
    klog_tail = 0;

    serial_print("[KLOG] KLOG buffer was initialized\n");
}

void klog_write(const char *s, size_t len){
    if (!klog_buffer || len == 0) return;

    if (len > KLOG_BUF_MAX) len = KLOG_BUF_MAX;

    uint64_t rflags = spin_lock_irqsave(&klog_write_lock);
    uint64_t start_head = klog_head;
    klog_head += len;
    spin_lock_irqrestore(&klog_write_lock, rflags);

    for (size_t i = 0; i < len; i++){
        uint64_t pos = (start_head + i) & KLOG_BUF_MASK;

        klog_buffer[pos].c = s[i];
        __atomic_store_n(&klog_buffer[pos].ready, 1, __ATOMIC_RELEASE);
    }
}

void klog_flush_to_screen(void){
    if (!klog_buffer) return;

    uint64_t rflags = spin_lock_irqsave(&klog_read_lock);

    int printed = 0;

    while (1){
        uint64_t current_head = __atomic_load_n(&klog_head, __ATOMIC_ACQUIRE);

        if ((current_head - klog_tail) > KLOG_BUF_MAX){
            klog_tail = current_head - KLOG_BUF_MAX;
            kputchar_direct('['); kputchar_direct('d'); kputchar_direct('r'); kputchar_direct('o'); kputchar_direct('p'); kputchar_direct(']'); 
            printed = 1;
        }

        if (klog_tail == klog_head) break;

        uint64_t pos = klog_tail & (KLOG_BUF_MASK);

        uint8_t ready = __atomic_load_n(&klog_buffer[pos].ready, __ATOMIC_ACQUIRE);
        if (!ready) break;

        kputchar_direct(klog_buffer[pos].c);

        __atomic_store_n(&klog_buffer[pos].ready, 0, __ATOMIC_RELEASE);

        uint64_t rflags = spin_lock_irqsave(&klog_write_lock);
        klog_tail++;
        spin_lock_irqrestore(&klog_write_lock, rflags);
        printed = 1;
    }

    if (printed) lfb_swap();

    spin_lock_irqrestore(&klog_read_lock, rflags);
}