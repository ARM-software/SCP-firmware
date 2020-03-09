/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) v2 Device Driver.
 */

#include <mhu2.h>

#include <mod_mhu2.h>
#include <mod_smt.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MHU_SLOT_COUNT_MAX 32

struct mhu2_smt_channel {
    fwk_id_t id;
    const struct mod_smt_driver_input_api *api;
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

    /* Mask of slots that are bound to an SMT channel */
    uint32_t bound_slots;

    /* Table of SMT channels bound to the channel */
    struct mhu2_smt_channel *smt_channel_table;
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
    unsigned int slot;
    struct mhu2_smt_channel *smt_channel;

    assert(channel_ctx != NULL);

    while (channel_ctx->recv_channel->STAT != 0) {
        slot = __builtin_ctz(channel_ctx->recv_channel->STAT);

        /*
         * If the slot is bound to an SMT channel, signal the message to the
         * SMT channel.
         */
        if (channel_ctx->bound_slots & (1 << slot)) {
            smt_channel = &channel_ctx->smt_channel_table[slot];
            smt_channel->api->signal_message(smt_channel->id);
        }

        /* Acknowledge the interrupt */
        channel_ctx->recv_channel->STAT_CLEAR = 1 << slot;
    }
}

/*
 * SMT module driver API
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

static const struct mod_smt_driver_api mhu2_mod_smt_driver_api = {
    .raise_interrupt = raise_interrupt,
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
        assert(false);
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
        assert(false);
        return FWK_E_DATA;
    }

    channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];
    channel_ctx->send = (struct mhu2_send_reg *)config->send;

    if (config->channel >= channel_ctx->send->MSG_NO_CAP) {
        assert(false);
        return FWK_E_DATA;
    }

    channel_ctx->config = config;
    channel_ctx->slot_count = slot_count;
    channel_ctx->send_channel = &channel_ctx->send->channel[config->channel];
    recv_reg = (struct mhu2_recv_reg *)config->recv;
    channel_ctx->recv_channel = &recv_reg->channel[config->channel];

    channel_ctx->smt_channel_table =
        fwk_mm_calloc(slot_count, sizeof(channel_ctx->smt_channel_table[0]));

    return FWK_SUCCESS;
}

static int mhu2_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mhu2_channel_ctx *channel_ctx;
    unsigned int slot;
    struct mhu2_smt_channel *smt_channel;

    if ((round == 1) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(id)];

        for (slot = 0; slot < MHU_SLOT_COUNT_MAX; slot++) {
            if (!(channel_ctx->bound_slots & (UINT32_C(1) << slot)))
                continue;

            smt_channel = &channel_ctx->smt_channel_table[slot];

            status = fwk_module_bind(smt_channel->id,
                                     FWK_ID_API(FWK_MODULE_IDX_SMT,
                                                MOD_SMT_API_IDX_DRIVER_INPUT),
                                     &smt_channel->api);
            if (status != FWK_SUCCESS) {
                /* Unable to bind back to SMT channel */
                assert(false);
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

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_SUB_ELEMENT)) {
        /*
         * Something tried to bind to the module or an element. Only binding to
         * a slot (sub-element) is allowed.
         */
        assert(false);
        return FWK_E_ACCESS;
    }

    channel_ctx = &ctx.channel_ctx_table[fwk_id_get_element_idx(target_id)];
    slot = fwk_id_get_sub_element_idx(target_id);

    if (channel_ctx->bound_slots & (1 << slot)) {
        /* Something tried to bind to a slot that has already been bound to */
        assert(false);
        return FWK_E_ACCESS;
    }

    channel_ctx->smt_channel_table[slot].id = source_id;
    channel_ctx->bound_slots |= 1 << slot;

    *api = &mhu2_mod_smt_driver_api;

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
            assert(false);
            return status;
        }
        status = fwk_interrupt_enable(channel_ctx->config->irq);
        if (status != FWK_SUCCESS) {
            /* Failed to enable isr */
            assert(false);
            return status;
        }
    }

    return FWK_SUCCESS;
}

/* MHU v2 module definition */
const struct fwk_module module_mhu2 = {
    .name = "MHU2",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_MHU2_API_IDX_COUNT,
    .init = mhu2_init,
    .element_init = mhu2_channel_init,
    .bind = mhu2_bind,
    .start = mhu2_start,
    .process_bind_request = mhu2_process_bind_request,
};
