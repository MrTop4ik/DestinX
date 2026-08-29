#include <drivers/console.h>

vfs_ops_t terminal_ops = {
    .write = terminal_write,
    .read = NULL,
    .close = NULL
};

struct FILE terminal_file = {
    .fp = 0,
    .type = FILE_TYPE_DEVICE,
    .position = 0,
    .ops = &terminal_ops,
    .private_data = NULL
};

int terminal_write(struct FILE *file, const char *buf, size_t count){
    kring_write(buf, count);
    return count;
}