#include <drivers/console.h>
#include <kernel/scheduler/thread.h>
#include <kernel/sync/mutex.h>

vfs_ops_t terminal_ops = {
    .write = terminal_write,
    .read = terminal_read,
    .close = NULL
};

struct FILE stdin = {
    .fp = {0},
    .type = FILE_TYPE_CHAR_DEV,
    .flags = O_RDONLY,
    .position = 0,
    .ops = &terminal_ops,
    .private_data = NULL
};

struct FILE stdout = {
    .fp = {0},
    .type = FILE_TYPE_CHAR_DEV,
    .flags = O_WRONLY,
    .position = 0,
    .ops = &terminal_ops,
    .private_data = NULL
};

mutex_t term_read_mutex = {0};
thread_t *read_blocked_thread = NULL;

int terminal_write(struct FILE *file, const char *buf, size_t count){
    kring_write(buf, count, 1);
    return count;
}

int terminal_read(struct FILE *file, const char *buf, size_t count){
    if (!count) return 0;
    mutex_lock(&term_read_mutex);
    serial_print("[STDIN] Reading Started for Thread %d\n", current_thread->tid);
    if (!check_for_n()){
        read_blocked_thread = current_thread;
        current_thread->state = BLOCKED;
        yield();
    }
    int read = kring_flush_to_rbuf(buf, count);
    serial_print("[STDIN] Reading Ended for Thread %d\n", current_thread->tid);
    mutex_unlock(&term_read_mutex);
    return 0;
}