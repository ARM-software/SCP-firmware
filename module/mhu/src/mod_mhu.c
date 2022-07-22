/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) Device Driver.
 */

#include <internal/mhu.h>

#include <mod_mhu.h>
#include <mod_transport.h>

#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

/*
 * Maximum number of slots per MHU device. The maximum number of slots is 31 and
 * not 32 because bit [31] in the MHU STAT register is reserved in secure MHUs
 * for indicating that a non-secure access attempt occurred. This reservation
 * also applies to non-secure MHUs for consistency, though the bit is unused.
 */
#define MHU_SLOT_COUNT_MAX 31

struct mhu_transport_channel {
    fwk_id_t id;
    struct mod_transport_driver_input_api *api;
};

/* MHU device context */
struct mhu_device_ctx {
    /* Pointer to the device configuration */
    const struct mod_mhu_device_config *config;

    /* Number of slots (represented by sub-elements) */
    unsigned int slot_count;

    /* Mask of slots that are bound to a TRANSPORT/SMT channel */
    uint32_t bound_slots;

    /* Table of TRANSPORT channels bound to the device */
    struct mhu_transport_channel *transport_channel_table;
};

/* MHU context */
struct mod_mhu_ctx {
    /* Table of device contexts */
    struct mhu_device_ctx *device_ctx_table;

    /* Number of devices in the device context table*/
    unsigned int device_count;
};

static struct mod_mhu_ctx mhu_ctx;

static void mhu_isr(void)
{
    int status;
    unsigned int interrupt;
    unsigned int device_idx;
    struct mhu_device_ctx *device_ctx;
    struct mhu_reg *reg;
    unsigned int slot;
    struct mhu_transport_channel *transport_channel;

    status = fwk_interrupt_get_current(&interrupt);
    if (status != FWK_SUCCESS) {
        return;
    }

    for (device_idx = 0; device_idx < mhu_ctx.device_count; device_idx++) {
        device_ctx = &mhu_ctx.device_ctx_table[device_idx];
        if (device_ctx->config->irq == interrupt) {
            break;
        }
    }

    if (device_idx >= mhu_ctx.device_count) {
        return;
    }

    reg = (struct mhu_reg *)device_ctx->config->in;

    /* Loop over all the slots */
    while (reg->STAT != 0) {
        slot = (unsigned int)__builtin_ctz(reg->STAT);

        /*
         * If the slot is bound to an TRANSPORT channel, signal the message
         * to the TRANSPORT channel.
         */
        if ((device_ctx->bound_slots & (uint32_t)(1U << slot)) != (uint32_t)0) {
            transport_channel = &device_ctx->transport_channel_table[slot];
            status =
                transport_channel->api->signal_message(transport_channel->id);
            if (status != FWK_SUCCESS) {
                FWK_LOG_DEBUG("[MHU] %s @%d", __func__, __LINE__);
            }
        }

        /* Acknowledge the interrupt */
        reg->CLEAR = 1U << slot;
    }
}

/*
 * TRANSPORT module driver API
 */

static int raise_interrupt(fwk_id_t slot_id)
{
    struct mhu_device_ctx *device_ctx;
    unsigned int slot;
    struct mhu_reg *reg;


    device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(slot_id)];
    slot = fwk_id_get_sub_element_idx(slot_id);
    reg = (struct mhu_reg *)device_ctx->config->out;

    reg->SET |= (1U << slot);

    return FWK_SUCCESS;
}

const struct mod_transport_driver_api mhu_mod_transport_driver_api = {
    .trigger_event = raise_interrupt,
};

/*
 * Framework handlers
 */

static int mhu_init(fwk_id_t module_id, unsigned int device_count,
                    const void *unused)
{
    if (device_count == 0) {
        return FWK_E_PARAM;
    }

    mhu_ctx.device_ctx_table = fwk_mm_calloc(device_count,
        sizeof(mhu_ctx.device_ctx_table[0]));

    mhu_ctx.device_count = device_count;

    return FWK_SUCCESS;
}

static int mhu_device_init(fwk_id_t device_id, unsigned int slot_count,
                           const void *data)
{
    struct mod_mhu_device_config *config = (struct mod_mhu_device_config *)data;
    struct mhu_device_ctx *device_ctx;

    if ((config->in == 0) || (config->out == 0)) {
        return FWK_E_PARAM;
    }

    device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    device_ctx->transport_channel_table = fwk_mm_calloc(
        slot_count, sizeof(device_ctx->transport_channel_table[0]));

    device_ctx->config = config;
    device_ctx->slot_count = slot_count;

    return FWK_SUCCESS;
}

static int mhu_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mhu_device_ctx *device_ctx;
    unsigned int slot;
    struct mhu_transport_channel *transport_channel;

    if ((round == 1U) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

        for (slot = 0; slot < MHU_SLOT_COUNT_MAX; slot++) {
            if ((device_ctx->bound_slots & (uint32_t)(1U << slot)) ==
                (uint32_t)0) {
                continue;
            }

            transport_channel = &device_ctx->transport_channel_table[slot];

            status = fwk_module_bind(
                transport_channel->id,
                FWK_ID_API(
                    FWK_MODULE_IDX_TRANSPORT,
                    MOD_TRANSPORT_API_IDX_DRIVER_INPUT),
                &transport_channel->api);
            if (status != FWK_SUCCESS) {
                return status;
            }
        }
    }

    return FWK_SUCCESS;
}

static int mhu_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                    fwk_id_t api_id, const void **api)
{
    struct mhu_device_ctx *device_ctx;
    unsigned int slot;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_SUB_ELEMENT)) {
        return FWK_E_ACCESS;
    }

    device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(target_id)];
    slot = fwk_id_get_sub_element_idx(target_id);

    if (device_ctx->bound_slots & (1U << slot)) {
        return FWK_E_ACCESS;
    }

    device_ctx->transport_channel_table[slot].id = source_id;
    device_ctx->bound_slots |= 1U << slot;

    *api = &mhu_mod_transport_driver_api;

    return FWK_SUCCESS;
}

static int mhu_start(fwk_id_t id)
{
    int status;
    struct mhu_device_ctx *device_ctx;

    if (fwk_id_get_type(id) == FWK_ID_TYPE_MODULE) {
        return FWK_SUCCESS;
    }

    device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

    if (device_ctx->bound_slots != 0) {
        status = fwk_interrupt_set_isr(device_ctx->config->irq, &mhu_isr);
        if (status != FWK_SUCCESS) {
            return status;
        }
        status = fwk_interrupt_enable(device_ctx->config->irq);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

/* MHU module definition */
const struct fwk_module module_mhu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = mhu_init,
    .element_init = mhu_device_init,
    .bind = mhu_bind,
    .start = mhu_start,
    .process_bind_request = mhu_process_bind_request,
};
