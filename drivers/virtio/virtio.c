#include <drivers/virtio.h>

virtio_pci_dev_t virtio_gpu = {0, 0, 0};

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