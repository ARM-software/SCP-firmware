/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) Device Driver.
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
#include <mod_input_mhu.h>
#include <mod_msg_smt.h>
#include <scmi_agents.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

struct __attribute((packed)) mod_virtio_smt_memory {
    uint32_t message_header;
    uint32_t payload[];
};

struct virt_msg {
    char out_buf[128];
    int out_len;
    char in_buf[128];
    int in_len;
};

struct mhu_smt_channel {
    fwk_id_t id;
    struct mod_msg_smt_driver_input_api *api;

    /* input message */
    struct virt_msg *msg;
};

/* MHU device context */
struct mhu_device_ctx {
    /* Channel configuration data */
    struct mod_input_mhu_channel_config *config;

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
 * Input host helpers
 */

static int kbd_simple_msg(int proto, int msg, int token, struct mod_virtio_smt_memory *memory)
{

    memory->message_header = token << 18 | proto << 10 | msg;
    return sizeof(memory->message_header);

}

static int kbd_simple_one(int proto, int msg, int token, int param0, struct mod_virtio_smt_memory *memory)
{
    int length;

    memory->message_header = token << 18 | proto << 10 | msg;
    length = sizeof(memory->message_header);
    memory->payload[0] = param0;
    length += sizeof(memory->payload[0]);

    return length;

}

static int kbd_simple_two(int proto, int msg, int token, int param0, int param1, struct mod_virtio_smt_memory *memory)
{
    int length;

    memory->message_header = token << 18 | proto << 10 | msg;
    length = sizeof(memory->message_header);
    memory->payload[0] = param0;
    length += sizeof(memory->payload[0]);
    memory->payload[1] = param1;
    length += sizeof(memory->payload[1]);

    return length;
}

static int kbd_sensor_trip(int proto, int msg, int token, int param0, int param1, struct mod_virtio_smt_memory *memory)
{
    int length;

    memory->message_header = token << 18 | proto << 10 | msg;
    length = sizeof(memory->message_header);
    memory->payload[0] = param0;
    length += sizeof(memory->payload[0]);
    memory->payload[1] = 0x0 << 4 | 0x3;
    length += sizeof(memory->payload[1]);
    memory->payload[2] = (uint32_t)param1;
    length += sizeof(memory->payload[2]);
    memory->payload[3] = 0;
    length += sizeof(memory->payload[3]);

    return length;
}

static int kbd_set_message(char *str, struct mod_virtio_smt_memory *memory)
{
    int i;
    unsigned int level;

    if (!strcmp(str,"version\n")) {
        return kbd_simple_msg(0x10, 0x0, 0xAA, memory);
    } else    if (!strcmp(str,"base attributes\n")) {
        return kbd_simple_msg(0x10, 0x1, 0xAA, memory);
    } else    if (!strcmp(str,"vendor\n")) {
        return kbd_simple_msg(0x10, 0x3, 0xAA, memory);
    } else    if (!strcmp(str,"sub vendor\n")) {
        return kbd_simple_msg(0x10, 0x4, 0xAA, memory);
    } else    if (!strcmp(str,"implementation\n")) {
        return kbd_simple_msg(0x10, 0x5, 0xAA, memory);
    } else    if (!strcmp(str,"list\n")) {
        return kbd_simple_one(0x10, 0x6, 0xAA, 0, memory);
    } else    if (!strcmp(str,"agent\n")) {
        return kbd_simple_one(0x10, 0x6, 0xAA, 1, memory);
    } else if (!strcmp(str,"clock version\n")) {
        return kbd_simple_msg(0x14, 0x0, 0xCC, memory);
    } else    if (!strcmp(str,"clock attributes\n")) {
        return kbd_simple_msg(0x14, 0x1, 0xCC, memory);
    } else    if (!strncmp(str,"clock attribute", 15 )) {
        sscanf(str, "clock attribute %d", &i);
        return kbd_simple_one(0x14, 0x3, 0xCC, i, memory);
    } else    if (!strncmp(str,"clock enable", 12 )) {
        sscanf(str, "clock enable %d", &i);
        return kbd_simple_two(0x14, 0x7, 0xcc, i, 1, memory);
    } else    if (!strncmp(str,"clock disable", 12 )) {
        sscanf(str, "clock disable %d", &i);
        return kbd_simple_two(0x14, 0x7, 0xcc, i, 0, memory);
    } else if (!strcmp(str,"power version\n")) {
        return kbd_simple_msg(0x11, 0x0, 0xBB, memory);
    } else    if (!strcmp(str,"power attributes\n")) {
        return kbd_simple_msg(0x11, 0x1, 0xBB, memory);
    } else    if (!strncmp(str,"power attribute", 15 )) {
        sscanf(str, "power attribute %d", &i);
        return kbd_simple_one(0x11, 0x3, 0xBB, i, memory);
        return 0;
    } else    if (!strncmp(str,"power enable", 12 )) {
        sscanf(str, "power enable %d", &i);
        return kbd_simple_two(0x11, 0x4, 0xBB, i, 1, memory);
    } else    if (!strncmp(str,"power disable", 12 )) {
        sscanf(str, "power disable %d", &i);
        return kbd_simple_two(0x11, 0x4, 0xBB, i, 0, memory);
    } else    if (!strncmp(str,"power notify", 12 )) {
        sscanf(str, "power notify %d", &i);
        return kbd_simple_two(0x11, 0x6, 0xBB, i, 1, memory);
    } else if (!strcmp(str,"perf version\n")) {
        return kbd_simple_msg(0x13, 0x0, 0xBB, memory);
    } else    if (!strcmp(str,"perf attributes\n")) {
        return kbd_simple_msg(0x13, 0x1, 0xBB, memory);
    } else    if (!strncmp(str,"perf msg attributes", 19 )) {
        sscanf(str, "perf msg attributes %d", &i);
        return kbd_simple_one(0x13, 0x2, 0xBB, i, memory);
    } else    if (!strncmp(str,"perf domain attributes", 22 )) {
        sscanf(str, "perf domain attributes %d", &i);
        return kbd_simple_one(0x13, 0x3, 0xBB, i, memory);
    } else    if (!strncmp(str,"perf describe levels", 20 )) {
        sscanf(str, "perf describe levels %d %u", &i, &level);
        return kbd_simple_two(0x13, 0x4, 0xBB, i, level, memory);
    } else    if (!strncmp(str,"perf set", 8 )) {
        sscanf(str, "perf set %d %u", &i, &level);
        return kbd_simple_two(0x13, 0x7, 0xBB, i, level, memory);
    } else    if (!strncmp(str,"perf get", 8 )) {
        sscanf(str, "perf get %d", &i);
        return kbd_simple_one(0x13, 0x8, 0xBB, i, memory);
    } else    if (!strncmp(str,"perf notify level", 17 )) {
        sscanf(str, "perf notify level %d", &i);
        return kbd_simple_two(0x11, 0xA, 0xBB, i, 1, memory);
    } else if (!strcmp(str,"sensor version\n")) {
        return kbd_simple_msg(0x15, 0x0, 0xFF, memory);
    } else    if (!strcmp(str,"sensor attributes\n")) {
        return kbd_simple_msg(0x15, 0x1, 0xFF, memory);
    } else    if (!strncmp(str,"sensor msg attributes", 21 )) {
        sscanf(str, "sensor msg attributes %d", &i);
        return kbd_simple_one(0x15, 0x2, 0xFF, i, memory);
    } else    if (!strncmp(str,"sensor description", 18 )) {
        sscanf(str, "sensor description %d", &i);
        return kbd_simple_one(0x15, 0x3, 0xFF, i, memory);
    } else    if (!strncmp(str,"sensor get", 10 )) {
        sscanf(str, "sensor get %d", &i);
        return kbd_simple_two(0x15, 0x6, 0xFF, i, 0, memory);
    } else    if (!strncmp(str,"sensor notify", 13 )) {
        sscanf(str, "sensor notify %d", &i);
        return kbd_simple_two(0x15, 0x4, 0xFF, i, 1, memory);
    } else    if (!strncmp(str,"sensor trip", 11 )) {
        sscanf(str, "sensor trip %d %u", &i, &level);
        return kbd_sensor_trip(0x15, 0x5, 0xFF, i, level, memory);
    }

    return -1;
}

void input_signal_message(struct mhu_device_ctx *device_ctx, unsigned int channel_idx, struct virt_msg *msg )
{
    struct mhu_smt_channel *smt_channel;

    smt_channel = &device_ctx->smt_channel_table[channel_idx];
    smt_channel->msg = msg;

    smt_channel->api->signal_message(smt_channel->id,
                                     msg->out_buf,
                                     msg->out_len,
                                     msg->in_buf,
                                     msg->in_len);

    fwk_process_event_queue();

    fwk_log_flush();
}

static gboolean input_handle_request( GIOChannel *channel, GIOCondition cond, gpointer data)
{
    gchar *str_return;
    gsize length, terminator_pos;
    GError *error = NULL;
    struct virt_msg msg;

    if (g_io_channel_read_line( channel, &str_return, &length, &terminator_pos, &error) == G_IO_STATUS_ERROR)
        g_warning("Something went wrong");

    if (length == 0)
	return TRUE;

    if (error != NULL) {
        g_warning("Something went wrong");
        exit(1);
    }

    msg.out_len = kbd_set_message(str_return, (struct mod_virtio_smt_memory *) &msg.out_buf);
    msg.in_len = 128;

    if (msg.out_len > 0)
        /* Input keyboard emulates 1st channel request */
        input_signal_message(&mhu_ctx.device_ctx_table[0], 0, &msg);

    g_free( str_return );
    return TRUE;
}

static int input_init_channels(struct mhu_device_ctx *device_ctx)
{
    GError *error = NULL;
    GIOChannel *channel = g_io_channel_unix_new( STDIN_FILENO );
    g_io_add_watch( channel, G_IO_IN, input_handle_request, NULL );
    g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, &error);
    if (error != NULL) {
        g_warning("Something went wrong");
        exit(1);
    }
    return FWK_SUCCESS;
}

/*
 * Mailbox module driver API
 */

/*
 * Provide a notify interface to the Mailbox driver
 */
static int raise_queue_notify(fwk_id_t channel_id, size_t size)
{
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

    if (device_count == 0)
        return FWK_E_PARAM;

    mhu_ctx.device_ctx_table = fwk_mm_calloc(device_count,
                                             sizeof(*mhu_ctx.device_ctx_table));
    if (mhu_ctx.device_ctx_table == NULL)
        return FWK_E_NOMEM;

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

    device_ctx->config = (struct mod_input_mhu_channel_config*)data;

    device_ctx->smt_channel_table = fwk_mm_calloc(slot_count,
                                                  sizeof(device_ctx->smt_channel_table[0]));

    device_ctx->slot_count = slot_count;

    for (slot = 0; slot < device_ctx->slot_count; slot++) {
            smt_channel = &device_ctx->smt_channel_table[slot];

            smt_channel->id.value = 0;
    }

    /* TBF: Use PENDING to request the creation of a context */
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

            if (smt_channel->id.value == 0)
                continue;

            status = fwk_module_bind(smt_channel->id,
                device_ctx->config->driver_api_id,
                &smt_channel->api);

            if (status != FWK_SUCCESS)
                return status;
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

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_SUB_ELEMENT))
        return FWK_E_ACCESS;

    if (fwk_id_get_api_idx(api_id) != 0)
        return FWK_E_PARAM;

    elt_idx = fwk_id_get_element_idx(target_id);
    if (elt_idx >= mhu_ctx.device_count)
        return FWK_E_PARAM;

    device_ctx = &mhu_ctx.device_ctx_table[elt_idx];

    slot = fwk_id_get_sub_element_idx(target_id);
    if (slot >= device_ctx->slot_count)
        return FWK_E_PARAM;

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

    input_init_channels(device_ctx);

    return FWK_SUCCESS;
}

/* OPTEE_MHU module definition */
const struct fwk_module module_input_mhu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = vhost_init,
    .element_init = vhost_device_init,
    .bind = vhost_bind,
    .start = vhost_start,
    .process_bind_request = vhost_process_bind_request,
};
