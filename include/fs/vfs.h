#pragma once
#include <stdint.h>
#include <stddef.h>

#define MAX_FILEPATH 256

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
    char fp[MAX_FILEPATH];
    file_type_t type;
    size_t size;
    size_t position;
    vfs_ops_t *ops;
    void *private_data;
} file_t;

uint64_t open(const char *fp);
int close(uint64_t fd);