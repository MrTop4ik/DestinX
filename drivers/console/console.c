#include <drivers/console.h>

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

int terminal_write(struct FILE *file, const char *buf, size_t count){
    kring_write(buf, count);
    return count;
}

int terminal_read(struct FILE *file, const char *buf, size_t count){
    return 0;
}