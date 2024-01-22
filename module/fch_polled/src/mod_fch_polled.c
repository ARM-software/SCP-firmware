/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Generic implementation for fast channels polling.
 */

#include <mod_fch_polled.h>
#include <mod_timer.h>
#include <mod_transport.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

/* Platform FCH channel context */
struct mod_fch_polled_channel_ctx {
    /* Pointer to the channel configuration */
    const struct mod_fch_polled_channel_config *config;

    uintptr_t param;

    void (*fch_callback)(uintptr_t param);
};

/* Platform FCH context */
struct mod_fch_polled_ctx {
    /* Table of channel contexts */
    struct mod_fch_polled_channel_ctx *channel_ctx_table;

    /* Alarm API for fast channels */
    const struct mod_timer_alarm_api *fch_alarm_api;

    /* Number of channels in the channel context table*/
    unsigned int channel_count;

    /* Fast channel configuration */
    struct mod_fch_polled_config *fch_config;
};

static struct mod_fch_polled_ctx fch_polled_ctx;

static void fast_channel_alarm_callback(uintptr_t ch_ctx)
{
    struct mod_fch_polled_channel_ctx *channel_ctx;

    channel_ctx = (struct mod_fch_polled_channel_ctx *)ch_ctx;

    /* Call the callback function that has been registered for this channel */
    channel_ctx->fch_callback(channel_ctx->param);
}

static int start_alarm(struct mod_fch_polled_channel_ctx *channel_ctx)
{
    uint32_t fch_interval_msecs;

    /* Set the fast channel polling rate */
    fch_interval_msecs =
        FWK_MAX(
            (uint32_t)FCH_MIN_POLL_RATE_US,
            (uint32_t)fch_polled_ctx.fch_config->fch_poll_rate) /
        1000;

    /* Start the alarm */
    return fch_polled_ctx.fch_alarm_api->start(
        fch_polled_ctx.fch_config->fch_alarm_id,
        fch_interval_msecs,
        MOD_TIMER_ALARM_TYPE_PERIODIC,
        fast_channel_alarm_callback,
        (uintptr_t)channel_ctx);
}

/*
 * Transport Driver API
 */
static int mod_fch_polled_get_fch_address(
    fwk_id_t fch_id,
    struct mod_transport_fast_channel_addr *fch_addr)
{
    struct mod_fch_polled_channel_ctx *channel_ctx;

    channel_ctx =
        &fch_polled_ctx.channel_ctx_table[fwk_id_get_element_idx(fch_id)];

    fwk_assert(fch_addr != NULL);

    *fch_addr = channel_ctx->config->fch_addr;

    return FWK_SUCCESS;
}

static int mod_fch_polled_get_fch_interrupt_type(
    fwk_id_t fch_id,
    enum mod_transport_fch_interrupt_type *interrupt_type)
{
    fwk_assert(interrupt_type != NULL);

    *interrupt_type = MOD_TRANSPORT_FCH_INTERRUPT_TYPE_TIMER;

    return FWK_SUCCESS;
}

static int mod_fch_polled_get_fch_doorbell_info(
    fwk_id_t fch_id,
    struct mod_transport_fch_doorbell_info *doorbell_info)
{
    fwk_assert(doorbell_info != NULL);
    /*
     * Since this is a fast channel specific driver, doorbell is not
     * supported.
     */
    doorbell_info->doorbell_support = false;

    return FWK_SUCCESS;
}

static int mod_fch_polled_get_fch_rate_limit(
    fwk_id_t fch_id,
    uint32_t *fch_rate_limit)
{
    fwk_assert(fch_rate_limit != NULL);

    *fch_rate_limit = fch_polled_ctx.fch_config->rate_limit;

    return FWK_SUCCESS;
}

static int mod_fch_polled_register_callback(
    fwk_id_t fch_id,
    uintptr_t param,
    void (*fch_callback)(uintptr_t param))
{
    struct mod_fch_polled_channel_ctx *channel_ctx;

    channel_ctx =
        &fch_polled_ctx.channel_ctx_table[fwk_id_get_element_idx(fch_id)];

    fwk_assert(fch_callback != NULL);

    /* Store the context specific value */
    channel_ctx->param = param;

    /* Store the callback function pointer */
    channel_ctx->fch_callback = fch_callback;

    /* Start the alarm */
    return start_alarm(channel_ctx);
}

static const struct mod_transport_driver_api fch_polled_transport_driver_api = {
    .get_fch_address = mod_fch_polled_get_fch_address,
    .get_fch_interrupt_type = mod_fch_polled_get_fch_interrupt_type,
    .get_fch_doorbell_info = mod_fch_polled_get_fch_doorbell_info,
    .get_fch_rate_limit = mod_fch_polled_get_fch_rate_limit,
    .fch_register_callback = mod_fch_polled_register_callback,
};

/*
 * Framework API
 */
static int mod_fch_polled_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0u) {
        return FWK_E_DATA;
    }

    fch_polled_ctx.channel_ctx_table = fwk_mm_calloc(
        element_count, sizeof(fch_polled_ctx.channel_ctx_table[0]));

    fch_polled_ctx.channel_count = element_count;

    fch_polled_ctx.fch_config = (struct mod_fch_polled_config *)data;
    /* validate channel configuration */
    if (fch_polled_ctx.fch_config->fch_poll_rate == 0) {
        return FWK_E_DATA;
    }

    if (!fwk_id_type_is_valid(fch_polled_ctx.fch_config->fch_alarm_id)) {
        return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

static int mod_fch_polled_channel_init(
    fwk_id_t channel_id,
    unsigned int unused,
    const void *data)
{
    struct mod_fch_polled_channel_ctx *channel_ctx;

    const struct mod_transport_fast_channel_addr *fch_addr;

    channel_ctx =
        &fch_polled_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    channel_ctx->config = (struct mod_fch_polled_channel_config *)data;

    fch_addr = &channel_ctx->config->fch_addr;

    if ((fch_addr->local_view_address == (uintptr_t)NULL) ||
        (fch_addr->target_view_address == (uintptr_t)NULL) ||
        (fch_addr->length == 0)) {
        return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

static int mod_fch_polled_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1) {
        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    /* Bind to the timer alarm API */
    status = fwk_module_bind(
        fch_polled_ctx.fch_config->fch_alarm_id,
        MOD_TIMER_API_ID_ALARM,
        &fch_polled_ctx.fch_alarm_api);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("%sError while binding to alarm API", MOD_NAME);
        return FWK_E_PANIC;
    }

    return status;
}

static int mod_fch_polled_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    int status;
    enum mod_fch_polled_api_idx api_id_type =
        (enum mod_fch_polled_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
    case MOD_FCH_POLLED_API_IDX_TRANSPORT:
        *api = &fch_polled_transport_driver_api;
        status = FWK_SUCCESS;
        break;

    default:
        /* Invalid API */
        fwk_unexpected();
        status = FWK_E_PARAM;
    }

    return status;
}

const struct fwk_module module_fch_polled = {
    .api_count = (unsigned int)MOD_FCH_POLLED_API_IDX_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_fch_polled_init,
    .element_init = mod_fch_polled_channel_init,
    .bind = mod_fch_polled_bind,
    .process_bind_request = mod_fch_polled_process_bind_request,
};
