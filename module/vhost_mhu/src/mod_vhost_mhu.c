/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      vhost user based transport channel Device Driver.
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
#include <mod_vhost_mhu.h>
#include <mod_msg_smt.h>
#include <scmi_agents.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <gio/gunixsocketaddress.h>
#include <glib-unix.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include "subprojects/libvhost-user/libvhost-user-glib.h"
#include "subprojects/libvhost-user/libvhost-user.h"

struct iovec {
    void *iov_base;
    size_t iov_len;
};

struct mhu_smt_channel {
    fwk_id_t id;
    struct mod_msg_smt_driver_input_api *api;

    /* vhost user message */
    VuVirtqElement *elem;
};

/* MHU device context */
struct mhu_device_ctx {
    /* Vhost user inteface */
    VugDev dev;
    GSocket *socket;
    GThread *thread;

    /* Channel configuration data */
    struct mod_vhost_mhu_channel_config *config;

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
};

static struct mhu_ctx mhu_ctx;

/*
 * Virtio vhost helpers
 */
/* Device implements some SCMI notifications, or delayed responses. */
#define VIRTIO_SCMI_F_P2A_CHANNELS 0

/* Definitions from virtio-scmi specifications */
#define VHOST_USER_SCMI_MAX_QUEUES       2

/* vhost-user-scmi definitions */
#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof(((type *) 0)->member) *__mptr = (ptr);     \
        (type *) ((char *) __mptr - offsetof(type, member));})
#endif

static uint64_t vscmi_get_features(VuDev *dev)
{
    uint64_t features = 1ull << VIRTIO_SCMI_F_P2A_CHANNELS;

    return features;
}

static void vscmi_set_features(VuDev *dev, uint64_t features)
{
    if (features) {
        g_autoptr(GString) s = g_string_new("Requested un-handled feature");
        g_string_append_printf(s, " 0x%" PRIx64 "", features);
        g_debug("%s: %s", __func__, s->str);
    }
}

void vhost_signal_message(struct mhu_device_ctx *device_ctx, unsigned int channel_idx, VuVirtqElement *elem)
{
    struct mhu_smt_channel *smt_channel;

    smt_channel = &device_ctx->smt_channel_table[channel_idx];
    smt_channel->elem = elem;

    smt_channel->api->signal_message(smt_channel->id,
                                     elem->out_sg[0].iov_base,
                                     elem->out_sg[0].iov_len,
                                     elem->in_sg[0].iov_base,
                                     elem->in_sg[0].iov_len);

    fwk_process_event_queue();

    fwk_log_flush();
}

static void vscmi_handle_request_ctrl(VuDev *dev, int qidx)
{
    struct mhu_device_ctx *device_ctx = container_of(dev, struct mhu_device_ctx, dev.parent);

    VuVirtq *vq = vu_get_queue(dev, qidx);

    for (;;) {
        VuVirtqElement *elem;

        elem = vu_queue_pop(dev, vq, sizeof(VuVirtqElement));
        if (!elem) {
            break;
        }

        g_debug("%s: %s:%d got queue (in %d, out %d)", __func__, device_ctx->config->socket_path, qidx, elem->in_num,
                elem->out_num);

        vhost_signal_message(device_ctx, qidx, elem);
    }
}

void vscmi_host_user_notification(VuDev *dev, VuVirtqElement *elem, int qidx, size_t in_hdr_len)
{
    VuVirtq *vq = vu_get_queue(dev, qidx);

    vu_queue_push(dev, vq, elem, in_hdr_len);

    vu_queue_notify(dev, vq);
}

static void
vscmi_queue_set_started(VuDev *dev, int qidx, bool started)
{
    VuVirtq *vq = vu_get_queue(dev, qidx);

    g_debug("%s: idx %d started %d", __func__, qidx, started);

    vu_set_queue_handler(dev, vq, started ? vscmi_handle_request_ctrl : NULL);
}

static void vscmi_panic(VuDev *dev, const char *msg)
{
    g_critical("%s\n", msg);
    exit(EXIT_FAILURE);
}

static int vscmi_init_channels(struct mhu_device_ctx *device_ctx);

static gpointer server_wait_vm_thread(gpointer data)
{
    struct mhu_device_ctx *device_ctx = (struct mhu_device_ctx *)data;

    vscmi_init_channels(device_ctx);

    return NULL;
}

static void vscmi_destroy_channels(struct mhu_device_ctx *device_ctx);

/*
 * vi2c_process_msg: process messages of vhost-user interface
 *
 * Any that are not handled here are processed by the libvhost library
 * itself.
 */
static int vscmi_process_msg(VuDev *dev, VhostUserMsg *msg, int *do_reply)
{
    struct mhu_device_ctx *device_ctx = container_of(dev, struct mhu_device_ctx, dev.parent);

    if (msg->request == VHOST_USER_NONE) {
        g_debug("%s VHOST_USER_NONE", __func__);

        vscmi_destroy_channels(device_ctx);
        device_ctx->thread = g_thread_new (device_ctx->config->socket_path, server_wait_vm_thread, device_ctx);

        return 1;
    }

    return 0;
}

static const VuDevIface vuiface = {
    .set_features = vscmi_set_features,
    .get_features = vscmi_get_features,
    .queue_set_started = vscmi_queue_set_started,
    .process_msg = vscmi_process_msg,
};

static int vscmi_init_channels(struct mhu_device_ctx *device_ctx)
{
    GError *error = NULL;
    const gchar *socket_path = (const gchar *) device_ctx->config->socket_path;

    /*
     * Now create a vhost-user socket that we will receive messages
     * on. Once we have our handler set up we can enter the glib main
     * loop.
     */
    if (!device_ctx->config->socket_path) {
        return 0;
    }

    g_autoptr(GSocketAddress) addr = g_unix_socket_address_new(socket_path);
    g_autoptr(GSocket) bind_socket = g_socket_new(G_SOCKET_FAMILY_UNIX, G_SOCKET_TYPE_STREAM,
                                                  G_SOCKET_PROTOCOL_DEFAULT, &error);

    if (!g_socket_bind(bind_socket, addr, false, &error)) {
        g_printerr("Failed to bind to socket at %s (%s).\n",
                   socket_path, error->message);
        return FWK_E_DEVICE;
    }

    if (!g_socket_listen(bind_socket, &error)) {
        g_printerr("Failed to listen on socket %s (%s).\n",
                   socket_path, error->message);
        return FWK_E_DEVICE;
    }

    g_message("awaiting connection to %s", socket_path);
    device_ctx->socket = g_socket_accept(bind_socket, NULL, &error);
    if (!device_ctx->socket) {
        g_printerr("Failed to accept on socket %s (%s).\n",
                   socket_path, error->message);
        return FWK_E_DEVICE;
    }

    if (!vug_init(&device_ctx->dev, VHOST_USER_SCMI_MAX_QUEUES, g_socket_get_fd(device_ctx->socket),
        vscmi_panic, &vuiface)) {
        g_printerr("Failed to initialize libvhost-user-glib.\n");
        return FWK_E_DEVICE;
    }

    return FWK_SUCCESS;
}

static void vscmi_destroy_channels(struct mhu_device_ctx *device_ctx)
{
    if (device_ctx->socket != NULL) {
        vug_deinit(&device_ctx->dev);
        g_socket_close(device_ctx->socket, NULL);
        g_object_unref(device_ctx->socket);
        device_ctx->socket = NULL;
    }

    unlink(device_ctx->config->socket_path);
}

/*
 * Mailbox module driver API
 */

/*
 * Provide a notify interface to the Mailbox driver
 */
static int raise_queue_notify(fwk_id_t channel_id, size_t size)
{
    size_t elt_idx = fwk_id_get_element_idx(channel_id);
    struct mhu_device_ctx *device_ctx = &mhu_ctx.device_ctx_table[elt_idx];
    size_t chnl_idx = fwk_id_get_sub_element_idx(channel_id);
    struct mhu_smt_channel *smt_channel = &device_ctx->smt_channel_table[chnl_idx];

    vscmi_host_user_notification(&device_ctx->dev.parent, smt_channel->elem, chnl_idx, size);

    /* There should be a message in the mailbox */
    return FWK_SUCCESS;
}

const struct mod_msg_smt_driver_ouput_api vhost_mod_smt_driver_api = {
    .raise_notification = raise_queue_notify,
};

/*
 * Framework handlers
 */

static int vhost_init(fwk_id_t module_id, unsigned int device_count,
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

static int vhost_device_init(fwk_id_t device_id, unsigned int slot_count,
                           const void *data)
{
    unsigned int slot;
    size_t elt_idx = fwk_id_get_element_idx(device_id);
    struct mhu_device_ctx *device_ctx = &mhu_ctx.device_ctx_table[elt_idx];
    struct mhu_smt_channel *smt_channel;

    device_ctx->config = (struct mod_vhost_mhu_channel_config*)data;

    device_ctx->smt_channel_table = fwk_mm_calloc(slot_count,
                                                  sizeof(device_ctx->smt_channel_table[0]));

    device_ctx->slot_count = slot_count;

    for (slot = 0; slot < device_ctx->slot_count; slot++) {
        smt_channel = &device_ctx->smt_channel_table[slot];

        smt_channel->id.value = 0;
    }

    device_ctx->socket = NULL;

    return FWK_SUCCESS;
}

static int vhost_bind(fwk_id_t id, unsigned int round)
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

static int vhost_process_bind_request(fwk_id_t source_id,
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
    *api = &vhost_mod_smt_driver_api;

    return FWK_SUCCESS;
}

static int vhost_start(fwk_id_t id)
{
    size_t elt_idx;
    struct mhu_device_ctx *device_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    elt_idx = fwk_id_get_element_idx(id);
    device_ctx = &mhu_ctx.device_ctx_table[elt_idx];

    device_ctx->thread = g_thread_new(device_ctx->config->socket_path, server_wait_vm_thread, device_ctx);

    return FWK_SUCCESS;
}

static int vhost_stop(fwk_id_t id)
{
    size_t elt_idx;
    struct mhu_device_ctx *device_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    elt_idx = fwk_id_get_element_idx(id);
    device_ctx = &mhu_ctx.device_ctx_table[elt_idx];

    vscmi_destroy_channels(device_ctx);

    return FWK_SUCCESS;
}

/* OPTEE_MHU module definition */
const struct fwk_module module_vhost_mhu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = vhost_init,
    .element_init = vhost_device_init,
    .bind = vhost_bind,
    .start = vhost_start,
    .stop = vhost_stop,
    .process_bind_request = vhost_process_bind_request,
};
