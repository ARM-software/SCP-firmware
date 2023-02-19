/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Delegated ATU management - ATU driver relies on a external component.
 */

#include <internal/atu.h>
#include <internal/atu_common.h>
#include <internal/atu_delegate_utils.h>

#include <mod_atu.h>
#include <mod_timer.h>
#include <mod_transport.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

/* Shared ATU module context */
static struct mod_atu_ctx *shared_atu_ctx;

/*
 * ATU API implementation
 */
static int atu_add_region(
    const struct atu_region_map *region,
    fwk_id_t atu_device_id,
    uint8_t *region_idx)
{
    struct atu_device_ctx *device_ctx;
    int status;

    /* Return error if region or region index parameters are null pointers */
    if ((region == NULL) || (region_idx == NULL)) {
        return FWK_E_PARAM;
    }

    /* Validate the output bus attributes for the ATU region */
    status = atu_validate_region_attributes(region->attributes);
    if (status != FWK_SUCCESS) {
        return status;
    }

    device_ctx = &shared_atu_ctx
                      ->device_ctx_table[fwk_id_get_element_idx(atu_device_id)];

    return atu_send_msg_add_region(region, device_ctx, region_idx);
}

static int atu_remove_region(
    uint8_t region_idx,
    fwk_id_t atu_device_id,
    fwk_id_t requester_id)
{
    struct atu_device_ctx *device_ctx;

    device_ctx = &shared_atu_ctx
                      ->device_ctx_table[fwk_id_get_element_idx(atu_device_id)];

    return atu_send_msg_remove_region(region_idx, device_ctx, requester_id);
}

const struct mod_atu_api atu_delegate_api = {
    .add_region = atu_add_region,
    .remove_region = atu_remove_region,
};

/*
 * Framework helper functions
 */
static int atu_device_init(fwk_id_t device_id)
{
    struct atu_device_ctx *device_ctx;

    if (shared_atu_ctx == NULL) {
        return FWK_E_DATA;
    }

    device_ctx =
        &shared_atu_ctx->device_ctx_table[fwk_id_get_element_idx(device_id)];

    if (device_ctx->config->response_wait_timeout_us == 0) {
        return FWK_E_PARAM;
    }

    device_ctx->atu_api = &atu_delegate_api;

    return FWK_SUCCESS;
}

static int atu_bind(struct atu_device_ctx *device_ctx)
{
    int status;
    fwk_id_t timer_api_id, transport_api_id;

    transport_api_id =
        FWK_ID_API(FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_FIRMWARE);

    /* Bind to transport module firmware api */
    status = fwk_module_bind(
        device_ctx->config->transport_id,
        transport_api_id,
        &device_ctx->transport_api);

    if (status != FWK_SUCCESS) {
        return status;
    }

    timer_api_id = FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER);

    /* Bind to timer API */
    status = fwk_module_bind(
        device_ctx->config->timer_id, timer_api_id, &device_ctx->timer_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

static int atu_start(struct atu_device_ctx *device_ctx)
{
    /* Get mailbox size from transport module */
    device_ctx->transport_api->get_max_payload_size(
        device_ctx->config->transport_id, &device_ctx->payload_buffer_size);

    /* Allocate memory for response message buffer */
    device_ctx->recv_msg_buffer.payload =
        fwk_mm_calloc(1, device_ctx->payload_buffer_size);

    return FWK_SUCCESS;
}

static int atu_delegate_set_shared_ctx(struct mod_atu_ctx *atu_ctx_param)
{
    if (atu_ctx_param == NULL) {
        return FWK_E_PARAM;
    }

    shared_atu_ctx = atu_ctx_param;

    return FWK_SUCCESS;
}

/* Pointers to ATU driver module framework handlers */
static struct mod_atu_ops mod_atu_delegate_ops = {
    .atu_init_shared_ctx = atu_delegate_set_shared_ctx,
    .atu_device_init = atu_device_init,
    .atu_bind = atu_bind,
    .atu_start = atu_start,
};

/*
 * Transport module Firmware signal API implementation
 */
static int signal_error(fwk_id_t atu_device_id)
{
    struct atu_device_ctx *device_ctx;

    FWK_LOG_ERR("[ATU] ERROR in the received message");

    device_ctx = &shared_atu_ctx
                      ->device_ctx_table[fwk_id_get_element_idx(atu_device_id)];

    /* Release the transport channel context lock */
    device_ctx->transport_api->release_transport_channel_lock(
        device_ctx->config->transport_id);

    return FWK_SUCCESS;
}

static int signal_message(fwk_id_t atu_device_id)
{
    fwk_id_t ch_id;
    struct atu_device_ctx *device_ctx;
    int status;

    device_ctx = &shared_atu_ctx
                      ->device_ctx_table[fwk_id_get_element_idx(atu_device_id)];

    ch_id = device_ctx->config->transport_id;

    /* Get the message header from transport module */
    status = device_ctx->transport_api->get_message_header(
        ch_id, &device_ctx->recv_msg_buffer.header);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[ATU] ERROR! Unable to read response message header");
        FWK_LOG_ERR("[ATU] status: %s", fwk_status_str(status));
    }

    /* Get the message payload from transport module */
    status = device_ctx->transport_api->get_payload(
        ch_id,
        (void *)&device_ctx->recv_msg_buffer.payload,
        &device_ctx->recv_msg_buffer.payload_size);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[ATU] ERROR! Unable to read response payload");
        FWK_LOG_ERR("[ATU] status: %s", fwk_status_str(status));
    }

    if (device_ctx->recv_msg_buffer.payload_size == 0) {
        FWK_LOG_ERR("[ATU] Error! Invalid response received");
    }

    /*
     * Release the transport channel context lock so that the next message
     * can be processed.
     */
    status = device_ctx->transport_api->release_transport_channel_lock(ch_id);

    if (status == FWK_SUCCESS) {
        /* Set the flag to indicate that a new response has been received */
        device_ctx->is_new_msg_received = true;
    }

    return FWK_SUCCESS;
}

const struct mod_transport_firmware_signal_api signal_api = {
    .signal_error = signal_error,
    .signal_message = signal_message,
};

void atu_get_signal_api(const void **api)
{
    *api = &signal_api;
}

void atu_get_delegate_ops(const struct mod_atu_ops **atu_ops)
{
    *atu_ops = &mod_atu_delegate_ops;
}
