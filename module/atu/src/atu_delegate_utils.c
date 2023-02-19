/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      ATU<-->Transport module interface implementation
 */

#include <internal/atu.h>
#include <internal/atu_delegate_utils.h>

#include <mod_atu.h>
#include <mod_timer.h>
#include <mod_transport.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_status.h>
#include <fwk_string.h>

const char *atu_response_error_name[ATU_MSG_RESPONSE_COUNT] = {
    [ATU_MSG_RESPONSE_GENERAL_ERROR] = "Request failed",
    [ATU_MSG_RESPONSE_INVALID_REGION_ERROR] = "Invalid ATU region number",
    [ATU_MSG_RESPONSE_REGION_NOT_ALLOWED_ERROR] = "ATU region not authorized",
    [ATU_MSG_RESPONSE_REGION_INVALID_ADDRESS_ERROR] =
        "Invalid translation region",
    [ATU_MSG_RESPONSE_INVALID_REQUEST_ERROR] = "Invalid request",
    [ATU_MSG_RESPONSE_INVALID_PAYLOAD_ERROR] = "Invalid payload",
    [ATU_MSG_RESPONSE_REGION_OWNERSHIP_ERROR] =
        "Requester does not own the ATU region",
    [ATU_MSG_RESPONSE_REGION_OVERLAP_ERROR] =
        "Requested region overlaps with existing region",
};

static const unsigned int payload_size_table[ATU_MSG_ID_COUNT] = {
    [ATU_MSG_ID_REQ_ADD_REGION] = sizeof(struct atu_region_map),
    [ATU_MSG_ID_REQ_REMOVE_REGION] =
        sizeof(struct atu_msg_remove_region_payload),
};

static inline bool check_response_msg(void *device_ctx_ptr)
{
    struct atu_device_ctx *device_ctx;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    /* Return the flag */
    return device_ctx->is_new_msg_received;
}

static int atu_send_msg(
    enum atu_msg_id msg_id,
    const void *payload,
    size_t payload_size,
    void *device_ctx_ptr)
{
    struct atu_device_ctx *device_ctx;
    bool request_ack;
    int status;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    /* Send the request message using transport API */
    request_ack = false;
    status = device_ctx->transport_api->transmit(
        device_ctx->config->transport_id,
        (uint32_t)msg_id,
        payload,
        payload_size,
        request_ack);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[ATU] ERROR! Unable to send the request message");
        return status;
    }

    /* Wait till a response message is received or timeout value is reached */
    status = device_ctx->timer_api->wait(
        device_ctx->config->timer_id,
        device_ctx->config->response_wait_timeout_us,
        check_response_msg,
        (void *)device_ctx);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[ATU] ERROR! No response received. ATU request timed out");
        return FWK_E_TIMEOUT;
    }

    return FWK_SUCCESS;
}

int atu_send_msg_add_region(
    const struct atu_region_map *region,
    void *device_ctx_ptr,
    uint8_t *region_idx)
{
    struct atu_device_ctx *device_ctx;
    struct atu_msg_add_region_response *response;
    struct atu_msg_buffer *msg_buffer;
    int status;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    if (payload_size_table[ATU_MSG_ID_REQ_ADD_REGION] >
        device_ctx->payload_buffer_size) {
        return FWK_E_PARAM;
    }

    FWK_LOG_INFO("[ATU] Requesting to configure translation region...");

    /*
     * Send the request and wait for a response message.
     *
     * Note: The atu_send_msg() function returns when a response message is
     * received or when an error is encountered.
     */
    status = atu_send_msg(
        ATU_MSG_ID_REQ_ADD_REGION,
        (void *)region,
        payload_size_table[ATU_MSG_ID_REQ_ADD_REGION],
        device_ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /*
     * Clear the flag since the response message has been received and it is
     * being processed.
     */
    device_ctx->is_new_msg_received = false;
    msg_buffer = &device_ctx->recv_msg_buffer;

    if (msg_buffer->header != ATU_MSG_ID_REQ_ADD_REGION) {
        FWK_LOG_ERR("[ATU] Error! Invalid response received");
        return FWK_E_DATA;
    }

    response = (struct atu_msg_add_region_response *)msg_buffer->payload;

    /*
     * The requested operation was unsuccessful, if the expected response is
     * not received.
     */
    if (response->status != ATU_MSG_RESPONSE_SUCCESS) {
        FWK_LOG_ERR(
            "[ATU] Error! %s", atu_response_error_name[response->status]);
        return FWK_E_DATA;
    }

    FWK_LOG_INFO("[ATU] Success. Requested region enabled");

    /*
     * The response contains the region index of the mapped ATU region if the
     * request was processed successfully. Return the ATU region index back to
     * the caller.
     */
    *region_idx = response->region_idx;

    return status;
}

int atu_send_msg_remove_region(
    const uint8_t region_idx,
    void *device_ctx_ptr,
    fwk_id_t requester_id)
{
    struct atu_device_ctx *device_ctx;
    struct atu_msg_remove_region_response *response;
    struct atu_msg_buffer *msg_buffer;
    int status;

    struct atu_msg_remove_region_payload payload = {
        .region_idx = (uint32_t)region_idx,
        .requester_id = requester_id.value,
    };

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    FWK_LOG_INFO("[ATU] Requesting to remove translation region...");

    /*
     * Send the request and wait for a response message.
     *
     * Note: The atu_send_msg() function returns when a response message is
     * received or when an error is encountered.
     */
    status = atu_send_msg(
        ATU_MSG_ID_REQ_REMOVE_REGION,
        &payload,
        payload_size_table[ATU_MSG_ID_REQ_REMOVE_REGION],
        device_ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /*
     * Clear the flag since the response message has been received and it is
     * being processed.
     */
    device_ctx->is_new_msg_received = false;
    msg_buffer = &device_ctx->recv_msg_buffer;

    if (msg_buffer->header != ATU_MSG_ID_REQ_REMOVE_REGION) {
        FWK_LOG_ERR("[ATU] Error! Invalid response received");
        return FWK_E_DATA;
    }

    response = (struct atu_msg_remove_region_response *)msg_buffer->payload;

    /*
     * The requested operation was unsuccessful, if the expected response is
     * not received.
     */
    if (response->status != ATU_MSG_RESPONSE_SUCCESS) {
        FWK_LOG_ERR(
            "[ATU] Error! %s", atu_response_error_name[response->status]);
        return FWK_E_DATA;
    }

    FWK_LOG_INFO("[ATU] Success. Requested region removed");

    return FWK_SUCCESS;
}
