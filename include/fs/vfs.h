#pragma once
#include <stdint.h>
#include <stddef.h>

typedef enum {
    FILE_TYPE_REGULAR = 0,
    FILE_TYPE_DEVICE,
    FILE_TYPE_SOCKET,
} file_type_t;

struct FILE;

typedef struct {
    int (*read)(struct FILE *file, const char *buf, size_t count);
    int (*write)(struct FILE *file, const char *buf, size_t count);
    int (*close)(struct FILE *file);
} vfs_ops_t;

typedef struct FILE {
    file_type_t type;
    size_t position;
    vfs_ops_t *ops;
    void *private_data;
} file_t;