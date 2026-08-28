#pragma once
#include <fs/vfs.h>
#include <drivers/kring.h>

extern vfs_ops_t terminal_ops;
extern struct FILE terminal_file;

int terminal_write(struct FILE *file, const char *buf, size_t count);