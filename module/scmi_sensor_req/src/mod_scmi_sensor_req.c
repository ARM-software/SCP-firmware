/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) Sensor
 *      requester support.
 */

#include "internal/scmi_sensor_req.h"
#include "mod_scmi_header.h"
#include "mod_scmi_sensor_req.h"

#include <mod_scmi.h>
#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*
 * Per element context
 */
struct scmi_sensor_req_elem_ctx {
    const struct scmi_sensor_req_config *config;
};

/*
 * General context
 */
struct scmi_sensor_req_context {
    /* token to track the sent messages */
    uint8_t token;
    /* number of configured elements */
    uint32_t element_count;
    /* SCMI send message API */
    const struct mod_scmi_from_protocol_req_api *scmi_api;
    /* SCMI command return data API */
    const struct mod_sensor_driver_response_api *resp_api;
    /* Per element context */
    struct scmi_sensor_req_elem_ctx *ctx_table;
};

static int scmi_sensor_req_ret_reading_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size);

/*
 * Internal variables.
 */
static struct scmi_sensor_req_context scmi_sensor_req_ctx;

static int (*handler_table[MOD_SCMI_SENSOR_COMMAND_COUNT])(
    fwk_id_t,
    const uint32_t *,
    size_t) = {
    [MOD_SCMI_SENSOR_READING_GET] = scmi_sensor_req_ret_reading_handler
};

static unsigned int payload_size_table[MOD_SCMI_SENSOR_COMMAND_COUNT] = {
    [MOD_SCMI_SENSOR_READING_GET] =
        (unsigned int)sizeof(struct scmi_sensor_protocol_reading_get_p2a),
};

static_assert(
    FWK_ARRAY_SIZE(handler_table) == FWK_ARRAY_SIZE(payload_size_table),
    "[SCMI] Sensor management protocol table sizes not consistent");

/*
 * Static helper for getting the corresponding sensor HAL ID from
 * a given Service ID.
 */
static int get_sensor_hal_id_from_service_id(
    fwk_id_t service_id,
    fwk_id_t *sensor_hal_id)
{
    unsigned int sens_req_idx;
    int status = FWK_E_PARAM;

    /*
     * Linear search for the element that is configured with the
     * given Service ID.
     */
    for (sens_req_idx = 0u; sens_req_idx < scmi_sensor_req_ctx.element_count;
         sens_req_idx++) {
        if (fwk_id_is_equal(
                service_id,
                scmi_sensor_req_ctx.ctx_table[sens_req_idx]
                    .config->service_id)) {
            *sensor_hal_id = scmi_sensor_req_ctx.ctx_table[sens_req_idx]
                                 .config->sensor_hal_id;
            status = FWK_SUCCESS;
            break;
        }
    }

    return status;
}

/*
 * Sensor Requester Response handlers
 */

/*
 * Return Sensor reading handler. This is the Reading Get response handler.
 * It calls the Sensor HAL callback function to return back the reading.
 */
static int scmi_sensor_req_ret_reading_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size)
{
    struct mod_sensor_driver_resp_params resp_params = { 0 };
    fwk_id_t sensor_hal_id;
    int32_t ret_status;
    int status;

    /*
     * Get the Sensor ID element which corresponds to service_id.
     */
    status = get_sensor_hal_id_from_service_id(service_id, &sensor_hal_id);

    if (status == FWK_SUCCESS) {
        /*
         * As per the SCMI spec, the return values (payload) are:
         *  - int32 status
         *  - readings[n]
         */
        ret_status = *((int32_t *)payload);
        resp_params.status =
            (ret_status == SCMI_SUCCESS) ? FWK_SUCCESS : FWK_E_DEVICE;
        payload++;
        resp_params.value = *((uint64_t *)payload);
        scmi_sensor_req_ctx.resp_api->reading_complete(
            sensor_hal_id, &resp_params);
    }

    return status;
}

/*
 * Sensor Requester APIs implementations
 */

/*
 * Sensor read value request. This function is the get_value implementation
 * of the Sensor HAL driver API. It sends the Reading Get SCMI command to the
 * required platform.
 */
static int scmi_sensor_req_get_value(fwk_id_t id, mod_sensor_value_t *value)
{
    int status;
    uint8_t scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_SENSOR;
    uint8_t scmi_message_id = (uint8_t)MOD_SCMI_SENSOR_READING_GET;
    uint32_t element_idx;
    struct scmi_sensor_req_elem_ctx *ctx;
    struct scmi_sensor_protocol_reading_get_a2p payload = { 0 };

    element_idx = fwk_id_get_element_idx(id);

    if (element_idx < scmi_sensor_req_ctx.element_count) {
        ctx = &(scmi_sensor_req_ctx.ctx_table[element_idx]);

        payload.sensor_id = ctx->config->scmi_sensor_id;
        payload.flags = (uint32_t)(ctx->config->async_flag);

        status = scmi_sensor_req_ctx.scmi_api->scmi_send_message(
            scmi_message_id,
            scmi_protocol_id,
            /*
             * Token is incremented with each message sent to ease
             * debugging
             */
            scmi_sensor_req_ctx.token++,
            ctx->config->service_id,
            (const void *)&payload,
            sizeof(payload),
            true);

        if (status == FWK_SUCCESS) {
            status = FWK_PENDING;
        }
    } else {
        status = FWK_E_PARAM;
    }

    return status;
}

static int scmi_sensor_req_get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    return FWK_E_SUPPORT;
}

static struct mod_sensor_driver_api scmi_sensor_req_api = {
    .get_value = scmi_sensor_req_get_value,
    .get_info = scmi_sensor_req_get_info,
};

/*
 * SCMI module -> SCMI sensor requester module interface
 */
static int scmi_sensor_req_get_scmi_protocol_id(
    fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_SENSOR;

    return FWK_SUCCESS;
}

static int scmi_sensor_req_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    int handler_status, resp_status;

    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return FWK_E_RANGE;
    }

    if (payload_size != payload_size_table[message_id]) {
        return FWK_E_PARAM;
    }

    handler_status =
        handler_table[message_id](service_id, payload, payload_size);

    resp_status =
        scmi_sensor_req_ctx.scmi_api->response_message_handler(service_id);

    return (handler_status != FWK_SUCCESS) ? handler_status : resp_status;
}

static struct mod_scmi_to_protocol_api scmi_sensor_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_sensor_req_get_scmi_protocol_id,
    .message_handler = scmi_sensor_req_message_handler,
};

/*
 * Framework interface
 */
static int scmi_sensor_req_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    if (element_count == 0) {
        return FWK_E_DATA;
    }

    scmi_sensor_req_ctx.element_count = element_count;

    scmi_sensor_req_ctx.ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct scmi_sensor_req_elem_ctx));

    return FWK_SUCCESS;
}

static int scmi_sensor_req_elem_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct scmi_sensor_req_elem_ctx *ctx;
    const struct scmi_sensor_req_config *config;
    uint32_t element_idx = fwk_id_get_element_idx(element_id);

    fwk_assert(data != NULL);

    if (element_idx >= scmi_sensor_req_ctx.element_count) {
        return FWK_E_DATA;
    }

    config = (const struct scmi_sensor_req_config *)data;

    ctx = &(scmi_sensor_req_ctx.ctx_table[element_idx]);
    ctx->config = config;
    return FWK_SUCCESS;
}

static int scmi_sensor_req_bind(fwk_id_t id, unsigned int round)
{
    struct scmi_sensor_req_elem_ctx *ctx;
    int status = FWK_SUCCESS;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
            status = fwk_module_bind(
                FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
                FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL_REQ),
                &scmi_sensor_req_ctx.scmi_api);
        } else {
            ctx = &(scmi_sensor_req_ctx.ctx_table[fwk_id_get_element_idx(id)]);
            status = fwk_module_bind(
                ctx->config->sensor_hal_id,
                mod_sensor_api_id_driver_response,
                &scmi_sensor_req_ctx.resp_api);
        }
    }

    return status;
}

static int scmi_sensor_req_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    uint32_t source_idx = fwk_id_get_module_idx(source_id);

    fwk_assert(api != NULL);

    if (source_idx == FWK_MODULE_IDX_SCMI) {
        *api = &scmi_sensor_mod_scmi_to_protocol_api;
        return FWK_SUCCESS;
    }

    if (source_idx == FWK_MODULE_IDX_SENSOR) {
        *api = &scmi_sensor_req_api;
        return FWK_SUCCESS;
    }

    return FWK_E_ACCESS;
}

const struct fwk_module module_scmi_sensor_req = {
    .api_count = (unsigned int)2,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_sensor_req_init,
    .element_init = scmi_sensor_req_elem_init,
    .bind = scmi_sensor_req_bind,
    .process_bind_request = scmi_sensor_req_process_bind_request,
};
