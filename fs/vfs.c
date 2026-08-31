#include <fs/vfs.h>
#include <kernel/scheduler/thread.h>
#include <fs/dfs.h>

vfs_ops_t dfs_ops = {
    .read = dfs_file_read,
    .write = dfs_file_write,
    .close = dfs_file_close
};

spinlock_t open_lock = {0};

uint64_t open(const char *fp, uint64_t flags){
    inode_t *inode = dfs_get_inode(fp);
    if (!inode) return -1;

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
    file->flags = flags;
    file->position = 0;
    file->ops = &dfs_ops;
    file->private_data = inode;

    return fd;
}

int close(uint64_t fd){
    if (fd >= 0 && fd <= 2) return 0;
    else if (fd < 0 || fd > MAX_FD) return -1;
    
    struct FILE *file = current_thread->process->fd_table[fd];
    if (!file) return -1;

    int ret = file->ops->close(file);
    current_thread->process->fd_table[fd] = NULL;
    return ret;
}