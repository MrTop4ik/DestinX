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

void dfs_read(const char *fp, uint8_t *buffer, uint64_t offset, uint64_t size){
	if (!buffer || !size || (offset < 0)) return;

	inode_t *cur_inode = &dfs_ctx.inode_table[1];

	uint64_t phys_buf = pmm_alloc_page();
	uint64_t virt_buf = phys_buf + DIRECT_OFFSET;
	memset((void*)virt_buf, 0, PAGE_SIZE_4KB);

	int len = 0;

	while (*(fp) != '\0'){
		if (*fp != '/'){
			fp++;
			continue;
		}

		fp++;

		while (fp[len] != '/' && fp[len] != '\0') len++;

		if ((cur_inode->type == DFS_TYPE_DIR) && *fp != '\0'){
			uint32_t total_entries = cur_inode->size / sizeof(dir_entry_t);

			int status = ahci_read(&ahci_regs->ports[0], cur_inode->extents[0].start_block * 8, 8, &phys_buf, 1);
			if (status != 0){
				pmm_free_page(phys_buf);
				return;
			}

			dir_entry_t *entry_list = (dir_entry_t *)virt_buf;
			int found = 0;
			for (int i = 0; i < total_entries; i++){
				dir_entry_t *entry = &entry_list[i];
				if (memcmp(entry->name, fp, len) == 0){
					cur_inode = &dfs_ctx.inode_table[entry->inode_num];
					found = 1;
				}
			}
			
			if (found) continue;
			pmm_free_page(phys_buf);
			return;
		} else {
			pmm_free_page(phys_buf);
			return;
		}
	}

	if (cur_inode->type == DFS_TYPE_FILE){
		if (offset > cur_inode->size){
			pmm_free_page(phys_buf);
			return;
		}

		if (cur_inode->size < (offset + size)) size = cur_inode->size;

		uint64_t pages_needed = (size + PAGE_SIZE_4KB - 1) / PAGE_SIZE_4KB;
		
		uint64_t first_page = pmm_alloc_pages(pages_needed);
		uint64_t virt_fisrt_page = first_page + DIRECT_OFFSET;
		memset((void*)virt_fisrt_page, 0, PAGE_SIZE_4KB);
		
		uint64_t pages[pages_needed];

		for (int i = 0; i < pages_needed; i++) pages[i] = first_page + i * PAGE_SIZE_4KB;
		int status = ahci_read(&ahci_regs->ports[0], (offset / 512) + (cur_inode->extents[0].start_block * 8), (size + 512 - 1) / 512, pages, pages_needed);
		if (status != 0){
			for (int i = 0; i < pages_needed; i++) pmm_free_page(pages[i]);
			pmm_free_page(phys_buf);
			return;
		}

		memcpy(buffer, (void*)(virt_fisrt_page + (offset % 512)), size);

		serial_print("[DFS READ] Successfully readed from file\n");
		return;
	}
}
