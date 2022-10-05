/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rcar_mfismh.h"

#include <rcar_mmap.h>

#include <mod_transport.h>

#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

/*
 * Maximum number of slots per MFISMH device. The maximum number of slots is 31
 * and not 32 because bit [31] in the MFISMH STAT register is reserved in secure
 * MFISMHs for indicating that a non-secure access attempt occurred.
 * This reservation also applies to non-secure MFISMHs for consistency,
 * though the bit is unused.
 */
#define MFISMH_SLOT_COUNT_MAX 31

struct mfismh_reg *mfis_regs;

struct mfismh_transport_channel {
    fwk_id_t id;
    struct mod_transport_driver_input_api *api;
};

/* MFISMH device context */
struct mfismh_device_ctx {
    /* Pointer to the device configuration */
    const struct mod_rcar_mfismh_device_config *config;

    /* Number of slots (represented by sub-elements) */
    unsigned int slot_count;

    /* Mask of slots that are bound to an TRANSPORT channel */
    uint32_t bound_slots;

    /* Table of TRANSPORT channels bound to the device */
    struct mfismh_transport_channel *transport_channel_table;
};

/* MFISMH context */
struct mfismh_ctx {
    /* Table of device contexts */
    struct mfismh_device_ctx *device_ctx_table;

    /* Number of devices in the device context table*/
    unsigned int device_count;
};

static struct mfismh_ctx mfismh_ctx;

static void mfismh_isr(void)
{
    int status;
    unsigned int interrupt;
    unsigned int device_idx;
    struct mfismh_device_ctx *device_ctx;
    unsigned int slot;
    struct mfismh_reg *reg;
    struct mfismh_transport_channel *transport_channel;

    status = fwk_interrupt_get_current(&interrupt);
    if (status != FWK_SUCCESS)
        return;

    if (!IS_MFIS_IRQ(interrupt))
        return;

    for (device_idx = 0; device_idx < mfismh_ctx.device_count; device_idx++) {
        device_ctx = &mfismh_ctx.device_ctx_table[device_idx];
        if (device_ctx->config->irq == interrupt)
            break;
    }

    if (device_idx >= mfismh_ctx.device_count)
        return;

    reg = (struct mfismh_reg *)&mfis_regs[MFIS_IRQ2NO(interrupt)];

    /* Acknowledge the interrupt */
    reg->CCR.eir = 0;

    /* Loop over all the slots */ /* For prototyping, slot number is 0 only. */
    {
        slot = 0;

        /*
         * If the slot is bound to an TRANSPORT channel, signal the message to
         * the TRANSPORT channel.
         */
        if (device_ctx->bound_slots & (1 << slot)) {
            transport_channel = &device_ctx->transport_channel_table[slot];
            transport_channel->api->signal_message(transport_channel->id);
        }
    }
}

/*
 * TRANSPORT module driver API
 */
static int raise_interrupt(fwk_id_t slot_id)
{
    /* This function is unsupported. */
    return FWK_SUCCESS;
}

const struct mod_transport_driver_api mfismh_mod_transport_driver_api = {
    .trigger_event = raise_interrupt,
};

/*
 * Framework handlers
 */
static int mfismh_init(
    fwk_id_t module_id,
    unsigned int device_count,
    const void *unused)
{
    if (device_count == 0)
        return FWK_E_PARAM;

    mfismh_ctx.device_ctx_table =
        fwk_mm_calloc(device_count, sizeof(mfismh_ctx.device_ctx_table[0]));
    if (mfismh_ctx.device_ctx_table == NULL)
        return FWK_E_NOMEM;

    mfismh_ctx.device_count = device_count;

    return FWK_SUCCESS;
}

static int mfismh_device_init(
    fwk_id_t device_id,
    unsigned int slot_count,
    const void *data)
{
    struct mod_rcar_mfismh_device_config *config =
        (struct mod_rcar_mfismh_device_config *)data;
    struct mfismh_device_ctx *device_ctx;

    device_ctx =
        &mfismh_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    device_ctx->transport_channel_table = fwk_mm_calloc(
        slot_count, sizeof(device_ctx->transport_channel_table[0]));
    if (device_ctx->transport_channel_table == NULL)
        return FWK_E_NOMEM;

    device_ctx->config = config;
    device_ctx->slot_count = slot_count;

    mfis_regs = (struct mfismh_reg *)MFISAREICR_BASE;

    return FWK_SUCCESS;
}

static int mfismh_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mfismh_device_ctx *device_ctx;
    unsigned int slot;
    struct mfismh_transport_channel *transport_channel;

    if ((round == 1) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        device_ctx = &mfismh_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

        for (slot = 0; slot < MFISMH_SLOT_COUNT_MAX; slot++) {
            if (!(device_ctx->bound_slots & (1 << slot)))
                continue;

            transport_channel = &device_ctx->transport_channel_table[slot];

            status = fwk_module_bind(
                transport_channel->id,
                FWK_ID_API(
                    FWK_MODULE_IDX_TRANSPORT,
                    MOD_TRANSPORT_API_IDX_DRIVER_INPUT),
                &transport_channel->api);
            if (status != FWK_SUCCESS)
                return status;
        }
    }

    return FWK_SUCCESS;
}

static int mfismh_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    struct mfismh_device_ctx *device_ctx;
    unsigned int slot;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_SUB_ELEMENT))
        return FWK_E_ACCESS;

    device_ctx =
        &mfismh_ctx.device_ctx_table[fwk_id_get_element_idx(target_id)];
    slot = fwk_id_get_sub_element_idx(target_id);

    if (device_ctx->bound_slots & (1 << slot))
        return FWK_E_ACCESS;

    device_ctx->transport_channel_table[slot].id = source_id;
    device_ctx->bound_slots |= 1 << slot;

    *api = &mfismh_mod_transport_driver_api;

    return FWK_SUCCESS;
}

static int mfismh_start(fwk_id_t id)
{
    int status;
    struct mfismh_device_ctx *device_ctx;

    /* for Module */
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    device_ctx = &mfismh_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

    if (device_ctx->bound_slots != 0) {
        status = fwk_interrupt_set_isr(device_ctx->config->irq, &mfismh_isr);
        if (status != FWK_SUCCESS)
            return status;
        status = fwk_interrupt_enable(device_ctx->config->irq);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

/* MFISMH module definition */
const struct fwk_module module_rcar_mfismh = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = mfismh_init,
    .element_init = mfismh_device_init,
    .bind = mfismh_bind,
    .start = mfismh_start,
    .process_bind_request = mfismh_process_bind_request,
};
