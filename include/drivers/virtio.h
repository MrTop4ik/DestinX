#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/drivers/pci.h>
#include <arch/x86_64/drivers/pit.h>

#define VIRTIO_VENDOR_ID 0x1AF4
#define VIRTIO_GPU_DEVID 0x1050

#define VIRTIO_GPU_F_VIRGL (1ULL << 0)
#define VIRTIO_F_VERSION_1 (1ULL << 32)

#define COMMON_CFG_TYPE 1
#define NOTIFY_CFG_TYPE 2
#define DEVICE_CFG_TYPE 4

#define COMMON_CFG_VIRT 0xFFFFFFFFFFFFC0000
#define NOTIFY_CFG_VIRT 0xFFFFFFFFFFFFD0000
#define DEVICE_CFG_VIRT 0xFFFFFFFFFFFFE0000


#define VIRTIO_STATUS_RESET          0x00
#define VIRTIO_STATUS_ACKNOWLEDGE    0x01
#define VIRTIO_STATUS_DRIVER         0x02
#define VIRTIO_STATUS_FEATURES_OK    0x08
#define VIRTIO_STATUS_DRIVER_OK      0x80
#define VIRTIO_STATUS_FAILED         0x80

typedef struct {
    uint8_t  common_bar;
    uint32_t common_offset;
    uint64_t common_bar_addr;
    uint8_t  notify_bar;
    uint32_t notify_offset;
    uint64_t notify_bar_addr;
    uint32_t notify_multiplier;
    uint8_t  device_bar;
    uint32_t device_offset;
    uint64_t device_bar_addr;
} vgpu_regions_t;

typedef struct {
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    vgpu_regions_t regions;
} virtio_gpu_pci_dev_t;

struct virtio_pci_common_cfg {
    uint32_t device_feature_select;
    uint32_t device_feature;       
    uint32_t driver_feature_select;
    uint32_t driver_feature;       
    uint16_t msix_config;          
    uint16_t num_queues;           
    uint8_t  device_status;        
    uint8_t  config_generation;
    uint16_t queue_select;
    uint16_t queue_size;
    uint16_t queue_msix_vector;
    uint16_t queue_enable;
    uint16_t queue_notify_off;
    uint64_t queue_desc;
    uint64_t queue_avail;
    uint64_t queue_used;
}__attribute__((__packed__));

struct virtio_gpu_config {
    uint32_t events_read;
    uint32_t events_clear;
    uint32_t num_scanouts;
    uint32_t reserved;
}__attribute__((__packed__));


void find_virtio_gpu(void);
void parse_virtio_gpu_capabilities(void);
void map_virtio_gpu_capablities(void);
void init_virtio_gpu_handshake(void);
void init_virtio_gpu_features(void);