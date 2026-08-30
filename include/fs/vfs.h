#pragma once
#include <stdint.h>
#include <stddef.h>

#define MAX_FILEPATH 256

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR   0x0002

typedef enum {
    FILE_TYPE_FREE = 0,
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_CHAR_DEV,
    FILE_TYPE_BLOCK_DEV,
    FILE_TYPE_PIPE,
    FILE_TYPE_SOCKET
} file_type_t;

struct FILE;

typedef struct {
    int (*read)(struct FILE *file, const char *buf, size_t count);
    int (*write)(struct FILE *file, const char *buf, size_t count);
    int (*close)(struct FILE *file);
} vfs_ops_t;

typedef struct FILE {
    char fp[MAX_FILEPATH];
    file_type_t type;
    uint64_t flags;
    size_t size;
    size_t position;
    vfs_ops_t *ops;
    void *private_data;
} file_t;

uint64_t open(const char *fp, uint64_t flags);
int close(uint64_t fd);