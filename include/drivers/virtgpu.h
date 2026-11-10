#pragma once
#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/drivers/serial.h>
#include <libc/string.h>
#include <mm/vmalloc.h>
#include <drivers/virtio.h>

#define VIRTIO_GPU_CMD_RESOURCE_CREATE_2D      0x0101
#define VIRTIO_GPU_RESP_OK_NODATA              0x1100
#define VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM       1

#define MAX_REQ_LEN     128
#define MAX_RESP_LEM    64

#define VIRTIO_GPU_FLAG_FENCE (1 << 0)

struct virtio_gpu_ctrl_hdr {
    uint32_t type;
    uint32_t flags;
    uint64_t fence_id;
    uint32_t ctx_id;
    uint32_t padding;
}__attribute__((__packed__));

struct virtio_gpu_resource_create_2d {
    struct virtio_gpu_ctrl_hdr hdr;
    uint32_t resource_id;
    uint32_t format;
    uint32_t width;
    uint32_t height;
}__attribute__((__packed__));

struct virtio_gpu_ctrl_resp {
    struct virtio_gpu_ctrl_hdr hdr; 
}__attribute__((__packed__));

typedef struct virtio_gpu_cmd_node {
    uint8_t  cmd_data[MAX_REQ_LEN] __attribute__((aligned(64)));
    uint32_t cmd_len;
    uint8_t  resp_data[MAX_RESP_LEM] __attribute__((aligned(64)));
    uint32_t resp_len;
    struct virtio_gpu_cmd_node *next;
    struct virtio_gpu_cmd_node *prev;
} virtio_gpu_cmd_node_t;

void virtio_gpu_push_command(void *cmd, uint32_t cmd_len, uint32_t resp_len);
void virtio_gpu_flush_commands(virtqueue_t *vq);
void virtio_gpu_create_canvas(uint32_t width, uint32_t height);
void add_cmd_to_list(virtio_gpu_cmd_node_t *node);
