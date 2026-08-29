#include <fs/vfs.h>
#include <kernel/scheduler/thread.h>
#include <fs/dfs.h>

vfs_ops_t dfs_ops = {
    .read = NULL,
    .write = NULL,
    .close = NULL
};

spinlock_t open_lock = {0};

uint64_t open(const char *fp){
    size_t size = dfs_get_size(fp);
    if (size == (size_t)-1) return -1;

    uint64_t fd = -1;

    uint64_t rflags = spin_lock_irqsave(&open_lock);

    for (int i = 0; i < MAX_FD; i++){
        if (current_thread->process->fd_table[i] == NULL){
            fd = i;
            break;
        }
    }

    if (fd == -1){
        spin_lock_irqrestore(&open_lock, rflags);
        return -1;
    }

    struct FILE *file = (struct FILE *)kmalloc(sizeof(struct FILE));
    current_thread->process->fd_table[fd] = file;

    spin_lock_irqrestore(&open_lock, rflags);

    memcpy(file->fp, fp, MAX_FILEPATH - 1);
    file->fp[MAX_FILEPATH - 1] = '\0';
    file->type = FILE_TYPE_REGULAR;
    file->size = size;
    file->position = 0;
    file->ops = &dfs_ops;
    file->private_data = NULL;

    serial_print("%s\n", file->fp);

    return fd;
}