#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/drivers/pci.h>

#define VIRTIO_VENDOR_ID   0x1AF4
#define VIRTIO_GPU_DEVID   0x1050

typedef struct {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
} virtio_pci_dev_t;

void find_virtio_gpu(void);