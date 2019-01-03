/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Clock Management Protocol Support.
 */

#include <string.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <internal/scmi.h>
#include <internal/scmi_clock.h>
#include <mod_clock.h>
#include <mod_scmi.h>
#include <mod_scmi_clock.h>

struct scmi_clock_ctx {
    /*! SCMI Clock Module Configuration */
    const struct mod_scmi_clock_config *config;

    /*! Maximum supported number of pending, asynchronous clock rate changes */
    uint8_t max_pending_transactions;

    /*!
     * Pointer to the table of agent descriptors, used to provide per-agent
     * views of clocks in the system.
     */
    const struct mod_scmi_clock_agent *agent_table;

    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* Clock module API */
    const struct mod_clock_api *clock_api;
};

static int scmi_clock_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_clock_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_clock_protocol_message_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_clock_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_clock_rate_get_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_clock_rate_set_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_clock_config_set_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_clock_describe_rates_handler(fwk_id_t service_id,
    const uint32_t *payload);

/*
 * Internal variables.
 */
static struct scmi_clock_ctx scmi_clock_ctx;

static int (* const handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_PROTOCOL_VERSION] = scmi_clock_protocol_version_handler,
    [SCMI_PROTOCOL_ATTRIBUTES] = scmi_clock_protocol_attributes_handler,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_clock_protocol_message_attributes_handler,
    [SCMI_CLOCK_ATTRIBUTES] = scmi_clock_attributes_handler,
    [SCMI_CLOCK_RATE_GET] = scmi_clock_rate_get_handler,
    [SCMI_CLOCK_RATE_SET] = scmi_clock_rate_set_handler,
    [SCMI_CLOCK_CONFIG_SET] = scmi_clock_config_set_handler,
    [SCMI_CLOCK_DESCRIBE_RATES] = scmi_clock_describe_rates_handler,
};

static const unsigned int payload_size_table[] = {
    [SCMI_PROTOCOL_VERSION] = 0,
    [SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        sizeof(struct scmi_protocol_message_attributes_a2p),
    [SCMI_CLOCK_ATTRIBUTES] = sizeof(struct scmi_clock_attributes_a2p),
    [SCMI_CLOCK_RATE_GET] = sizeof(struct scmi_clock_rate_get_a2p),
    [SCMI_CLOCK_RATE_SET] = sizeof(struct scmi_clock_rate_set_a2p),
    [SCMI_CLOCK_CONFIG_SET] = sizeof(struct scmi_clock_config_set_a2p),
    [SCMI_CLOCK_DESCRIBE_RATES] = sizeof(struct scmi_clock_describe_rates_a2p),
};

/*
 * Given a service identifier, retrieve a pointer to its agent's
 * \c mod_scmi_clock_agent structure within the agent table.
 */
static int get_agent_entry(
    fwk_id_t service_id,
    const struct mod_scmi_clock_agent **agent)
{
    int status;
    unsigned int agent_id;

    if (agent == NULL)
        return FWK_E_PARAM;

    status = scmi_clock_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= scmi_clock_ctx.config->agent_count)
        return FWK_E_PARAM;

    *agent =
        (struct mod_scmi_clock_agent *)&scmi_clock_ctx.agent_table[agent_id];

    return FWK_SUCCESS;
}

/*
 * Given a service identifier and a clock index, retrieve a pointer to the
 * clock's \c mod_scmi_clock_device structure within the agent's device table.
 * Optionally, a pointer to the agent may be retrieved as well.
 */
static int get_clock_device_entry(
    fwk_id_t service_id,
    unsigned int clock_idx,
    const struct mod_scmi_clock_device **clock_device,
    const struct mod_scmi_clock_agent **agent)
{
    int status;
    const struct mod_scmi_clock_agent *agent_entry;

    if (clock_device == NULL)
        return FWK_E_PARAM;

    status = get_agent_entry(service_id, &agent_entry);
    if (status != FWK_SUCCESS)
        return status;

    if (clock_idx >= agent_entry->device_count)
        return FWK_E_RANGE;

    *clock_device = &agent_entry->device_table[clock_idx];

    assert((*clock_device)->permissions != MOD_SCMI_CLOCK_PERM_INVALID);
    assert(fwk_module_is_valid_element_id((*clock_device)->element_id));

    if (agent != NULL)
        *agent = agent_entry;

    return FWK_SUCCESS;
}

/*
 * Query the permissions for a service making a request to determine if
 * the operation is permitted on the given clock device.
 */
static int check_service_permission(
    const struct mod_scmi_clock_device *device,
    enum mod_scmi_clock_permissions requested_permission,
    bool *granted)
{
    if (granted == NULL)
        return FWK_E_PARAM;

    *granted = device->permissions & requested_permission;

    return FWK_SUCCESS;
}

/*
 * Protocol Version
 */
static int scmi_clock_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_CLOCK,
    };

    scmi_clock_ctx.scmi_api->respond(service_id, &return_values,
                                     sizeof(return_values));
    return FWK_SUCCESS;
}

/*
 * Protocol Attributes
 */
static int scmi_clock_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    const struct mod_scmi_clock_agent *agent;
    struct scmi_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
    };

    status = get_agent_entry(service_id, &agent);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_GENERIC_ERROR;
        goto exit;
    }

    return_values.attributes = SCMI_CLOCK_PROTOCOL_ATTRIBUTES(
            scmi_clock_ctx.max_pending_transactions,
            agent->device_count
        );

exit:
    scmi_clock_ctx.scmi_api->respond(service_id, &return_values,
                                     sizeof(return_values));
    return status;
}

/*
 * Protocol Message Attributes
 */
static int scmi_clock_protocol_message_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    size_t response_size;
    const struct scmi_protocol_message_attributes_a2p *parameters;
    unsigned int message_id;
    struct scmi_protocol_message_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
        .attributes = 0,
    };

    parameters = (const struct scmi_protocol_message_attributes_a2p*)
        payload;
    message_id = parameters->message_id;

    if ((message_id >= FWK_ARRAY_SIZE(handler_table)) ||
        (handler_table[message_id] == NULL)) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

exit:
    response_size = (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status);
    scmi_clock_ctx.scmi_api->respond(service_id, &return_values, response_size);
    return FWK_SUCCESS;
}

/*
 * Clock Attributes
 */
static int scmi_clock_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    const struct mod_scmi_clock_agent *agent;
    const struct mod_scmi_clock_device *clock_device;
    bool service_permission_granted;
    size_t response_size;
    enum mod_clock_state clock_state;
    const struct scmi_clock_attributes_a2p *parameters;
    struct scmi_clock_attributes_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };

    parameters = (const struct scmi_clock_attributes_a2p*)payload;

    status = get_clock_device_entry(service_id,
                                    parameters->clock_id,
                                    &clock_device,
                                    &agent);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = check_service_permission(clock_device,
        MOD_SCMI_CLOCK_PERM_ATTRIBUTES, &service_permission_granted);
    if (status != FWK_SUCCESS)
        goto exit;

    if (!service_permission_granted) {
        return_values.status = SCMI_DENIED;
        goto exit;
    }

    status = scmi_clock_ctx.clock_api->get_state(clock_device->element_id,
                                                 &clock_state);
    if (status != FWK_SUCCESS)
        goto exit;

    return_values = (struct scmi_clock_attributes_p2a) {
        .status = SCMI_SUCCESS,
        .attributes = SCMI_CLOCK_ATTRIBUTES(
            clock_state == MOD_CLOCK_STATE_RUNNING),
    };

    strncpy(return_values.clock_name,
            fwk_module_get_name(clock_device->element_id),
            sizeof(return_values.clock_name) - 1);

exit:
    response_size = (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status);
    scmi_clock_ctx.scmi_api->respond(service_id, &return_values, response_size);
    return status;
}

/*
 * Clock Rate Get
 */
static int scmi_clock_rate_get_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    const struct mod_scmi_clock_agent *agent;
    const struct mod_scmi_clock_device *clock_device;
    bool service_permission_granted;
    size_t response_size;
    uint64_t rate;
    const struct scmi_clock_rate_get_a2p *parameters;
    struct scmi_clock_rate_get_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };

    parameters = (const struct scmi_clock_rate_get_a2p*)payload;

    status = get_clock_device_entry(service_id,
                                    parameters->clock_id,
                                    &clock_device,
                                    &agent);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = check_service_permission(clock_device,
        MOD_SCMI_CLOCK_PERM_ATTRIBUTES, &service_permission_granted);
    if (status != FWK_SUCCESS)
        goto exit;

    if (!service_permission_granted) {
        return_values.status = SCMI_NOT_SUPPORTED;
        goto exit;
    }

    status = scmi_clock_ctx.clock_api->get_rate(
        clock_device->element_id, &rate);
    return_values.rate[0] = (uint32_t)rate;
    return_values.rate[1] = (uint32_t)(rate >> 32);

    if (status == FWK_SUCCESS)
        return_values.status = SCMI_SUCCESS;

exit:
    response_size = (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status);
    scmi_clock_ctx.scmi_api->respond(service_id, &return_values, response_size);
    return status;
}

/*
 * Clock Rate Set (Synchronous Only)
 */
static int scmi_clock_rate_set_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    const struct mod_scmi_clock_agent *agent;
    const struct mod_scmi_clock_device *clock_device;
    bool service_permission_granted;
    size_t response_size;
    uint64_t rate;
    bool round_auto;
    bool round_up;
    bool asynchronous;
    const struct scmi_clock_rate_set_a2p *parameters;
    struct scmi_clock_rate_set_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };

    parameters = (const struct scmi_clock_rate_set_a2p*)payload;
    round_up = parameters->flags & SCMI_CLOCK_RATE_SET_ROUND_UP_MASK;
    round_auto = parameters->flags & SCMI_CLOCK_RATE_SET_ROUND_AUTO_MASK;
    asynchronous = parameters->flags & SCMI_CLOCK_RATE_SET_ASYNC_MASK;
    rate = (uint64_t)parameters->rate[0] +
           (((uint64_t)parameters->rate[1]) << 32);

    status = get_clock_device_entry(service_id,
                                    parameters->clock_id,
                                    &clock_device,
                                    &agent);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = check_service_permission(clock_device,
        MOD_SCMI_CLOCK_PERM_ATTRIBUTES, &service_permission_granted);
    if (status != FWK_SUCCESS)
        goto exit;

    if (!service_permission_granted) {
        return_values.status = SCMI_NOT_SUPPORTED;
        goto exit;
    }

    if (asynchronous) {
        /* Support for async clock set commands not yet implemented */
        return_values.status = SCMI_NOT_SUPPORTED;
        goto exit;
    }

    status = scmi_clock_ctx.clock_api->set_rate(clock_device->element_id, rate,
        round_auto ? MOD_CLOCK_ROUND_MODE_NEAREST :
        (round_up ? MOD_CLOCK_ROUND_MODE_UP : MOD_CLOCK_ROUND_MODE_DOWN));
    if (status == FWK_E_RANGE) {
        return_values.status = SCMI_INVALID_PARAMETERS;
        goto exit;
    }
    if (status != FWK_SUCCESS)
        goto exit;

    return_values.status = SCMI_SUCCESS;

exit:
    response_size = (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status);
    scmi_clock_ctx.scmi_api->respond(service_id, &return_values, response_size);
    return FWK_SUCCESS;
}

/*
 * Clock Config Set
 */
static int scmi_clock_config_set_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    bool enable;
    bool service_permission_granted;
    size_t response_size;
    const struct scmi_clock_config_set_a2p *parameters;
    const struct mod_scmi_clock_agent *agent;
    const struct mod_scmi_clock_device *clock_device;
    struct scmi_clock_rate_set_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };

    parameters = (const struct scmi_clock_config_set_a2p*)payload;
    enable = parameters->attributes & SCMI_CLOCK_CONFIG_SET_ENABLE_MASK;

    status = get_clock_device_entry(service_id,
                                    parameters->clock_id,
                                    &clock_device,
                                    &agent);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = check_service_permission(clock_device,
        MOD_SCMI_CLOCK_PERM_ATTRIBUTES, &service_permission_granted);
    if (status != FWK_SUCCESS)
        goto exit;

    if (!service_permission_granted) {
        return_values.status = SCMI_NOT_SUPPORTED;
        goto exit;
    }

    status = scmi_clock_ctx.clock_api->set_state(
        clock_device->element_id,
        enable ? MOD_CLOCK_STATE_RUNNING : MOD_CLOCK_STATE_STOPPED);
    if (status == FWK_E_SUPPORT) {
        return_values.status = SCMI_NOT_SUPPORTED;
        goto exit;
    }
    if (status != FWK_SUCCESS)
        goto exit;

    return_values.status = SCMI_SUCCESS;

exit:
    response_size = (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status);
    scmi_clock_ctx.scmi_api->respond(service_id, &return_values, response_size);
    return FWK_SUCCESS;
}

/*
 * Clock Describe Rates
 */
static int scmi_clock_describe_rates_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    const struct mod_scmi_clock_agent *agent;
    const struct mod_scmi_clock_device *clock_device;
    bool service_permission_granted;
    unsigned int i;
    size_t max_payload_size;
    uint32_t payload_size;
    uint32_t index;
    unsigned int rate_count;
    unsigned int remaining_rates;
    uint64_t rate;
    struct scmi_clock_rate scmi_rate;
    struct scmi_clock_rate clock_range[3];
    struct mod_clock_info info;
    const struct scmi_clock_describe_rates_a2p *parameters;
    struct scmi_clock_describe_rates_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };

    parameters = (const struct scmi_clock_describe_rates_a2p*)payload;
    index = parameters->rate_index;
    payload_size = sizeof(return_values);

    status = get_clock_device_entry(service_id,
                                    parameters->clock_id,
                                    &clock_device,
                                    &agent);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = check_service_permission(clock_device,
        MOD_SCMI_CLOCK_PERM_ATTRIBUTES, &service_permission_granted);
    if (status != FWK_SUCCESS)
        goto exit;

    if (!service_permission_granted) {
        return_values.status = SCMI_NOT_SUPPORTED;
        goto exit;
    }

    /*
     * Get the maximum payload size to determine how many clock rate entries can
     * be returned in one response.
     */
    status = scmi_clock_ctx.scmi_api->get_max_payload_size(
        service_id, &max_payload_size);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_clock_ctx.clock_api->get_info(clock_device->element_id,
                                                &info);
    if (status != FWK_SUCCESS)
        goto exit;

    if (info.range.rate_type == MOD_CLOCK_RATE_TYPE_DISCRETE) {
        /* The clock has a discrete list of frequencies */

        if (index >= info.range.rate_count) {
            return_values.status = SCMI_INVALID_PARAMETERS;
            goto exit;
        }

        /* Can at least one entry be returned? */
        if (SCMI_CLOCK_RATES_MAX(max_payload_size) == 0) {
            status = FWK_E_SIZE;
            goto exit;
        }

        /* The number of rates being returned in this payload is defined as the
         * smaller of:
         * - The clock rates that are available between the index and the
             clock's maximum rate.
           - The number of rates that can be returned in each payload.
         */
        rate_count = FWK_MIN(SCMI_CLOCK_RATES_MAX(max_payload_size),
            info.range.rate_count - index);

        /*
         * Because the agent gives a starting index into the clock's rate list
         * the number of rates remaining is calculated as the number of rates
         * the clock supports minus the index, with the number of rates being
         * returned in this payload subtracted.
         */
        remaining_rates = (info.range.rate_count - index) - rate_count;

        /* Give the number of rates sent in the message payload */
        return_values.num_rates_flags =
            SCMI_CLOCK_DESCRIBE_RATES_NUM_RATES_FLAGS(
                rate_count,
                SCMI_CLOCK_RATE_FORMAT_LIST,
                remaining_rates
            );

        /* Set each rate entry in the payload to the associated frequency */
        for (i = 0; i < rate_count; i++,
                payload_size += sizeof(struct scmi_clock_rate)) {
            status = scmi_clock_ctx.clock_api->get_rate_from_index(
                clock_device->element_id,
                index + i,
                &rate);
            if (status != FWK_SUCCESS)
                goto exit;

            scmi_rate.low = (uint32_t)rate;
            scmi_rate.high = (uint32_t)(rate >> 32);

            status = scmi_clock_ctx.scmi_api->write_payload(service_id,
                payload_size, &scmi_rate, sizeof(scmi_rate));
            if (status != FWK_SUCCESS)
                goto exit;
        }
    } else {
        /* The clock has a linear stepping */

        /* Is the payload area large enough to return the complete triplet? */
        if (SCMI_CLOCK_RATES_MAX(max_payload_size) < 3) {
            status = FWK_E_SIZE;
            goto exit;
        }

        return_values.num_rates_flags =
            SCMI_CLOCK_DESCRIBE_RATES_NUM_RATES_FLAGS(
                /* Only a single rate is returned */
                1,
                SCMI_CLOCK_RATE_FORMAT_RANGE,
                /* No further rates are available */
                0
            );

        /* Store the range data in the range entry in the payload */
        clock_range[0].low = (uint32_t)info.range.min;
        clock_range[0].high = (uint32_t)(info.range.min >> 32);
        clock_range[1].low = (uint32_t)info.range.max;
        clock_range[1].high = (uint32_t)(info.range.max >> 32);
        clock_range[2].low = (uint32_t)info.range.step;
        clock_range[2].high = (uint32_t)(info.range.step >> 32);

        status = scmi_clock_ctx.scmi_api->write_payload(service_id,
            payload_size, &clock_range, sizeof(clock_range));
        if (status != FWK_SUCCESS)
            goto exit;
        payload_size += sizeof(clock_range);
    }

    return_values.status = SCMI_SUCCESS;
    status = scmi_clock_ctx.scmi_api->write_payload(service_id, 0,
        &return_values, sizeof(return_values));

exit:
    scmi_clock_ctx.scmi_api->respond(service_id,
        (return_values.status == SCMI_SUCCESS) ?
            NULL : &return_values.status,
        (return_values.status == SCMI_SUCCESS) ?
            payload_size : sizeof(return_values.status));
    return status;
}

/*
 * SCMI module -> SCMI clock module interface
 */
static int scmi_clock_get_scmi_protocol_id(fwk_id_t protocol_id,
                                           uint8_t *scmi_protocol_id)
{
    int status;

    status = fwk_module_check_call(protocol_id);
    if (status != FWK_SUCCESS)
        return status;

    *scmi_protocol_id = SCMI_PROTOCOL_ID_CLOCK;

    return FWK_SUCCESS;
}

static int scmi_clock_message_handler(fwk_id_t protocol_id, fwk_id_t service_id,
    const uint32_t *payload, size_t payload_size, unsigned int message_id)
{
    int status;
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Clock management protocol table sizes not consistent");
    assert(payload != NULL);

    status = fwk_module_check_call(protocol_id);
    if (status != FWK_SUCCESS)
        return status;

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return_value = SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

    return handler_table[message_id](service_id, payload);

error:
    scmi_clock_ctx.scmi_api->respond(service_id,
                                     &return_value, sizeof(return_value));
    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api scmi_clock_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_clock_get_scmi_protocol_id,
    .message_handler = scmi_clock_message_handler
};

/*
 * Framework handlers
 */

static int scmi_clock_init(fwk_id_t module_id, unsigned int element_count,
                           const void *data)
{
    const struct mod_scmi_clock_config *config =
        (const struct mod_scmi_clock_config *)data;

    if ((config == NULL) || (config->agent_table == NULL))
        return FWK_E_PARAM;

    scmi_clock_ctx.config = config;
    scmi_clock_ctx.max_pending_transactions = config->max_pending_transactions;
    scmi_clock_ctx.agent_table = config->agent_table;

    return FWK_SUCCESS;
}

static int scmi_clock_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_clock_ctx.scmi_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_CLOCK),
        FWK_ID_API(FWK_MODULE_IDX_CLOCK, 0), &scmi_clock_ctx.clock_api);
}

static int scmi_clock_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    *api = &scmi_clock_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

/* SCMI Clock Management Protocol Definition */
const struct fwk_module module_scmi_clock = {
    .name = "SCMI Clock Management Protocol",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_clock_init,
    .bind = scmi_clock_bind,
    .process_bind_request = scmi_clock_process_bind_request,
};
