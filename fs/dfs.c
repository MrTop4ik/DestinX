#include <fs/dfs.h>

dfs_ctx_t dfs_ctx;

inode_t *dfs_mount_root(){
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

uint64_t dfs_read(const char *fp, uint8_t *buffer, uint64_t offset, uint64_t size){
	if (!buffer || !size || (offset < 0)) return 0;

	inode_t *cur_inode = dfs_get_inode(fp);

	uint64_t page_count_to_read = ((offset + size - (offset & PAGE_MASK_4KB)) + PAGE_SIZE_4KB - 1) / PAGE_SIZE_4KB; 
	
	uint64_t pages_paddrs[page_count_to_read];
	int miss = 0;

	for (int i = 0; i < page_count_to_read; i++){
		pages_paddrs[i] = get_page_addr(cur_inode->inode_num, (offset & PAGE_MASK_4KB) / PAGE_SIZE_4KB + i);
		if (!pages_paddrs[i]) miss = 1;
	}

	if (!miss){
		if (page_count_to_read == 1){ 
			memcpy(buffer, (void*)(pages_paddrs[0] + DIRECT_OFFSET), size);
			serial_print("[DFS READ] Successfully read from file\n");
			return size;
		}

		memcpy(buffer, (void*)(pages_paddrs[0] + DIRECT_OFFSET), PAGE_SIZE_4KB - offset);

		if (page_count_to_read == 2){
			memcpy(buffer + PAGE_SIZE_4KB - offset, (void*)(pages_paddrs[1] + DIRECT_OFFSET), (size + offset) % PAGE_SIZE_4KB);
			serial_print("[DFS READ] Successfully read from file\n");
			return size;
		}

		for (int i = 1; i < page_count_to_read - 1; i++) memcpy(buffer + i * PAGE_SIZE_4KB, (void*)(pages_paddrs[i] + DIRECT_OFFSET), PAGE_SIZE_4KB);
		memcpy(buffer - offset + (page_count_to_read - 1) * PAGE_SIZE_4KB, (void*)(pages_paddrs[page_count_to_read - 1] + DIRECT_OFFSET), (size + offset) % PAGE_SIZE_4KB);
		serial_print("[DFS READ] Successfully read from file\n");
		return size;
	}

	serial_print("[DFS READ] Cache Miss\n");


	if (offset > cur_inode->size) return 0;

	if (cur_inode->size < (offset + size)) size = cur_inode->size - offset;
	
	for (int i = 0; i < page_count_to_read; i++){
		if (!pages_paddrs[i]){
			uint64_t paddr = pmm_alloc_page();
			int status = ahci_read(&ahci_regs->ports[0], ((offset & PAGE_MASK_4KB) / PAGE_SIZE_4KB + i) * 8 + cur_inode->extent.start_block * 8, 8, &paddr, 1);
			if (status != 0){
				pmm_free_page(paddr);
				return 0;
			}
			add_page_to_cache(cur_inode->inode_num, (offset & PAGE_MASK_4KB) / PAGE_SIZE_4KB + i, paddr);
		}
	}

	uint64_t bytes_read = dfs_read(fp, buffer, offset, size);
	if (!bytes_read) return 0;

	return size;
}

uint64_t dfs_write(const char *fp, uint8_t *buffer, uint64_t offset, uint64_t size){
	if (!buffer || !size || (offset < 0)) return 0;

	inode_t *cur_inode = dfs_get_inode(fp);

	if (cur_inode->type == DFS_TYPE_FILE){
		if (offset > cur_inode->size) return 0;

		if (cur_inode->size < (offset + size)) size = cur_inode->size - offset;

		uint64_t pages_needed = (size + PAGE_SIZE_4KB - 1) / PAGE_SIZE_4KB;
		
		uint64_t first_page = pmm_alloc_pages(pages_needed);
		uint64_t virt_fisrt_page = first_page + DIRECT_OFFSET;
		memset((void *)virt_fisrt_page, 0, PAGE_SIZE_4KB);
		
		uint64_t pages[pages_needed];

		for (int i = 0; i < pages_needed; i++) pages[i] = first_page + i * PAGE_SIZE_4KB;
		int status = ahci_read(&ahci_regs->ports[0], (offset / 512) + (cur_inode->extent.start_block * 8), (size + 512 - 1) / 512, pages, pages_needed);
		if (status != 0){
			for (int i = 0; i < pages_needed; i++) pmm_free_page(pages[i]);
			return 0;
		}

		memcpy((void*)(virt_fisrt_page + (offset % 512)), buffer, size);

		status = ahci_write(&ahci_regs->ports[0], (offset / 512) + (cur_inode->extent.start_block * 8), (size + 512 - 1) / 512, pages, pages_needed);
		if (status != 0){
			for (int i = 0; i < pages_needed; i++) pmm_free_page(pages[i]);
			return 0;
		}

		for (int i = 0; i < pages_needed; i++) pmm_free_page(pages[i]);
		
		serial_print("[DFS READ] Successfully written to file\n");
		return size;
	}

	return 0;
}

inode_t *dfs_get_inode(const char *path){
	inode_t *cur_inode = &dfs_ctx.inode_table[1];

	uint64_t phys_buf = pmm_alloc_page();
	uint64_t virt_buf = phys_buf + DIRECT_OFFSET;
	memset((void*)virt_buf, 0, PAGE_SIZE_4KB);

	int len = 0;

	while (*path != '\0'){
		if (*path != '/'){
			path++;
			continue;
		}

		path++;

		while (path[len] != '/' && path[len] != '\0') len++;

		if ((cur_inode->type == DFS_TYPE_DIR) && *path != '\0'){
			uint32_t total_entries = cur_inode->size / sizeof(dir_entry_t);

			int status = ahci_read(&ahci_regs->ports[0], cur_inode->extent.start_block * 8, 8, &phys_buf, 1);
			if (status != 0){
				pmm_free_page(phys_buf);
				return NULL;
			}

			dir_entry_t *entry_list = (dir_entry_t *)virt_buf;
			int found = 0;
			for (int i = 0; i < total_entries; i++){
				dir_entry_t *entry = &entry_list[i];
				if (memcmp(entry->name, path, len) == 0){
					cur_inode = &dfs_ctx.inode_table[entry->inode_num];
					found = 1;
				}
			}
			
			if (found) continue;
			pmm_free_page(phys_buf);
			return NULL;
		} else {
			pmm_free_page(phys_buf);
			return NULL;
		}
	}

	pmm_free_page(phys_buf);
	return cur_inode;

	return NULL;
}

int dfs_file_read(struct FILE *file, const char *buf, size_t count){
	inode_t *inode = (inode_t *)file->private_data;
	if (file->position >= inode->size) return 0;
	if (file->position + count > inode->size) count = inode->size - file->position;

	uint64_t bytes_read = dfs_read(file->fp, buf, file->position, count);
	if (bytes_read == -1) return 0;

	file->position += bytes_read;
	
	return bytes_read;
}

int dfs_file_write(struct FILE *file, const char *buf, size_t count){
	inode_t *inode = (inode_t *)file->private_data;
	if (file->position >= inode->size) return 0;
	if (file->position + count > inode->size) count = inode->size - file->position;

	uint64_t bytes_written = dfs_write(file->fp, buf, file->position, count);
	if (bytes_written == -1) return 0;

	file->position += bytes_written;
	
	return bytes_written;
}

int dfs_file_close(struct FILE *file){
    kfree(file);
	return 0;
}