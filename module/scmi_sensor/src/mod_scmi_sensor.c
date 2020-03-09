/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI sensor management protocol support.
 */

#include <internal/scmi.h>
#include <internal/scmi_sensor.h>

#include <mod_scmi.h>
#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct sensor_operations {
    /*
     * Service identifier currently requesting operation from this sensor.
     * A 'none' value means that there is no pending request.
     */
    fwk_id_t service_id;
};

struct scmi_sensor_ctx {
    /* Number of sensors */
    unsigned int sensor_count;

    /* SCMI protocol module to SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* Sensor module API */
    const struct mod_sensor_api *sensor_api;

    /* Pointer to a table of sensor operations */
    struct sensor_operations *sensor_ops_table;
};

static int scmi_sensor_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_sensor_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_sensor_protocol_msg_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_sensor_protocol_desc_get_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_sensor_reading_get_handler(fwk_id_t service_id,
    const uint32_t *payload);

struct scmi_sensor_event_parameters {
    fwk_id_t sensor_id;
};

static const fwk_id_t mod_scmi_sensor_event_id_get_request =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SCMI_SENSOR,
                      SCMI_SENSOR_EVENT_IDX_REQUEST);

/*
 * Internal variables.
 */
static struct scmi_sensor_ctx scmi_sensor_ctx;

static int (*handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_PROTOCOL_VERSION] =
                       scmi_sensor_protocol_version_handler,
    [SCMI_PROTOCOL_ATTRIBUTES] =
                       scmi_sensor_protocol_attributes_handler,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
                       scmi_sensor_protocol_msg_attributes_handler,
    [SCMI_SENSOR_DESCRIPTION_GET] =
                       scmi_sensor_protocol_desc_get_handler,
    [SCMI_SENSOR_READING_GET] = scmi_sensor_reading_get_handler
};

static unsigned int payload_size_table[] = {
    [SCMI_PROTOCOL_VERSION] = 0,
    [SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
                       sizeof(struct scmi_protocol_message_attributes_a2p),
    [SCMI_SENSOR_DESCRIPTION_GET] =
                       sizeof(struct scmi_sensor_protocol_description_get_a2p),
    [SCMI_SENSOR_READING_GET] =
                       sizeof(struct scmi_sensor_protocol_reading_get_a2p),
};

/*
 * Static helper for responding to SCMI.
 */
static void scmi_sensor_respond(
    struct scmi_sensor_protocol_reading_get_p2a *return_values,
    fwk_id_t sensor_id)
{
    unsigned int sensor_idx;
    fwk_id_t service_id;

    /*
     * The service identifier used for the response is retrieved from the
     * sensor operations table.
     */
    sensor_idx = fwk_id_get_element_idx(sensor_id);
    service_id = scmi_sensor_ctx.sensor_ops_table[sensor_idx].service_id;

    scmi_sensor_ctx.scmi_api->respond(service_id,
        return_values,
        (return_values->status == SCMI_SUCCESS) ?
        sizeof(*return_values) : sizeof(return_values->status));

    /*
     * Set the service identifier to 'none' to indicate the sensor is
     * available again.
     */
    scmi_sensor_ctx.sensor_ops_table[sensor_idx].service_id = FWK_ID_NONE;
}

/*
 * Sensor management protocol implementation
 */
static int scmi_sensor_protocol_version_handler(fwk_id_t service_id,
                                                const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_SENSOR,
    };

    scmi_sensor_ctx.scmi_api->respond(service_id, &return_values,
                                      sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_sensor_protocol_attributes_handler(fwk_id_t service_id,
                                                   const uint32_t *payload)
{
    struct scmi_sensor_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
        .attributes = scmi_sensor_ctx.sensor_count,
        .sensor_reg_len = 0, /* Unsupported */
    };

    scmi_sensor_ctx.scmi_api->respond(service_id, &return_values,
                                      sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_sensor_protocol_msg_attributes_handler(fwk_id_t service_id,
                                                       const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *parameters;
    struct scmi_protocol_message_attributes_p2a return_values;

    parameters = (const struct scmi_protocol_message_attributes_a2p *)
                 payload;

    if ((parameters->message_id < FWK_ARRAY_SIZE(handler_table)) &&
        (handler_table[parameters->message_id] != NULL)) {
        return_values = (struct scmi_protocol_message_attributes_p2a) {
            .status = SCMI_SUCCESS,
            /* All commands have an attributes value of 0 */
            .attributes = 0,
        };
    } else
        return_values.status = SCMI_NOT_FOUND;

    scmi_sensor_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return FWK_SUCCESS;
}

static int scmi_sensor_protocol_desc_get_handler(fwk_id_t service_id,
                                                 const uint32_t *payload)
{
    int status;
    size_t payload_size;
    size_t max_payload_size;
    const struct scmi_sensor_protocol_description_get_a2p *parameters =
               (const struct scmi_sensor_protocol_description_get_a2p *)payload;
    struct scmi_sensor_desc desc = { 0 };
    unsigned int num_descs, desc_index, desc_index_max;
    struct mod_sensor_info sensor_info;
    struct scmi_sensor_protocol_description_get_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };
    fwk_id_t sensor_id;

    payload_size = sizeof(return_values);

    status = scmi_sensor_ctx.scmi_api->get_max_payload_size(service_id,
                                                            &max_payload_size);
    if (status != FWK_SUCCESS)
        goto exit;

    if (SCMI_SENSOR_DESCS_MAX(max_payload_size) == 0) {
        /* Can't even fit one sensor description in the payload */
        assert(false);
        status = FWK_E_SIZE;
        goto exit;
    }

    parameters =
        (const struct scmi_sensor_protocol_description_get_a2p *)payload;
    desc_index = parameters->desc_index;

    if (desc_index >= scmi_sensor_ctx.sensor_count) {
        return_values.status = SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    num_descs = FWK_MIN(SCMI_SENSOR_DESCS_MAX(max_payload_size),
        (scmi_sensor_ctx.sensor_count - desc_index));
    desc_index_max = (desc_index + num_descs - 1);

    for (; desc_index <= desc_index_max; ++desc_index,
         payload_size += sizeof(desc)) {

        desc = (struct scmi_sensor_desc) {
            .sensor_id = desc_index,
            .sensor_attributes_low = 0, /* None supported */
        };

        sensor_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, desc_index);
        if (!fwk_module_is_valid_element_id(sensor_id)) {
            /* domain_idx did not map to a sensor device */
            assert(false);
            return_values.status = SCMI_NOT_FOUND;
            goto exit;
        }

        status = scmi_sensor_ctx.sensor_api->get_info(sensor_id, &sensor_info);
        if (status != FWK_SUCCESS) {
            /* Unable to get sensor info */
            assert(false);
            goto exit;
        }

        if (sensor_info.type >= MOD_SENSOR_TYPE_COUNT) {
            /* Invalid sensor type */
            assert(false);
            goto exit;
        }

        if ((sensor_info.unit_multiplier <
             SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MIN) ||
            (sensor_info.unit_multiplier >
             SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UNIT_MULTIPLIER_MAX)) {

            /* Sensor unit multiplier out of range */
            assert(false);
            goto exit;
        }

        if ((sensor_info.update_interval_multiplier <
             SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MIN) ||
            (sensor_info.update_interval_multiplier >
             SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_MULTIPLIER_MAX)) {

            /* Sensor update interval multiplier is out of range */
            assert(false);
            goto exit;
        }

        if (sensor_info.update_interval >=
            SCMI_SENSOR_DESC_ATTRS_HIGH_SENSOR_UPDATE_INTERVAL_MASK) {

            /* Update interval is too big to fit in its mask */
            assert(false);
            goto exit;
        }

        desc.sensor_attributes_high =
            SCMI_SENSOR_DESC_ATTRIBUTES_HIGH(sensor_info.type,
                sensor_info.unit_multiplier,
                (uint32_t)sensor_info.update_interval_multiplier,
                (uint32_t)sensor_info.update_interval);

        /*
         * Copy sensor name into description struct. Copy n-1 chars to ensure a
         * NULL terminator at the end. (struct has been zeroed out)
         */
        strncpy(desc.sensor_name,
                fwk_module_get_name(sensor_id),
                sizeof(desc.sensor_name) - 1);

        status = scmi_sensor_ctx.scmi_api->write_payload(service_id,
            payload_size, &desc, sizeof(struct scmi_sensor_desc));
        if (status != FWK_SUCCESS) {
            /* Failed to write sensor description into message payload */
            assert(false);
            goto exit;
        }
    }

    return_values = (struct scmi_sensor_protocol_description_get_p2a) {
        .status = SCMI_SUCCESS,
        .num_sensor_flags = SCMI_SENSOR_NUM_SENSOR_FLAGS(num_descs,
            (scmi_sensor_ctx.sensor_count - desc_index_max - 1))
    };

    status = scmi_sensor_ctx.scmi_api->write_payload(service_id, 0,
        &return_values, sizeof(return_values));
    if (status != FWK_SUCCESS)
        return_values.status = SCMI_GENERIC_ERROR;

exit:
    scmi_sensor_ctx.scmi_api->respond(service_id,
        (return_values.status == SCMI_SUCCESS) ?
            NULL : &return_values.status,
        (return_values.status == SCMI_SUCCESS) ?
            payload_size : sizeof(return_values.status));

    return status;
}

static int scmi_sensor_reading_get_handler(fwk_id_t service_id,
                                           const uint32_t *payload)
{
    const struct scmi_sensor_protocol_reading_get_a2p *parameters;
    struct scmi_sensor_protocol_reading_get_p2a return_values;
    struct scmi_sensor_event_parameters *params;
    unsigned int sensor_idx;
    uint32_t flags;
    int status;

    parameters = (const struct scmi_sensor_protocol_reading_get_a2p *)payload;
    return_values.status = SCMI_GENERIC_ERROR;

    if (parameters->sensor_id >= scmi_sensor_ctx.sensor_count) {
        /* Sensor does not exist */
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    /* Reject asynchronous read requests for now */
    flags = parameters->flags;
    if (flags & SCMI_SENSOR_PROTOCOL_READING_GET_ASYNC_FLAG_MASK) {
        return_values.status = SCMI_NOT_SUPPORTED;
        status = FWK_SUCCESS;
        goto exit;
    }

    sensor_idx = parameters->sensor_id;

    /* Check if there is already a request pending for this sensor */
    if (!fwk_id_is_equal(
            scmi_sensor_ctx.sensor_ops_table[sensor_idx].service_id,
            FWK_ID_NONE)){
        return_values.status = SCMI_BUSY;
        status = FWK_SUCCESS;

        goto exit;
    }

    /* The get_value request is processed within the event being generated */
    struct fwk_event event = {
        .target_id = fwk_module_id_scmi_sensor,
        .id = mod_scmi_sensor_event_id_get_request,
    };

    params = (struct scmi_sensor_event_parameters *)event.params;
    params->sensor_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR,
                                       sensor_idx);

    status = fwk_thread_put_event(&event);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_GENERIC_ERROR;

        goto exit;
    }

    /* Store service identifier to indicate there is a pending request */
    scmi_sensor_ctx.sensor_ops_table[sensor_idx].service_id = service_id;

    return FWK_SUCCESS;

exit:
    scmi_sensor_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

/*
 * SCMI module -> SCMI sensor module interface
 */
static int scmi_sensor_get_scmi_protocol_id(fwk_id_t protocol_id,
                                            uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = SCMI_PROTOCOL_ID_SENSOR;

    return FWK_SUCCESS;
}

static int scmi_sensor_message_handler(fwk_id_t protocol_id,
                                       fwk_id_t service_id,
                                       const uint32_t *payload,
                                       size_t payload_size,
                                       unsigned int message_id)
{
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Sensor management protocol table sizes not consistent");
    assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return_value = SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        /* Incorrect payload size or message is not supported */
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

    return handler_table[message_id](service_id, payload);

error:
    scmi_sensor_ctx.scmi_api->respond(service_id, &return_value,
                                      sizeof(return_value));
    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api scmi_sensor_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_sensor_get_scmi_protocol_id,
    .message_handler = scmi_sensor_message_handler
};

/*
 * Framework interface
 */
static int scmi_sensor_init(fwk_id_t module_id,
                            unsigned int element_count,
                            const void *unused)
{
    if (element_count != 0) {
        /* This module should not have any elements */
        assert(false);
        return FWK_E_SUPPORT;
    }

    scmi_sensor_ctx.sensor_count = fwk_module_get_element_count(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR));
    if (scmi_sensor_ctx.sensor_count == 0)
        return FWK_E_SUPPORT;

    /* SCMI protocol uses a 16 bit number to store the number of sensors.
     * So expose no more than 0xFFFF number of sensors. */
    if (scmi_sensor_ctx.sensor_count > UINT16_MAX)
        scmi_sensor_ctx.sensor_count = UINT16_MAX;

    /* Allocate a table for the sensors state */
    scmi_sensor_ctx.sensor_ops_table =
        fwk_mm_calloc(scmi_sensor_ctx.sensor_count,
        sizeof(struct sensor_operations));

    /* Initialize the service identifier for each sensor to 'available' */
    for (unsigned int i = 0; i < scmi_sensor_ctx.sensor_count; i++)
        scmi_sensor_ctx.sensor_ops_table[i].service_id = FWK_ID_NONE;

    return FWK_SUCCESS;
}

static int scmi_sensor_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
                             FWK_ID_API(FWK_MODULE_IDX_SCMI,
                                        MOD_SCMI_API_IDX_PROTOCOL),
                             &scmi_sensor_ctx.scmi_api);
    if (status != FWK_SUCCESS) {
        /* Failed to bind to SCMI module */
        assert(false);
        return status;
    }

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR),
                             mod_sensor_api_id_sensor,
                             &scmi_sensor_ctx.sensor_api);
    if (status != FWK_SUCCESS) {
        /* Failed to bind to sensor module */
        assert(false);
        return status;
    }

    return FWK_SUCCESS;
}

static int scmi_sensor_process_bind_request(fwk_id_t source_id,
                                            fwk_id_t target_id,
                                            fwk_id_t api_id,
                                            const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    *api = &scmi_sensor_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

static int scmi_sensor_process_event(const struct fwk_event *event,
                                     struct fwk_event *resp_event)
{
    int status;
    uint64_t sensor_value;
    struct scmi_sensor_event_parameters *params;
    struct scmi_sensor_protocol_reading_get_p2a return_values;

    /* Request event to sensor HAL */
    if (fwk_id_is_equal(event->id, mod_scmi_sensor_event_id_get_request)) {
        params = (struct scmi_sensor_event_parameters *)event->params;

        status = scmi_sensor_ctx.sensor_api->get_value(params->sensor_id,
                                                       &sensor_value);
        if (status == FWK_SUCCESS) {
            /* Sensor value is ready */
            return_values = (struct scmi_sensor_protocol_reading_get_p2a) {
                .status = SCMI_SUCCESS,
                .sensor_value_low = (uint32_t)sensor_value,
                .sensor_value_high = (uint32_t)(sensor_value >> 32),
            };

            scmi_sensor_respond(&return_values, params->sensor_id);

            return status;
        } else if (status == FWK_PENDING) {
            /* Sensor value will be provided through a response event */
            return FWK_SUCCESS;
        } else {
            return_values = (struct scmi_sensor_protocol_reading_get_p2a) {
                .status = SCMI_HARDWARE_ERROR,
            };

            scmi_sensor_respond(&return_values, params->sensor_id);

            return FWK_E_PANIC;
        }
    }

    /* Response event from sensor HAL */
    if (fwk_id_is_equal(event->id, mod_sensor_event_id_read_request)) {
        struct mod_sensor_event_params *params =
            (struct mod_sensor_event_params *)event->params;

        return_values = (struct scmi_sensor_protocol_reading_get_p2a) {
            .sensor_value_low = (uint32_t)params->value,
            .sensor_value_high = (uint32_t)(params->value >> 32),
        };

        if (params->status == FWK_SUCCESS)
            return_values.status = SCMI_SUCCESS;
        else
            return_values.status = SCMI_HARDWARE_ERROR;

        scmi_sensor_respond(&return_values, event->source_id);
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_scmi_sensor = {
    .name = "SCMI sensor management",
    .api_count = 1,
    .event_count = SCMI_SENSOR_EVENT_IDX_COUNT,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_sensor_init,
    .bind = scmi_sensor_bind,
    .process_bind_request = scmi_sensor_process_bind_request,
    .process_event = scmi_sensor_process_event,
};

/* No elements, no module configuration data */
struct fwk_module_config config_scmi_sensor = { 0 };
