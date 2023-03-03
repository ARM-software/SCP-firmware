/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) v2 Device Driver.
 */

#include <mhu2.h>

#include <mod_mhu2.h>
#include <mod_transport.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MHU_SLOT_COUNT_MAX 32

#if defined(BUILD_HAS_INBAND_MSG_SUPPORT)
/* MHUv2 channel status register size in bytes */
#    define MHU_CHANNEL_STATUS_REGISTER_WIDTH 4

/*
 * For, transferring in-band messages, we require at least 9 MHUv2 channels
 * excluding the payload. Out of 9 channels, one is used for doorbell and 8
 * channels are used for transferring the in-band message in the same layout
 * as a shared memory which includes implementation defined reserved field,
 * channel status, channel flags, length, etc.
 *
 */
const uint8_t min_channels_required = 9;
#endif

struct mhu2_bound_channel {
    fwk_id_t id;
    const struct mod_transport_driver_input_api *driver_input_api;
};

/* MHU channel context */
struct mhu2_channel_ctx {
    /* Pointer to the channel configuration */
    const struct mod_mhu2_channel_config *config;

    /* Pointer to send register set */
    struct mhu2_send_reg *send;

    /* Pointers to channel-specific register sets */
    struct mhu2_send_channel_reg *send_channel;
    struct mhu2_recv_channel_reg *recv_channel;

    /* Number of slots (represented by sub-elements) */
    unsigned int slot_count;

    /* Mask of slots that are bound to an TRANSPORT channel */
    uint32_t bound_slots;

    /* Table of channels bound to the channel */
    struct mhu2_bound_channel *bound_channels_table;
};

/* MHU v2 context */
static struct mhu2_ctx {
    /* Table of channel contexts */
    struct mhu2_channel_ctx *channel_ctx_table;

    /* Number of channels in the channel context table*/
    unsigned int channel_count;
} ctx;

static void mhu2_isr(uintptr_t ctx_param)
{
    struct mhu2_channel_ctx *channel_ctx = (struct mhu2_channel_ctx *)ctx_param;
    struct mhu2_bound_channel *bound_channel;
    unsigned int slot;

    fwk_assert(channel_ctx != NULL);

    while (channel_ctx->recv_channel->STAT != 0) {
        slot = __builtin_ctz(channel_ctx->recv_channel->STAT);
        /*
         * If the slot is bound to a transport channel,
         * signal the message to the corresponding module
         */
        if (channel_ctx->bound_slots & (1 << slot)) {
            bound_channel = &channel_ctx->bound_channels_table[slot];

            bound_channel->driver_input_api->signal_message(bound_channel->id);
        }
        /* Acknowledge the interrupt */
        channel_ctx->recv_channel->STAT_CLEAR = 1 << slot;
    }
}

/*
 * TRANSPORT module driver API
 */

static int raise_interrupt(fwk_id_t slot_id)
{
    unsigned int slot;
    struct mhu2_channel_ctx *channel_ctx;
    struct mhu2_send_reg *send;

    channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(slot_id)];
    slot = fwk_id_get_sub_element_idx(slot_id);
    send = channel_ctx->send;

    /* Turn on receiver */
    send->ACCESS_REQUEST = 1;
    while (send->ACCESS_READY != 1)
        continue;

    channel_ctx->send_channel->STAT_SET |= (1 << slot);

    /* Signal that the receiver is no longer needed */
    send->ACCESS_REQUEST = 0;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
/*
 * transport module driver API
 */
static int mhu2_send_message(
    struct mod_transport_buffer *message,
    fwk_id_t slot_id)
{
    struct mhu2_channel_ctx *channel_ctx;
    struct mhu2_send_reg *send;
    uint8_t channel_count;
    size_t payload_size;
    uint8_t db_ch = 0, ch = 0;
    uint8_t channels_used_for_payload = 0;
    uint32_t *msg_ptr;

    channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(slot_id)];

    /* Get pointer to the MHUv2 send register */
    send = channel_ctx->send;

    /* Get pointer to the message to be sent */
    msg_ptr = (uint32_t *)message;

    /* Calculate the size of the payload */
    payload_size = message->length - sizeof(message->message_header);

    /* Calculate the number of channels required for payload */
    if (payload_size) {
        channels_used_for_payload =
            payload_size / MHU_CHANNEL_STATUS_REGISTER_WIDTH;
    }

    /* Read the number of channels implemented */
    channel_count = send->MSG_NO_CAP;

    /* Check if minimum number of MHUv2 channels are available */
    if (channel_count < (min_channels_required + channels_used_for_payload)) {
        FWK_LOG_INFO(
            "[MHUv2] ERROR! Message length exceeds the number of MHUv2"
            "channels available");
        return FWK_E_SUPPORT;
    }

    /* Turn on receiver */
    send->ACCESS_REQUEST = 1;
    while (send->ACCESS_READY != 1)
        continue;

    /* Get the channel used for doorbell */
    db_ch = channel_ctx->config->channel;

    /* Copy the in-band message to the message status registers */
    for (uint8_t idx = 0; ch < (min_channels_required - 1); ch++) {
        /* Skip the doorbell channel */
        if (ch == db_ch)
            continue;

        send->channel[ch].STAT_SET = msg_ptr[idx];
        idx++;
    }

    /*
     * If we have payload to be sent, copy the payload to message status
     * registers.
     */
    if (payload_size != 0) {
        for (uint8_t payload_idx = 0;
             payload_idx < (payload_size / MHU_CHANNEL_STATUS_REGISTER_WIDTH);
             payload_idx++) {
            /* Skip the doorbell channel */
            if ((ch + payload_idx) == db_ch)
                continue;

            send->channel[payload_idx + ch].STAT_SET =
                message->payload[payload_idx];
        }
    }

    /* Receiver no longer required */
    send->ACCESS_REQUEST = 0;

    return FWK_SUCCESS;
}

static int mhu2_get_message(
    struct mod_transport_buffer *message,
    fwk_id_t slot_id)
{
    struct mhu2_channel_ctx *channel_ctx;
    struct mhu2_recv_reg *recv;
    size_t payload_size;
    uint8_t db_ch = 0, ch = 0;
    uint32_t *msg_ptr;

    channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(slot_id)];

    /* Get the channel used for doorbell */
    db_ch = channel_ctx->config->channel;

    /* Get a pointer to buffer */
    msg_ptr = (uint32_t *)message;

    /* Get a pointer to MHUv2 receive register */
    recv = (struct mhu2_recv_reg *)channel_ctx->config->recv;

    /* Copy the in-band message from message status registers */
    for (uint8_t idx = 0; ch < (min_channels_required - 1); ch++) {
        /* Skip doorbell channel */
        if (ch == db_ch)
            continue;

        msg_ptr[idx++] = recv->channel[ch].STAT;

        /* Clear the message status register */
        recv->channel[ch].STAT_CLEAR = 0xffffffff;
    }

    /* Calculate size of the received payload */
    payload_size = message->length - sizeof(message->message_header);

    /* If payload is present, copy it from the message status registers */
    if (payload_size != 0) {
        for (uint8_t payload_idx = 0;
             payload_idx < (payload_size / MHU_CHANNEL_STATUS_REGISTER_WIDTH);
             payload_idx++) {
            /* Skip doorbell channel */
            if ((ch + payload_idx) == db_ch)
                continue;

            message->payload[payload_idx] =
                recv->channel[payload_idx + ch].STAT;

            /* Clear the message status register */
            recv->channel[payload_idx + ch].STAT_CLEAR = 0xffffffff;
        }
    }

    return FWK_SUCCESS;
}
#endif

struct mod_transport_driver_api mhu2_mod_transport_driver_api = {
#    ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    .send_message = mhu2_send_message,
    .get_message = mhu2_get_message,
#    endif
    .trigger_event = raise_interrupt,
};

/*
 * Framework handlers
 */

static int mhu2_init(fwk_id_t module_id,
                     unsigned int channel_count,
                     const void *unused)
{
    if (channel_count == 0) {
        /* There must be at least 1 mhu channel */
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    ctx.channel_ctx_table = fwk_mm_calloc(channel_count,
        sizeof(ctx.channel_ctx_table[0]));

    ctx.channel_count = channel_count;

    return FWK_SUCCESS;
}

static int mhu2_channel_init(fwk_id_t channel_id,
                             unsigned int slot_count,
                             const void *data)
{
    const struct mod_mhu2_channel_config *config = data;
    struct mhu2_channel_ctx *channel_ctx;
    struct mhu2_recv_reg *recv_reg;

    if ((config == NULL) || (config->recv == 0) || (config->send == 0)) {
        fwk_unexpected();
        return FWK_E_DATA;
    }

    channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];
    channel_ctx->send = (struct mhu2_send_reg *)config->send;

    if (config->channel >= channel_ctx->send->MSG_NO_CAP) {
        fwk_unexpected();
        return FWK_E_DATA;
    }

    channel_ctx->config = config;
    channel_ctx->slot_count = slot_count;
    channel_ctx->send_channel = &channel_ctx->send->channel[config->channel];
    recv_reg = (struct mhu2_recv_reg *)config->recv;

#if defined(BUILD_HAS_INBAND_MSG_SUPPORT)
    /*
     * Mask the channels used for transferring in-band messages.
     * Only the doorbell channel should be used to raise interrupt.
     */
    for (uint8_t ch = 0; ch < recv_reg->MSG_NO_CAP; ch++) {
        /* Skip the channel used for doorbell */
        if (ch == config->channel)
            continue;

        /*
         * Set channel mask.
         *
         * Masked channels don't generate interrupt when data is written
         * to them.
         */
        recv_reg->channel[ch].MASK_SET = 0xffffffff;
    }
#endif
    channel_ctx->recv_channel = &recv_reg->channel[config->channel];

    channel_ctx->bound_channels_table =
        fwk_mm_calloc(slot_count, sizeof(channel_ctx->bound_channels_table[0]));

    return FWK_SUCCESS;
}

static int mhu2_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mhu2_channel_ctx *channel_ctx;
    unsigned int slot;

    if ((round == 1) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(id)];

        for (slot = 0; slot < MHU_SLOT_COUNT_MAX; slot++) {
            struct mhu2_bound_channel *bound_channel;
            if (!(channel_ctx->bound_slots & (UINT32_C(1) << slot)))
                continue;

            bound_channel = &channel_ctx->bound_channels_table[slot];

            status = fwk_module_bind(
                bound_channel->id,
                FWK_ID_API(
                    FWK_MODULE_IDX_TRANSPORT,
                    MOD_TRANSPORT_API_IDX_DRIVER_INPUT),
                &bound_channel->driver_input_api);

            if (status != FWK_SUCCESS) {
                /* Unable to bind back to TRANSPORT channel */
                fwk_unexpected();
                return status;
            }
        }
    }

    return FWK_SUCCESS;
}

static int mhu2_process_bind_request(fwk_id_t source_id,
                                    fwk_id_t target_id,
                                    fwk_id_t api_id,
                                    const void **api)
{
    struct mhu2_channel_ctx *channel_ctx;
    unsigned int slot;
    enum mod_mhu2_api_idx api_id_type;
    api_id_type = (enum mod_mhu2_api_idx)fwk_id_get_api_idx(api_id);

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_SUB_ELEMENT)) {
        /*
         * Something tried to bind to the module or an element. Only binding to
         * a slot (sub-element) is allowed.
         */
        fwk_unexpected();
        return FWK_E_ACCESS;
    }

    channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(target_id)];
    slot = fwk_id_get_sub_element_idx(target_id);
    if (channel_ctx->bound_slots & (1 << slot)) {
        /* Something tried to bind to a slot that has already been bound to */
        fwk_unexpected();
        return FWK_E_ACCESS;
    }

    channel_ctx->bound_channels_table[slot].id = source_id;
    channel_ctx->bound_slots |= 1 << slot;

    if (api_id_type != MOD_MHU2_API_IDX_TRANSPORT_DRIVER) {
        /* Invalid config */
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    *api = &mhu2_mod_transport_driver_api;

    return FWK_SUCCESS;
}

static int mhu2_start(fwk_id_t id)
{
    int status;
    struct mhu2_channel_ctx *channel_ctx;

    if (fwk_id_get_type(id) == FWK_ID_TYPE_MODULE)
        return FWK_SUCCESS;

    channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(id)];

    if (channel_ctx->bound_slots != 0) {
        status = fwk_interrupt_set_isr_param(channel_ctx->config->irq,
                                             &mhu2_isr,
                                             (uintptr_t)channel_ctx);
        if (status != FWK_SUCCESS) {
            /* Failed to set isr */
            fwk_unexpected();
            return status;
        }
        status = fwk_interrupt_enable(channel_ctx->config->irq);
        if (status != FWK_SUCCESS) {
            /* Failed to enable isr */
            fwk_unexpected();
            return status;
        }
    }

    return FWK_SUCCESS;
}

/* MHU v2 module definition */
const struct fwk_module module_mhu2 = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_MHU2_API_IDX_COUNT,
    .init = mhu2_init,
    .element_init = mhu2_channel_init,
    .bind = mhu2_bind,
    .start = mhu2_start,
    .process_bind_request = mhu2_process_bind_request,
};
