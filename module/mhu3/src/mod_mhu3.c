/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit 3 (MHU3) Device Driver.
 */

#include <internal/mhu3.h>

#include <mod_mhu3.h>

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
 * Maximum number doorbell channels.
 *
 * MHUv3 architecture supports up to 128 channels but
 * only few channels are expected to be configured on
 * a typical platform for communication between
 *    SCP <---> MCP
 *    SCP <---> RSS
 *    SCP <---> AP
 *    SCP <---> LCPn
 */
#define MHU_DOORBELL_CHANNEL_COUNT_MAX 6

/* MHU device context */
struct mhu3_device_ctx {
    /* Pointer to the device configuration */
    const struct mod_mhu3_device_config *config;

    /* Number of channels (represented by sub-elements) */
    unsigned int channels_count;
};

/* MHU context */
struct mod_mhu3_ctx {
    /* Table of device contexts */
    struct mhu3_device_ctx *device_ctx_table;

    /* Number of devices in the device context table */
    unsigned int device_count;
};

static struct mod_mhu3_ctx mhu3_ctx;

/*
 * This implementation will not work if configured hardware(MHU3)
 * requires to support more than 32 doorbell channels.
 */
static void mhu3_dbch_isr(void)
{
    int status;
    unsigned int interrupt;
    unsigned int device_idx;
    struct mhu3_device_ctx *device_ctx;
    struct mhu3_mbx_reg *mbx_reg;
    struct mhu3_mbx_mdbcw_reg *mdbcw_reg;
    struct mod_mhu3_dbch_config *channel;
    unsigned int pending = 0U;

    status = fwk_interrupt_get_current(&interrupt);
    if (status != FWK_SUCCESS) {
        return;
    }

    for (device_idx = 0U; device_idx < mhu3_ctx.device_count; device_idx++) {
        device_ctx = &mhu3_ctx.device_ctx_table[device_idx];
        if (device_ctx->config->irq == interrupt) {
            break;
        }
    }

    if (device_idx >= mhu3_ctx.device_count) {
        return;
    }

    mbx_reg = (struct mhu3_mbx_reg *)device_ctx->config->in;
    mdbcw_reg = (struct mhu3_mbx_mdbcw_reg
                     *)((uint8_t *)mbx_reg + MHU3_MBX_MDBCW_PAGE_OFFSET);

    while (pending < device_ctx->channels_count) {
        channel = &(device_ctx->config->channels[pending].dbch);
        /* Status of the interrupts of doorbell channel
         * is read using MBX_DBCH_INT_ST<n> register where
         * n is 4, that is, each 32 bits of MBX_DBCH_INT_ST<n> will
         * indicate status of each corresponding doorbell channel
         * 0b1 indicates interrupt pending
         *
         * NOTE: We only check MBX_DBCH_INT_ST[0] because
         * although MHUv3 supports upto 128 channels, it is not
         * expected hardware to be configured more than 32 channels.
         */
        if ((1U << channel->mbx_channel) & mbx_reg->MBX_DBCH_INT_ST[0]) {
            /*
             * Clear Doorbell flag, we should clear only flag(bit) which is set
             * However we are using only one flag(bit) of corresponding
             * doorbell channel for communication
             */
            mdbcw_reg[channel->mbx_channel].MDBCW_CLR |=
                (1UL << channel->mbx_flag_pos);
        }

        pending++;
    }
}

/*
 * Framework handlers
 */

static int mhu3_init(
    fwk_id_t module_id,
    unsigned int device_count,
    const void *unused)
{
    if (device_count == 0U) {
        return FWK_E_PARAM;
    }

    mhu3_ctx.device_ctx_table =
        fwk_mm_calloc(device_count, sizeof(mhu3_ctx.device_ctx_table[0]));

    mhu3_ctx.device_count = device_count;

    return FWK_SUCCESS;
}

static int mhu3_device_init(
    fwk_id_t device_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_mhu3_device_config *config =
        (struct mod_mhu3_device_config *)data;
    struct mhu3_device_ctx *device_ctx;

    if ((config->in == 0) || (config->out == 0)) {
        return FWK_E_PARAM;
    }

    device_ctx = &mhu3_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    device_ctx->config = config;
    device_ctx->channels_count = sub_element_count;

    return FWK_SUCCESS;
}

static int mhu3_bind(fwk_id_t id, unsigned int round)
{
    return FWK_SUCCESS;
}

static int mhu3_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    return FWK_SUCCESS;
}

static int mhu3_start(fwk_id_t id)
{
    int status;
    struct mhu3_device_ctx *device_ctx;
    struct mhu3_pbx_reg *pbx;
    struct mhu3_mbx_reg *mbx;

    if (fwk_id_get_type(id) == FWK_ID_TYPE_MODULE) {
        return FWK_SUCCESS;
    }

    device_ctx = &mhu3_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

    pbx = (struct mhu3_pbx_reg *)device_ctx->config->out;
    mbx = (struct mhu3_mbx_reg *)device_ctx->config->in;
    if (pbx != NULL) {
        pbx->PBX_CTRL |= MHU3_OP_REQ;
    }
    if (mbx != NULL) {
        mbx->MBX_CTRL |= MHU3_OP_REQ;
    }

    status = fwk_interrupt_set_isr(device_ctx->config->irq, &mhu3_dbch_isr);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_interrupt_enable(device_ctx->config->irq);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

/* MHU module definition */
const struct fwk_module module_mhu3 = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = (unsigned int)MOD_MHU3_API_IDX_COUNT,
    .init = mhu3_init,
    .element_init = mhu3_device_init,
    .bind = mhu3_bind,
    .start = mhu3_start,
    .process_bind_request = mhu3_process_bind_request,
};
