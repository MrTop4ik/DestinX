#include "arch/x86_64/drivers/serial.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include <fs/dfs.h>
#include <stdint.h>

superblock_t *sb;

inode_t *dfs_mount() {
    uint64_t pages[2];
    pages[0] = pmm_alloc_page();

    int status = ahci_read(&ahci_regs->ports[0], 8, 8, pages, 1);
    if (status != 0) return NULL;

    sb = (superblock_t *)(pages[0] + DIRECT_OFFSET);

    if (sb->magic == DFS_MAGIC) serial_print("[DFS] DFS Magic check success\n");
    else serial_print("[DFS] Wrong Magic\n");

    pages[1] = pmm_alloc_page();

    status = ahci_read(&ahci_regs->ports[0], 16, 8, &pages[1], 1);
    if (status != 0){ pmm_free_page(pages[0]); return 0; };

    inode_t *root_inode = (inode_t *)(pages[1] + DIRECT_OFFSET + sizeof(inode_t));

    serial_print("[DFS] DFS successfully mounted");
    return root_inode;
}