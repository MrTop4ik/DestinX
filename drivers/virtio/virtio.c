#include <drivers/virtio.h>

virtio_gpu_pci_dev_t virtio_gpu;

void find_virtio_gpu(void){
    for (uint16_t bus = 0; bus < 256; bus++){
        for (uint8_t dev = 0; dev < 32; dev++){
            uint32_t reg_id = pci_read_dword((uint8_t)bus, dev, 0, 0x00);
            uint16_t vendor_id = reg_id & 0xFFFF;

            if (vendor_id == 0xFFFF) continue;
            
            uint8_t header_type = (pci_read_dword(bus, dev, 0, 0x0C) >> 16) & 0xFF;
            uint8_t max_func = (header_type & 0x80) ? 8 : 1;

            for (uint8_t func = 0; func < max_func; func++){
                if (func > 0){
                    reg_id = pci_read_dword((uint8_t)bus, dev, func, 0x00);
                    vendor_id = reg_id & 0xFFFF;
                    if (vendor_id == 0xFFFF) continue;
                }

                if (vendor_id == VIRTIO_VENDOR_ID){
                    uint16_t device_id = reg_id >> 16;

                    if (device_id == VIRTIO_GPU_DEVID){
                        virtio_gpu.bus = bus;
                        virtio_gpu.dev = dev;
                        virtio_gpu.func = func;
                        serial_print("[VIRTIO] Virtio GPU Was Found\n");
                        return;
                    }
                }
            }
        }
    }

    serial_print("[VIRTIO] Virtio GPU Was Found\n");
}

void parse_virtio_gpu_capabilities(void){
    uint32_t cap_reg = pci_read_dword(virtio_gpu.bus, virtio_gpu.dev, virtio_gpu.func, 0x34);
    uint16_t cap_ptr = (uint16_t)(cap_reg & 0xFFFF);

    while (cap_ptr != 0){
        uint32_t cap_header = pci_read_dword(virtio_gpu.bus, virtio_gpu.dev, virtio_gpu.func, cap_ptr);
        uint8_t cap_id = (uint8_t)(cap_header & 0xFF);
        uint8_t next_ptr = (uint8_t)((cap_header >> 8) & 0xFF);

        if (cap_id == 0x09){
            uint8_t cfg_type = (uint8_t)((cap_header >> 24) & 0xFF);
            
            uint8_t bar_num = pci_read_dword(virtio_gpu.bus, virtio_gpu.dev, virtio_gpu.func, cap_ptr + 4) & 0xFF;
            uint32_t bar_offset = pci_read_dword(virtio_gpu.bus, virtio_gpu.dev, virtio_gpu.func, cap_ptr + 8);

            uint32_t bar_info = pci_read_dword(virtio_gpu.bus, virtio_gpu.dev, virtio_gpu.func, 0x10 + bar_num * 4);
            uint8_t bar_flags = bar_info & 0x0F;
            uint64_t bar_address = bar_info & 0xFFFFFFF0;
            if (bar_info & (1 << 2)) bar_address |= (uint64_t)(pci_read_dword(virtio_gpu.bus, virtio_gpu.dev, virtio_gpu.func, 0x10 + bar_num * 4 + 4)) << 32;

            switch (cfg_type){
                case COMMON_CFG_TYPE:
                    serial_print("[VIRTIO] Found Common Config\n");
                    virtio_gpu.regions.common_bar = bar_num;
                    virtio_gpu.regions.common_offset = bar_offset;
                    virtio_gpu.regions.common_bar_addr = bar_address;
                    break;
                case NOTIFY_CFG_TYPE:
                    serial_print("[VIRTIO] Found Notify Config\n");
                    uint32_t multiplier = pci_read_dword(virtio_gpu.bus, virtio_gpu.dev, virtio_gpu.func, cap_ptr + 12);
                    virtio_gpu.regions.notify_bar = bar_num;
                    virtio_gpu.regions.notify_offset = bar_offset;
                    virtio_gpu.regions.notify_multiplier = multiplier;
                    virtio_gpu.regions.notify_bar_addr = bar_address;
                    break;
                case DEVICE_CFG_TYPE:
                    serial_print("[VIRTIO] Found Device Config\n");
                    virtio_gpu.regions.device_bar = bar_num;
                    virtio_gpu.regions.device_offset = bar_offset;
                    virtio_gpu.regions.device_bar_addr = bar_address;
                    break;
            }
        }

        cap_ptr = next_ptr;
    }
}

void map_virtio_gpu_capablities(void){
    vmm_map_page(read_cr3(), virtio_gpu.regions.common_bar_addr + virtio_gpu.regions.common_offset, COMMON_CFG_VIRT, PAGE_SIZE_4KB, PTE_WRITABLE | PTE_WT | PTE_CD);
    vmm_map_page(read_cr3(), virtio_gpu.regions.notify_bar_addr + virtio_gpu.regions.notify_offset, NOTIFY_CFG_VIRT, PAGE_SIZE_4KB, PTE_WRITABLE | PTE_WT | PTE_CD);
    vmm_map_page(read_cr3(), virtio_gpu.regions.device_offset + virtio_gpu.regions.device_offset, DEVICE_CFG_VIRT, PAGE_SIZE_4KB, PTE_WRITABLE | PTE_WT | PTE_CD);

    virtio_gpu.regions.common_cfg = (volatile struct virtio_pci_common_cfg *)COMMON_CFG_VIRT;
    virtio_gpu.regions.notify_cfg = (volatile uint64_t *)NOTIFY_CFG_VIRT;
    virtio_gpu.regions.device_cfg = (volatile uint64_t *)DEVICE_CFG_VIRT;
}

void init_virtio_gpu_handshake(void){
    virtio_gpu.regions.common_cfg->device_status = VIRTIO_STATUS_RESET;
    pit_wait_ms(10);

    virtio_gpu.regions.common_cfg->device_status |= VIRTIO_STATUS_ACKNOWLEDGE;
    virtio_gpu.regions.common_cfg->device_status |= VIRTIO_STATUS_DRIVER;
    serial_print("[VIRTIO] Handshake\n");
}

void init_virtio_gpu_features(void){
    virtio_gpu.regions.common_cfg->device_feature_select = 0;
    uint32_t device_features_low = virtio_gpu.regions.common_cfg->device_feature;

    virtio_gpu.regions.common_cfg->device_feature_select = 1;
    uint32_t device_features_high = virtio_gpu.regions.common_cfg->device_feature;

    uint64_t device_features = (uint64_t)(device_features_high) << 32 | device_features_low;

    uint64_t driver_features = 0;

    if (device_features & VIRTIO_F_VERSION_1)  driver_features |= VIRTIO_F_VERSION_1;
    else {
        virtio_gpu.regions.common_cfg->device_status |= VIRTIO_STATUS_FAILED;
        serial_print("[VIRTIO] Modern 1.x Not Supported\n");
        return;
    }

    virtio_gpu.regions.common_cfg->driver_feature_select = 0;
    virtio_gpu.regions.common_cfg->driver_feature = (uint32_t)(driver_features & 0xFFFFFFFF);

    virtio_gpu.regions.common_cfg->driver_feature_select = 1;
    virtio_gpu.regions.common_cfg->driver_feature = (uint64_t)(driver_features >> 32);

    virtio_gpu.regions.common_cfg->device_status = VIRTIO_STATUS_FEATURES_OK;

    if (!(virtio_gpu.regions.common_cfg->device_status & VIRTIO_STATUS_FEATURES_OK)){
        virtio_gpu.regions.common_cfg->device_status |= VIRTIO_STATUS_FAILED;
        serial_print("[VIRTIO] Smth went wrong during features init\n");
        return;
    }

    serial_print("[VIRTIO] Feautes init success\n");    
}

void create_virtio_gpu_queue(virtqueue_t *vq, uint16_t count){
    uint64_t paddr = pmm_alloc_pages(3); 
    memset((void*)(paddr + DIRECT_OFFSET), 0, PAGE_SIZE_4KB * 3);

    vq->raw_memory_block = paddr;
    vq->queue_size = count;

    vq->desc = (struct vring_desc *)paddr;
    vq->avail = (struct vring_avail *)(paddr + PAGE_MASK_4KB);
    vq->used = (struct vring_used *)(paddr + PAGE_SIZE_4KB * 2);
}

void configure_and_start_queue(void){
    virtio_gpu.regions.common_cfg->queue_select = 0;

    uint16_t hardware_queue_size = virtio_gpu.regions.common_cfg->queue_size;

    virtio_gpu.vq0 = kmalloc(sizeof(virtqueue_t));
    create_virtio_gpu_queue(virtio_gpu.vq0, hardware_queue_size);

    virtio_gpu.regions.common_cfg->queue_size = hardware_queue_size;

    virtio_gpu.regions.common_cfg->queue_desc  = (uint64_t)virtio_gpu.vq0->desc;
    virtio_gpu.regions.common_cfg->queue_avail = (uint64_t)virtio_gpu.vq0->avail;
    virtio_gpu.regions.common_cfg->queue_used  = (uint64_t)virtio_gpu.vq0->used;

    virtio_gpu.regions.common_cfg->queue_enable = 1;

    serial_print("[VIRTIO] Virtqueue 0 Successfully Activated with qsize %d\n", hardware_queue_size);
}
