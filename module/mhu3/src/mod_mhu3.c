/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit 3 (MHU3) Device Driver.
 */

#include <internal/mhu3.h>

#include <mod_mhu3.h>
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

/* MHU channel context */
struct mhu3_channel_ctx {
    /* ID of the transport channel the MHUv3 channel is bound to */
    fwk_id_t transport_id;
    /* Indicates whether the transport channel is bound to the MHUv3 channel */
    bool transport_id_bound;
    /* Transport API */
    const struct mod_transport_driver_input_api *transport_api;
    /*! Fast Channel Callback Parameter */
    uintptr_t callback_param;
    /*! Fast Channel Callback on isr */
    void (*callback)(uintptr_t param);
};

/* MHU device context */
struct mhu3_device_ctx {
    /* Pointer to the device configuration */
    const struct mod_mhu3_device_config *config;
    /* Table of channel contexts */
    struct mhu3_channel_ctx *channel_ctx_table;
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

static void mhu3_isr(void)
{
    int status;
    unsigned int interrupt;
    unsigned int device_idx;
    struct mhu3_device_ctx *device_ctx;
    struct mod_mhu3_channel_config *channel;
    struct mhu3_channel_ctx *channel_ctx;
    struct mhu3_mbx_reg *mbx_reg;
    struct mhu3_mbx_mdbcw_reg *mdbcw_reg;
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
    /*
     * In case we need to use it for doorbell. This way we don't have to assign
     * it every time we have a doorbell channel inside the loop
     */
    mdbcw_reg = (struct mhu3_mbx_mdbcw_reg
                     *)((uint8_t *)mbx_reg + MHU3_MBX_MDBCW_PAGE_OFFSET);

    while (pending < device_ctx->channels_count) {
        channel = &(device_ctx->config->channels[pending]);
        channel_ctx = &(device_ctx->channel_ctx_table[pending]);
        switch (channel->type) {
        case MOD_MHU3_CHANNEL_TYPE_DBCH:
            /*
             * This implementation will not work if configured hardware(MHU3)
             * requires to support more than 32 doorbell channels.
             *
             * Status of the interrupts of doorbell channel
             * is read using MBX_DBCH_INT_ST<n> register where
             * n = 0..3, that is, each 32 bits of MBX_DBCH_INT_ST<n> will
             * indicate status of each corresponding doorbell channel
             * 0b1 indicates interrupt pending.
             *
             * NOTE: We only check MBX_DBCH_INT_ST[0] because
             * although MHUv3 supports upto 128 channels, it is not
             * expected hardware to be configured more than 32 channels.
             */
            if (((1u << channel->dbch.mbx_channel) &
                 mbx_reg->MBX_DBCH_INT_ST[0]) != 0u) {
                /*
                 * Clear Doorbell flag, we should clear only the flag(bit) which
                 * is set. However, we are using only one flag(bit) of
                 * corresponding doorbell channel for communication.
                 */
                mdbcw_reg[channel->dbch.mbx_channel].MDBCW_CLR |=
                    (1UL << channel->dbch.mbx_flag_pos);
                if (channel_ctx->transport_id_bound) {
                    channel_ctx->transport_api->signal_message(
                        channel_ctx->transport_id);
                }
            }
            break;

        case MOD_MHU3_CHANNEL_TYPE_FCH:
            if (((mbx_reg->MBX_FCH_GRP_INT_ST[channel->fch.grp_num] >>
                  channel->fch.idx) &
                 1u) != 0u) {
                channel_ctx = &(device_ctx->channel_ctx_table[pending]);
                /*
                 * We only check for whether the callback is NULL as the
                 * register callback function checks for both callback and
                 * callback_param before registering so that we can save a few
                 * cycles here.
                 */
                if (channel_ctx->callback != NULL) {
                    channel_ctx->callback(channel_ctx->callback_param);
                }
            }
            break;

        default:
            break;
        }
        pending++;
    }
}

static int mhu3_raise_interrupt(fwk_id_t ch_id)
{
    int status;
    struct mhu3_device_ctx *device_ctx;
    unsigned int ch_idx;
    struct mhu3_pbx_reg *pbx_reg;
    struct mhu3_pbx_pdbcw_reg *pdbcw_reg;
    struct mod_mhu3_channel_config *channel;
    struct mod_mhu3_dbch_config *dbch_channel;

    device_ctx = &mhu3_ctx.device_ctx_table[fwk_id_get_element_idx(ch_id)];
    ch_idx = fwk_id_get_sub_element_idx(ch_id);
    pbx_reg = (struct mhu3_pbx_reg *)device_ctx->config->out;
    channel = &(device_ctx->config->channels[ch_idx]);

    switch (channel->type) {
    case MOD_MHU3_CHANNEL_TYPE_DBCH:
        pdbcw_reg = (struct mhu3_pbx_pdbcw_reg
                         *)((uint8_t *)pbx_reg + MHU3_PBX_PDBCW_PAGE_OFFSET);
        dbch_channel = &(channel->dbch);
        /*
         * We can use up to 32 flags(bits) per channels for 32 events, however
         * we are using only 1 flag(bit) per channel
         */
        if ((pdbcw_reg[dbch_channel->pbx_channel].PDBCW_ST &
             0x1u << dbch_channel->pbx_flag_pos) == 0u) {
            pdbcw_reg[dbch_channel->pbx_channel].PDBCW_SET |= 0x1u
                << dbch_channel->pbx_flag_pos;
            status = FWK_SUCCESS;
        } else {
            status = FWK_E_STATE;
        }
        break;

    default:
        status = FWK_E_PARAM;
        break;
    }

    return status;
}

#ifdef BUILD_HAS_FAST_CHANNELS

static int mhu3_get_fch_address(
    fwk_id_t fch_id,
    struct mod_transport_fast_channel_addr *fch_addr)
{
    int status = FWK_E_PARAM;
    unsigned int ch_idx, fch_offset;
    struct mhu3_device_ctx *device_ctx;
    struct mhu3_pbx_reg *pbx_reg, *pbx_reg_target;
    struct mhu3_mbx_reg *mbx_reg, *mbx_reg_target;
    uintptr_t fcw_regs, fcw_regs_target;
    struct mod_mhu3_channel_config *channel;
    struct mod_mhu3_fc_config *fch_channel;
    uint32_t fch_cfg0;

    if (!fwk_module_is_valid_sub_element_id(fch_id) || (fch_addr == NULL)) {
        return status;
    }

    device_ctx = &mhu3_ctx.device_ctx_table[fwk_id_get_element_idx(fch_id)];
    ch_idx = fwk_id_get_sub_element_idx(fch_id);
    pbx_reg = (struct mhu3_pbx_reg *)device_ctx->config->out;
    mbx_reg = (struct mhu3_mbx_reg *)device_ctx->config->in;

    pbx_reg_target = (struct mhu3_pbx_reg *)device_ctx->config->out_target;
    mbx_reg_target = (struct mhu3_mbx_reg *)device_ctx->config->in_target;

    channel = &(device_ctx->config->channels[ch_idx]);

    switch (channel->type) {
    case MOD_MHU3_CHANNEL_TYPE_FCH:
        fch_channel = &(channel->fch);
        if (fch_channel->direction == MOD_MHU3_FCH_DIR_OUT) {
            fcw_regs = (uintptr_t)pbx_reg + MHU3_PBX_PFCW_PAGE_OFFSET;
            fcw_regs_target =
                (uintptr_t)mbx_reg_target + MHU3_MBX_MDFCW_PAGE_OFFSET;
            fch_cfg0 = pbx_reg->PBX_FCH_CFG0;

            status = FWK_SUCCESS;
        } else if (fch_channel->direction == MOD_MHU3_FCH_DIR_IN) {
            fcw_regs = (uintptr_t)mbx_reg + MHU3_MBX_MDFCW_PAGE_OFFSET;
            fcw_regs_target =
                (uintptr_t)pbx_reg_target + MHU3_PBX_PFCW_PAGE_OFFSET;
            fch_cfg0 = mbx_reg->MBX_FCH_CFG0;

            status = FWK_SUCCESS;
        }

        if (status == FWK_SUCCESS) {
            fch_offset = fch_channel->idx +
                fch_channel->grp_num *
                    MHU3_MASKED_RECOVER(
                        fch_cfg0 & MHU3_FCH_CFG0_NUM_FCH_PER_GRP_MASK,
                        MHU3_FCH_CFG0_NUM_FCH_PER_GRP_BITSTART,
                        MHU3_FCH_CFG0_NUM_FCH_PER_GRP_LEN);
            fch_addr->length = (MHU3_MASKED_RECOVER(
                                    fch_cfg0 & MHU3_FCH_CFG0_FCH_WS_MASK,
                                    MHU3_FCH_CFG0_FCH_WS_BITSTART,
                                    MHU3_FCH_CFG0_FCH_WS_LEN) == FCH_WS_32BIT) ?
                sizeof(uint32_t) :
                sizeof(uint64_t);
            fch_offset *= fch_addr->length;
            fch_addr->local_view_address = fcw_regs + fch_offset;
            fch_addr->target_view_address = fcw_regs_target + fch_offset;
        }

        break;

    default:
        break;
    }

    return status;
}

static int mhu3_get_fch_interrupt_type(
    fwk_id_t fch_id,
    enum mod_transport_fch_interrupt_type *fch_interrupt_type)
{
    struct mhu3_device_ctx *device_ctx;
    struct mod_mhu3_channel_config *channel;
    unsigned int ch_idx;

    if (!fwk_module_is_valid_sub_element_id(fch_id) ||
        (fch_interrupt_type == NULL)) {
        return FWK_E_PARAM;
    }

    device_ctx = &mhu3_ctx.device_ctx_table[fwk_id_get_element_idx(fch_id)];
    ch_idx = fwk_id_get_sub_element_idx(fch_id);
    channel = &(device_ctx->config->channels[ch_idx]);

    if (channel->type != MOD_MHU3_CHANNEL_TYPE_FCH) {
        return FWK_E_PARAM;
    }

    *fch_interrupt_type = MOD_TRANSPORT_FCH_INTERRUPT_TYPE_HW;

    return FWK_SUCCESS;
}

static int mhu3_fch_register_callback(
    fwk_id_t fch_id,
    uintptr_t param,
    void (*fch_callback)(uintptr_t param))
{
    struct mhu3_device_ctx *device_ctx;
    unsigned int ch_idx;
    struct mhu3_channel_ctx *channel_ctx;

    if ((fch_callback == NULL) || ((void *)param == NULL)) {
        return FWK_E_PARAM;
    }

    if (!fwk_module_is_valid_sub_element_id(fch_id)) {
        return FWK_E_PARAM;
    }

    device_ctx = &mhu3_ctx.device_ctx_table[fwk_id_get_element_idx(fch_id)];
    ch_idx = fwk_id_get_sub_element_idx(fch_id);
    channel_ctx = &(device_ctx->channel_ctx_table[ch_idx]);
    channel_ctx->callback = fch_callback;
    channel_ctx->callback_param = param;

    return FWK_SUCCESS;
}

#endif

static struct mod_transport_driver_api mhu3_mod_transport_driver_api = {
    .trigger_event = mhu3_raise_interrupt,
#ifdef BUILD_HAS_FAST_CHANNELS
    .get_fch_address = mhu3_get_fch_address,
    .get_fch_interrupt_type = mhu3_get_fch_interrupt_type,
    .fch_register_callback = mhu3_fch_register_callback,
#endif
};

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
    int status = FWK_SUCCESS;
    struct mod_mhu3_device_config *config =
        (struct mod_mhu3_device_config *)data;
    struct mhu3_device_ctx *device_ctx;
    struct mhu3_mbx_reg *mbx_reg;
    struct mhu3_pbx_reg *pbx_reg;
    uint32_t fch_cfg0, ext_suprt;
    struct mod_mhu3_channel_config *channel;
    unsigned int channel_num;

    if ((config->in == 0u) || (config->out == 0u)) {
        return FWK_E_PARAM;
    }

    device_ctx = &mhu3_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    device_ctx->config = config;
    device_ctx->channels_count = sub_element_count;
    device_ctx->channel_ctx_table = fwk_mm_calloc(
        sub_element_count, sizeof(device_ctx->channel_ctx_table[0]));

    mbx_reg = (struct mhu3_mbx_reg *)device_ctx->config->in;
    pbx_reg = (struct mhu3_pbx_reg *)device_ctx->config->out;

    /*
     * Loop to verify that the desired Fast Channel Number and Group
     * are correct according to Hardware
     */
    for (channel_num = 0u; channel_num < sub_element_count; channel_num++) {
        channel = &(device_ctx->config->channels[channel_num]);
        if (channel->type == MOD_MHU3_CHANNEL_TYPE_FCH) {
            if (channel->fch.direction == MOD_MHU3_FCH_DIR_IN) {
                fch_cfg0 = mbx_reg->MBX_FCH_CFG0;
                ext_suprt = mbx_reg->MBX_FEAT_SPT0;
            } else {
                fch_cfg0 = pbx_reg->PBX_FCH_CFG0;
                ext_suprt = pbx_reg->PBX_FEAT_SPT0;
            }
            if (((ext_suprt & MHU3_FEAT_SPT0_FCE_SPT_MASK) == 0u) ||
                (channel->fch.grp_num >
                 MHU3_MASKED_RECOVER(
                     fch_cfg0 & MHU3_FCH_CFG0_NUM_FCG_MASK,
                     MHU3_FCH_CFG0_NUM_FCG_BITSTART,
                     MHU3_FCH_CFG0_NUM_FCG_LEN))) {
                status = FWK_E_PARAM;
                break;
            }
            if ((fch_cfg0 & MHU3_FCH_CFG0_NUM_FCH_PER_GRP_MASK) == 0u) {
                if (channel->fch.idx >
                    MHU3_MASKED_RECOVER(
                        fch_cfg0 & MHU3_FCH_CFG0_NUM_FCH_MASK,
                        MHU3_FCH_CFG0_NUM_FCH_BITSTART,
                        MHU3_FCH_CFG0_NUM_FCH_LEN)) {
                    status = FWK_E_PARAM;
                    break;
                }
            } else {
                if (channel->fch.idx >
                    MHU3_MASKED_RECOVER(
                        fch_cfg0 & MHU3_FCH_CFG0_NUM_FCH_PER_GRP_MASK,
                        MHU3_FCH_CFG0_NUM_FCH_PER_GRP_BITSTART,
                        MHU3_FCH_CFG0_NUM_FCH_PER_GRP_LEN)) {
                    status = FWK_E_PARAM;
                    break;
                }
                if ((channel->fch.grp_num ==
                     MHU3_MASKED_RECOVER(
                         fch_cfg0 & MHU3_FCH_CFG0_NUM_FCG_MASK,
                         MHU3_FCH_CFG0_NUM_FCG_BITSTART,
                         MHU3_FCH_CFG0_NUM_FCG_LEN)) &&
                    ((fch_cfg0 & MHU3_FCH_CFG0_NUM_FCG_MASK) != 0u)) {
                    /*
                     * Verify that the last Group contains an appropiate amounts
                     * of Channels.
                     */
                    if (channel->fch.idx >=
                        (MHU3_MASKED_RECOVER(
                             fch_cfg0 & MHU3_FCH_CFG0_NUM_FCH_MASK,
                             MHU3_FCH_CFG0_NUM_FCH_BITSTART,
                             MHU3_FCH_CFG0_NUM_FCH_LEN) -
                         (MHU3_MASKED_RECOVER(
                              fch_cfg0 & MHU3_FCH_CFG0_NUM_FCG_MASK,
                              MHU3_FCH_CFG0_NUM_FCG_BITSTART,
                              MHU3_FCH_CFG0_NUM_FCG_LEN) -
                          1u) *
                             MHU3_MASKED_RECOVER(
                                 fch_cfg0 & MHU3_FCH_CFG0_NUM_FCH_PER_GRP_MASK,
                                 MHU3_FCH_CFG0_NUM_FCH_PER_GRP_BITSTART,
                                 MHU3_FCH_CFG0_NUM_FCH_PER_GRP_LEN))) {
                        status = FWK_E_PARAM;
                        break;
                    }
                }
            }
        }
    }

    return status;
}

static int mhu3_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mhu3_device_ctx *device_ctx;
    struct mhu3_channel_ctx *channel_ctx;
    unsigned int channel_idx;

    if ((round == 1) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        device_ctx = &mhu3_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

        for (channel_idx = 0; channel_idx < device_ctx->channels_count;
             channel_idx++) {
            channel_ctx = &device_ctx->channel_ctx_table[channel_idx];
            if (channel_ctx->transport_id_bound) {
                status = fwk_module_bind(
                    channel_ctx->transport_id,
                    FWK_ID_API(
                        FWK_MODULE_IDX_TRANSPORT,
                        MOD_TRANSPORT_API_IDX_DRIVER_INPUT),
                    &channel_ctx->transport_api);

                if (status != FWK_SUCCESS) {
                    /* Unable to bind back to TRANSPORT channel */
                    fwk_unexpected();
                    return status;
                }
            }
        }
    }

    return FWK_SUCCESS;
}

static int mhu3_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    enum mod_mhu3_api_idx api_id_type;
    struct mhu3_device_ctx *device_ctx;
    struct mhu3_channel_ctx *channel_ctx;
    unsigned int ch_idx;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_SUB_ELEMENT)) {
        /*
         * Something tried to bind to the module or an element. Only binding to
         * a slot (sub-element) is allowed.
         */
        fwk_unexpected();
        return FWK_E_ACCESS;
    }

    device_ctx = &mhu3_ctx.device_ctx_table[fwk_id_get_element_idx(target_id)];
    ch_idx = fwk_id_get_sub_element_idx(target_id);

    channel_ctx = &(device_ctx->channel_ctx_table[ch_idx]);
    api_id_type = (enum mod_mhu3_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
    case MOD_MHU3_API_IDX_TRANSPORT_DRIVER:
        channel_ctx->transport_id = source_id;
        channel_ctx->transport_id_bound = true;
        *api = &mhu3_mod_transport_driver_api;
        break;

    default:
        /* Invalid config */
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int mhu3_start(fwk_id_t id)
{
    int status;
    unsigned int i;
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

    status = fwk_interrupt_set_isr(device_ctx->config->irq, &mhu3_isr);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_interrupt_enable(device_ctx->config->irq);
    if (status != FWK_SUCCESS) {
        return status;
    }

    for (i = 0u; i < device_ctx->channels_count; i++) {
        if (device_ctx->config->channels[i].type == MOD_MHU3_CHANNEL_TYPE_FCH) {
            mbx->MBX_FCH_CTRL |= SCP_MHU3_MBX_INT_EN;
            mbx->MBX_FCG_INT_EN |= 1u
                << device_ctx->config->channels[i].fch.grp_num;
        }
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
