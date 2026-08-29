#pragma once
#include <stdint.h>
#include <stddef.h>
#include <drivers/ahci.h>
#include <mm/vmalloc.h>
#include <fs/vfs.h>

#define DFS_MAGIC 0x00534644
#define DFS_BLOCKSIZE 0x1000
#define DFS_MAX_FILENAME 56

#define DFS_TYPE_FREE 0
#define DFS_TYPE_FILE 1
#define DFS_TYPE_DIR 2

typedef struct {
    uint32_t logical_block;
    uint32_t length;
    uint64_t start_block;
} __attribute__((packed)) extent_t;

typedef struct {
    uint64_t size;
    uint16_t type;
    uint16_t links_count;
    uint32_t flags;
    extent_t extent;
    uint8_t reserved[96];
} __attribute__((packed)) inode_t;

typedef struct {
    uint64_t magic;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t total_inodes;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t block_bitmap_block;
    uint32_t inode_bitmap_block;
    uint32_t inode_table_block;
    uint32_t data_blocks_start;
} __attribute__((packed)) superblock_t;

typedef struct {
    uint32_t inode_num;
    uint8_t type;
    uint8_t reserved[3];
    char name[DFS_MAX_FILENAME];
} __attribute__((packed)) dir_entry_t;

typedef struct {
    superblock_t *sb;
    uint8_t *inode_bitmap;
    uint8_t *block_bitmap;
    inode_t *inode_table;
} dfs_ctx_t;

extern dfs_ctx_t dfs_ctx;

inode_t *dfs_mount_root();
uint64_t dfs_read(const char *fp, uint8_t *buffer, uint64_t offset, uint64_t size);
size_t dfs_get_size(const char *path);
int dfs_file_read(struct FILE *file, const char *buf, size_t count);
int dfs_file_close(struct FILE *file);