/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     virtio based transport channel device driver.
 */

#include <stddef.h>
#include <stdint.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_macros.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_core.h>
#include <fwk_status.h>
#include <mod_vring_mhu.h>
#include <mod_msg_smt.h>
#include <scmi_agents.h>

#include <stdlib.h>
#include <string.h>

#ifdef BUILD_HAS_CMSIS
#include <cmsis_os2.h>
#endif

#include <zephyr.h>
#include <drivers/virtio.h>
#include <drivers/virtio_mmio.h>
#include <metal/mutex.h>
#include <openamp/virtqueue.h>
#include <openamp/virtio.h>
#include <sys/printk.h>
#include <device.h>
#include <errno.h>
#include <soc.h>

/* Device implements SCMI notifications, or delayed responses. */
#define VIRTIO_SCMI_F_P2A_CHANNELS 0

/* Device implements 1 request virtqueue and 1 optional notification virtqueue */
#define VIRTIO_SCMI_VQ_REQ 0
#define VIRTIO_SCMI_VQ_NOTIF 1
#define VIRTIO_SCMI_VQ_MAX_CNT 2

/* A typical virtio_scmi request is made of 1 request buffer and 1 response buffer */
/* Response/Notification buffer idx */
#define VIRTIO_SCMI_OUT_MSG 0
/* Request buffer idx */
#define VIRTIO_SCMI_IN_MSG 1
#define VIRTIO_SCMI_REQ_MAX_CNT 2

#define VIRTIO_SCMI_MAX_DATA 128

struct mhu_smt_channel {
    struct virtqueue *vq;
    /* virtio user message */
    struct virtio_buf virtiobuf;
    /* vring user buffer */
    struct virtqueue_buf vqbufs[VIRTIO_SCMI_REQ_MAX_CNT];
    volatile bool locked;
    fwk_id_t id;
    struct mod_msg_smt_driver_input_api *api;
};

/* MHU device context */
struct mhu_device_ctx {
    /* vring interface */
    const struct device *virtio;

    /* Channel configuration data */
    struct mod_vring_mhu_channel_config *config;

    /* Number of slots (represented by sub-elements) */
    unsigned int slot_count;

    /* Table of SMT channels bound to the device */
    struct mhu_smt_channel *smt_channel_table;
};

/* MHU context */
struct mhu_ctx {
    /* Table of device contexts */
    struct mhu_device_ctx *device_ctx_table;

    /* Number of devices in the device context table*/
    unsigned int device_count;

#ifdef BUILD_HAS_CMSIS
#define SIGNAL_ISR_EVENT 0x01
    /*
     * Identifier given by the underlying OS to the thread created by the
     * module or element.
     */
    osThreadId_t os_thread_id;
#endif
};

static struct mhu_ctx mhu_ctx;

/*
 * Mailbox module driver API
 */

/*
 * Provide a notify interface to the Mailbox driver
 */
static int vring_get_next_req(const struct device *dev, struct mhu_smt_channel *smt_channel)
{
    struct virtqueue_buf *elem = smt_channel->vqbufs;
    int ret;

    ret = virtio_pop(dev, smt_channel->vq, &smt_channel->virtiobuf);

    if (!ret) {
        /* No new request */
        return 0;
    }

    /* Start processing a request on the channel/virtqueue */
    smt_channel->locked = true;

    /* signal to virtio scmi message module that a new request is ready to be processed */
    smt_channel->api->signal_message(smt_channel->id,
                                     elem[VIRTIO_SCMI_IN_MSG].buf,
                                     elem[VIRTIO_SCMI_IN_MSG].len,
                                     elem[VIRTIO_SCMI_OUT_MSG].buf,
                                     elem[VIRTIO_SCMI_OUT_MSG].len);

    return 1;
}

static void vring_put_prev_req(const struct device *dev, struct mhu_smt_channel *smt_channel, size_t size)
{
    struct virtqueue_buf *elem = smt_channel->vqbufs;

    /* 1st buffer is the answer to other side */
    elem[VIRTIO_SCMI_OUT_MSG].len = size;

    /*
     * 2nd buffer is to receive the request from other side.
     * When we owns the bus, we provisionned this buffer for the other side and must
     * init the lenght to default size.
     *
     * This is not used by notification channel.
     */
    elem[VIRTIO_SCMI_IN_MSG].len = VIRTIO_SCMI_MAX_DATA;

    virtio_push(dev, smt_channel->vq, &smt_channel->virtiobuf);
}

static int raise_queue_notify(fwk_id_t channel_id, size_t size)
{
    size_t elt_idx = fwk_id_get_element_idx(channel_id);
    struct mhu_device_ctx *device_ctx = &mhu_ctx.device_ctx_table[elt_idx];
    size_t chnl_idx = fwk_id_get_sub_element_idx(channel_id);
    struct mhu_smt_channel *smt_channel = &device_ctx->smt_channel_table[chnl_idx];

    /* put current request */
    vring_put_prev_req(device_ctx->virtio, smt_channel, size);

    /* check if a request is pending */
    if (vring_get_next_req(device_ctx->virtio, smt_channel)) {
        return FWK_SUCCESS;
    }

    /* No pending request */
    smt_channel->locked = false;

    return FWK_SUCCESS;
}

const struct mod_msg_smt_driver_ouput_api vring_mod_smt_driver_api = {
    .raise_notification = raise_queue_notify,
};

static void vring_virtio_state(const struct device *dev, void *user_data, int state)
{
    struct mhu_device_ctx *device_ctx = user_data;
    struct mhu_smt_channel *smt_channel;
    unsigned int slot;

    if (!state) {
        /*
         * Connection with client has been lost. The ongoing requests are
         * meaningless and should not be sent back to client once done.
         * Buffer descriptor index is used to send back the request to the
         * client. Reset it to discard the request.
         */
        for (slot = 0; slot < device_ctx->slot_count; slot++) {
            smt_channel = &device_ctx->smt_channel_table[slot];

            smt_channel->virtiobuf.index = (unsigned int)(-1);
        }
    }
}

/*
 * Virtio vring helpers
 */

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof(((type *) 0)->member) *__mptr = (ptr);     \
        (type *) ((char *) __mptr - offsetof(type, member));})
#endif

static void vring_virtio_cb(struct virtqueue *vq)
{
    struct virtio_mmio_data *driver_data = container_of(vq->vq_dev, struct virtio_mmio_data, device);
    struct mhu_device_ctx *device_ctx = driver_data->user_data;
    struct mhu_smt_channel *smt_channel = &device_ctx->smt_channel_table[vq->vq_queue_index];
    uint32_t flags;

    if (!(smt_channel->locked) &&
        !(vring_get_next_req(device_ctx->virtio, smt_channel))) {
            return;
    }

#ifdef BUILD_HAS_CMSIS
    /*
     * A notification is about to be raised
     *
     * Either SCMI server is already processing a request
     * Or there is a new pending request
     *
     * Make sure to wakeup the SCMI server.
     *
     * This can generate spurious iteration of main thread loop but no
     * spurious wakeup and it will ensure that it never misses a
     * new request.
     */
    flags = osThreadFlagsSet(mhu_ctx.os_thread_id, SIGNAL_ISR_EVENT);
    if ((int32_t)flags < 0) {
        FWK_LOG_CRIT("[VRING] ThreadSetFlag error %d in %s", FWK_E_OS, __func__);
        return;
    }
#endif
}

static const char *scmi_vio_vqueue_names[] = { "request", "notification" };

static virtio_callback_t vring_vio_complete_callbacks[] = {
    vring_virtio_cb,
    vring_virtio_cb
};

struct __attribute((packed)) mod_virtio_smt_memory {
    uint32_t message_header;
    uint32_t payload[];
};

int vring_init_channel(struct mhu_device_ctx *device_ctx, unsigned int slot_count)
{
    struct mhu_smt_channel *smt_channel;
    struct virtqueue *vqs[VIRTIO_SCMI_VQ_MAX_CNT];
    const struct device *virtio;
    int ret, size, i;

    if (device_ctx->config->device_label == NULL) {
        return FWK_SUCCESS;
    }

    device_ctx->virtio = NULL;
    virtio = device_get_binding(device_ctx->config->device_label);
    if (!virtio) {
        FWK_LOG_ERR("[VRING] Device driver %s not found\n",device_ctx->config->device_label);
        return FWK_E_DEVICE;
    }

    /* Can't have more than 2 virtqueues for virtio SCMI */
    if (slot_count > VIRTIO_SCMI_VQ_MAX_CNT) {
        return FWK_E_PARAM;
    }

    /* Set notification feature if there are 2 channels */
    if (slot_count == VIRTIO_SCMI_VQ_MAX_CNT) {
        virtio_set_device_features(virtio, 1 << VIRTIO_SCMI_F_P2A_CHANNELS);
    }

    /* Get virqueues if any */
    ret = virtio_get_vqs(virtio, slot_count, vqs, vring_vio_complete_callbacks,
                  scmi_vio_vqueue_names);
    if (ret) {
        return FWK_E_DEVICE;
    }

    size = virtio_max_data_size_get(virtio);
    /* set virtqeue on each channel */
    for (i = 0; i < slot_count; i++) {
        /* Save vqs */
        smt_channel = &device_ctx->smt_channel_table[i];
        smt_channel->vq = vqs[i];

        /* preallocate buffers : only necessary for mmio driver */
        smt_channel->vqbufs[VIRTIO_SCMI_OUT_MSG].buf = virtio_get_buffer(virtio, smt_channel->vq, VIRTIO_SCMI_OUT_MSG);
        smt_channel->vqbufs[VIRTIO_SCMI_OUT_MSG].len = size;

        /* set virtiobuf */
        smt_channel->virtiobuf.out_num = 1;
        smt_channel->virtiobuf.buffers = smt_channel->vqbufs;

        if (i == VIRTIO_SCMI_VQ_REQ) {
            /*
             * 1st channel receives request and needs both in and out buffers
             * 2nd channel only sends notification and needs out buffer only
             */
            smt_channel->virtiobuf.in_num = 1;
            /* preallocated buffers : only necessary for mmio driver */
            smt_channel->vqbufs[VIRTIO_SCMI_IN_MSG].buf = virtio_get_buffer(virtio, smt_channel->vq, VIRTIO_SCMI_IN_MSG);
            smt_channel->vqbufs[VIRTIO_SCMI_IN_MSG].len = size;

            if (smt_channel->vqbufs[VIRTIO_SCMI_IN_MSG].buf != NULL)
                /*
                 * When we owns of the link, buffer is not null and it needs to
                 * provision the virtqueue to receive request
                 */
                virtio_push(virtio, smt_channel->vq, &smt_channel->virtiobuf);
        }
    }

    /* Save device */
    device_ctx->virtio = virtio;

    /* Save device ctx */
    virtio_set_callback(virtio, vring_virtio_state, (void *)device_ctx);

    /* Ready to process request */
    virtio_set_enabled(virtio, 1);

    return FWK_SUCCESS;
}

/*
 * Framework handlers
 */

static int vring_module_init(fwk_id_t module_id, unsigned int device_count,
                      const void *data)
{
    if (device_count == 0) {
        return FWK_E_PARAM;
    }

    mhu_ctx.device_ctx_table = fwk_mm_calloc(device_count,
                                             sizeof(*mhu_ctx.device_ctx_table));
    if (mhu_ctx.device_ctx_table == NULL) {
        return FWK_E_NOMEM;
    }

    mhu_ctx.device_count = device_count;

    return FWK_SUCCESS;
}

static int vring_device_init(fwk_id_t device_id, unsigned int slot_count,
                           const void *data)
{
    unsigned int slot;
    size_t elt_idx = fwk_id_get_element_idx(device_id);
    struct mhu_device_ctx *device_ctx = &mhu_ctx.device_ctx_table[elt_idx];
    struct mhu_smt_channel *smt_channel;

    device_ctx->config = (struct mod_vring_mhu_channel_config*)data;

    device_ctx->smt_channel_table = fwk_mm_calloc(slot_count,
                                                  sizeof(device_ctx->smt_channel_table[0]));

    device_ctx->slot_count = slot_count;

    for (slot = 0; slot < device_ctx->slot_count; slot++) {
            smt_channel = &device_ctx->smt_channel_table[slot];

            smt_channel->id.value = 0;
            smt_channel->locked = false;
    }

    vring_init_channel(device_ctx, slot_count);

    return FWK_SUCCESS;
}

static int vring_bind(fwk_id_t id, unsigned int round)
{
    int status;
    unsigned int slot;
    struct mhu_device_ctx *device_ctx;
    struct mhu_smt_channel *smt_channel;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    if (round == 1) {

        device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

        for (slot = 0; slot < device_ctx->slot_count; slot++) {
            smt_channel = &device_ctx->smt_channel_table[slot];

            if (smt_channel->id.value == 0) {
                continue;
            }

            status = fwk_module_bind(smt_channel->id,
                device_ctx->config->driver_api_id,
                &smt_channel->api);

            if (status != FWK_SUCCESS) {
                return status;
            }
        }
    }

    return FWK_SUCCESS;
}

static int vring_process_bind_request(fwk_id_t source_id,
                      fwk_id_t target_id,
                      fwk_id_t api_id,
                      const void **api)
{
    size_t elt_idx;
    unsigned int slot;
    struct mhu_device_ctx *device_ctx;
    struct mhu_smt_channel *smt_channel;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_SUB_ELEMENT)) {
        return FWK_E_ACCESS;
    }

    if (fwk_id_get_api_idx(api_id) != 0) {
        return FWK_E_PARAM;
    }

    elt_idx = fwk_id_get_element_idx(target_id);
    if (elt_idx >= mhu_ctx.device_count) {
        return FWK_E_PARAM;
    }

    device_ctx = &mhu_ctx.device_ctx_table[elt_idx];

    slot = fwk_id_get_sub_element_idx(target_id);
    if (slot >= device_ctx->slot_count) {
        return FWK_E_PARAM;
    }

    smt_channel = &device_ctx->smt_channel_table[slot];

    smt_channel->id = source_id;
    *api = &vring_mod_smt_driver_api;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_CMSIS
static void vring_thread_function(void *arg)
{
    uint32_t flags, signals = SIGNAL_ISR_EVENT;

    for (;;) {
        flags = osThreadFlagsWait(signals, osFlagsWaitAny, osWaitForever);

        if (!(flags & signals)) {
            continue;
        }

        fwk_process_event_queue();

        fwk_log_flush();
    }
}

#ifndef FMW_STACK_SIZE
#    define FMW_STACK_SIZE 1536
#endif

int vring_thread_create(struct mhu_ctx *module_ctx)
{
    osThreadAttr_t thread_attr = {
        .stack_size = FMW_STACK_SIZE,
    };

    module_ctx->os_thread_id =
        osThreadNew(vring_thread_function, module_ctx, &thread_attr);

    if (module_ctx->os_thread_id == NULL) {
        FWK_LOG_CRIT("[VRING] Thread Create error %d in %s", FWK_E_OS, __func__);
        return FWK_E_OS;
    }

    return FWK_SUCCESS;
}

#else
int vring_thread_create(struct mhu_ctx *module_ctx)
{
    return FWK_SUCCESS;
}
#endif

static int vring_start(fwk_id_t id)
{
    vring_thread_create(&mhu_ctx);
    return FWK_SUCCESS;
}

/* OPTEE_MHU module definition */
const struct fwk_module module_vring_mhu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = vring_module_init,
    .element_init = vring_device_init,
    .bind = vring_bind,
    .start = vring_start,
    .process_bind_request = vring_process_bind_request,
};
