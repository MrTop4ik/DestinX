#pragma once
#include <fs/vfs.h>
#include <drivers/kring.h>

extern vfs_ops_t terminal_ops;
extern struct FILE stdin;
extern struct FILE stdout;

int terminal_write(struct FILE *file, const char *buf, size_t count);
int terminal_read(struct FILE *file, const char *buf, size_t count);