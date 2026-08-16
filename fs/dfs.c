#include "arch/x86_64/drivers/serial.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include <fs/dfs.h>
#include <stdint.h>

dfs_ctx_t dfs_ctx;

inode_t *dfs_mount(){
    uint64_t page = pmm_alloc_page();

    int status = ahci_read(&ahci_regs->ports[0], 8, 8, &page, 1);
    if (status != 0) return NULL;

    superblock_t *sb = (superblock_t *)(page + DIRECT_OFFSET);

    if (sb->magic == DFS_MAGIC) serial_print("[DFS] DFS Magic check success\n");
    else serial_print("[DFS] Wrong Magic\n");

    uint64_t pages[34];
    memset(pages, 0, sizeof(pages));

    uint64_t p = pmm_alloc_pages(34);
    if (!p){ pmm_free_page(page); return NULL; }

    for (int i = 0; i < 34; i++) pages[i] = p + i * PAGE_SIZE_4KB;

    status = ahci_read(&ahci_regs->ports[0], 16, (8 * 34), pages, 34);
    if (status != 0){ 
        pmm_free_page(page);
        for (int i = 0; i < 34; i++) pmm_free_page(pages[i]);
        return NULL;
    }

    dfs_ctx.sb = sb;
    dfs_ctx.inode_bitmap = (uint8_t *)(pages[0] + DIRECT_OFFSET);
    dfs_ctx.block_bitmap = (uint8_t *)(pages[1] + DIRECT_OFFSET);
    dfs_ctx.inode_table  = (inode_t *)(pages[2] + DIRECT_OFFSET);

    inode_t *root_inode = &dfs_ctx.inode_table[1];

    if (root_inode) serial_print("[DFS] DFS Successfully mounted\n");
    else serial_print("[DFS] Failed to munt DFS\n");
    return root_inode;
}