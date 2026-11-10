#include <drivers/virtio/virtgpu.h>

uint64_t fence_count = 0;

virtio_gpu_cmd_node_t *cmd_list_head = NULL;
virtio_gpu_cmd_node_t *cmd_list_tail = NULL;
uint64_t cmd_count = 0;

void virtio_gpu_push_command(void *cmd, uint32_t cmd_len, uint32_t resp_len){
    virtio_gpu_cmd_node_t *node = (virtio_gpu_cmd_node_t *)kmalloc(sizeof(virtio_gpu_cmd_node_t));
    
    memcpy(node->cmd_data, cmd, cmd_len);
    node->cmd_len = cmd_len;
    node->resp_len = resp_len;
    
    add_cmd_to_list(node);
}

void virtio_gpu_flush_commands(virtqueue_t *vq){
    if (!cmd_count) return;

    virtio_gpu_cmd_node_t *cur_node = cmd_list_head;
    uint16_t load_cmd_count = cmd_count > virtio_gpu.vq0->queue_size / 2 ? virtio_gpu.vq0->queue_size / 2 : cmd_count;
    for (int i = 0; i < load_cmd_count; i++){
        uint16_t desc_req = i;
        uint16_t desc_resp = load_cmd_count + i;

        vq->desc[desc_req].addr = vmm_get_paddr(read_cr3(), (uint64_t)cur_node->cmd_data);
        vq->desc[desc_req].len = cur_node->cmd_len;
        vq->desc[desc_req].flags = VIRTQ_DESC_F_NEXT;
        vq->desc[desc_req].next = (uint16_t)desc_resp;

        vq->desc[desc_resp].addr = vmm_get_paddr(read_cr3(), (uint64_t)cur_node->resp_data);
        vq->desc[desc_resp].len = cur_node->resp_len;
        vq->desc[desc_resp].flags = VIRTQ_DESC_F_WRITE;
        vq->desc[desc_resp].next = 0;

        vq->avail->ring[(vq->avail->idx + desc_req) % vq->queue_size] = desc_req;

        if (i != load_cmd_count - 1) cur_node = cur_node->next;
    }

    struct virtio_gpu_ctrl_hdr *hdr = (struct virtio_gpu_ctrl_hdr *)cur_node->cmd_data;
    hdr->flags |= VIRTIO_GPU_FLAG_FENCE;
    fence_count += load_cmd_count; hdr->fence_id = fence_count;
    mfence();

    uint64_t saved_rflags = read_rflags();
    write_rflags(saved_rflags & ~0x200);
    virtio_gpu.virtgpu_blocked_thread = current_thread;
    virtio_gpu.virtgpu_blocked_thread->state = BLOCKED;
    vq->avail->idx += load_cmd_count;
    mfence();
    *virtio_gpu.regions.notify_cfg = 0;
    mfence();
    yield(1);

    cur_node = cmd_list_head;

    for (int i = 0; i < load_cmd_count; i++){
        virtio_gpu_cmd_node_t *next_node = cur_node->next;
        kfree(cur_node);
        cur_node = next_node;
    }

    cmd_list_head = cur_node;
    if (!cmd_list_head) cmd_list_tail = NULL;
    cmd_count -= load_cmd_count;

    serial_print("[VIRTIO] Commands loaded: %d", load_cmd_count);
}

void virtio_gpu_create_canvas(uint32_t width, uint32_t height){
    struct virtio_gpu_resource_create_2d cmd = {0};

    cmd.hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
    cmd.hdr.flags = 0;
    cmd.hdr.fence_id = 0;
    cmd.hdr.ctx_id = 0;

    cmd.resource_id = 1;
    cmd.format = VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM;
    cmd.width = width;
    cmd.height = height;
    
    virtio_gpu_push_command(&cmd, sizeof(cmd), 24);
}

void add_cmd_to_list(virtio_gpu_cmd_node_t *node){
    if (!cmd_list_head){
        node->next = node->prev = NULL;
        cmd_list_head = node;
        cmd_list_tail = node;
    } else {
        node->next = NULL;
        node->prev = cmd_list_tail;
        node->prev->next = node;
        cmd_list_tail = node;
    }
    cmd_count++;
}
