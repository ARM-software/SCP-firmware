/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI performance domain management protocol support.
 */

#include <internal/scmi.h>
#include <internal/scmi_perf.h>

#include <mod_dvfs.h>
#include <mod_scmi.h>
#include <mod_scmi_perf.h>

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
#include <stddef.h>
#include <string.h>

static int scmi_perf_protocol_version_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_perf_protocol_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_perf_protocol_message_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_perf_domain_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_perf_describe_levels_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_perf_level_set_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_perf_level_get_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_perf_limits_set_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_perf_limits_get_handler(
    fwk_id_t service_id, const uint32_t *payload);

static int (*handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_PROTOCOL_VERSION] =
                       scmi_perf_protocol_version_handler,
    [SCMI_PROTOCOL_ATTRIBUTES] =
                       scmi_perf_protocol_attributes_handler,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
                       scmi_perf_protocol_message_attributes_handler,
    [SCMI_PERF_DOMAIN_ATTRIBUTES] =
                       scmi_perf_domain_attributes_handler,
    [SCMI_PERF_DESCRIBE_LEVELS] =
                       scmi_perf_describe_levels_handler,
    [SCMI_PERF_LIMITS_SET] =
                       scmi_perf_limits_set_handler,
    [SCMI_PERF_LIMITS_GET] =
                       scmi_perf_limits_get_handler,
    [SCMI_PERF_LEVEL_SET] =
                       scmi_perf_level_set_handler,
    [SCMI_PERF_LEVEL_GET] =
                       scmi_perf_level_get_handler
};

static unsigned int payload_size_table[] = {
    [SCMI_PROTOCOL_VERSION] = 0,
    [SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
                       sizeof(struct scmi_protocol_message_attributes_a2p),
    [SCMI_PERF_DOMAIN_ATTRIBUTES] =
                       sizeof(struct scmi_perf_domain_attributes_a2p),
    [SCMI_PERF_DESCRIBE_LEVELS] =
                       sizeof(struct scmi_perf_describe_levels_a2p),
    [SCMI_PERF_LEVEL_SET] =
                       sizeof(struct scmi_perf_level_set_a2p),
    [SCMI_PERF_LEVEL_GET] =
                       sizeof(struct scmi_perf_level_get_a2p),
    [SCMI_PERF_LIMITS_SET] =
                       sizeof(struct scmi_perf_limits_set_a2p),
    [SCMI_PERF_LIMITS_GET] =
                       sizeof(struct scmi_perf_limits_get_a2p),
};

struct perf_operations {
    /*
     * Service identifier currently requesting operation.
     * A 'none' value means that there is no pending request.
     */
    fwk_id_t service_id;
};


struct scmi_perf_ctx {
    /* SCMI Performance Module Configuration */
    const struct mod_scmi_perf_config *config;

    /* Number of power domains */
    uint32_t domain_count;

    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* DVFS module API */
    const struct mod_dvfs_domain_api *dvfs_api;

    /* Pointer to a table of operations */
    struct perf_operations *perf_ops_table;
};

static struct scmi_perf_ctx scmi_perf_ctx;

/* Event indices */
enum scmi_perf_event_idx {
    SCMI_PERF_EVENT_IDX_LEVEL_GET_REQUEST,
    SCMI_PERF_EVENT_IDX_LIMITS_GET_REQUEST,
    SCMI_PERF_EVENT_IDX_COUNT,
};

struct scmi_perf_event_parameters {
    fwk_id_t domain_id;
};

static const fwk_id_t scmi_perf_get_level =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SCMI_PERF,
                      SCMI_PERF_EVENT_IDX_LEVEL_GET_REQUEST);

static const fwk_id_t scmi_perf_get_limits =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SCMI_PERF,
                      SCMI_PERF_EVENT_IDX_LIMITS_GET_REQUEST);

/*
 * Protocol command handlers
 */

static int scmi_perf_protocol_version_handler(fwk_id_t service_id,
                                              const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_PERF,
    };

    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
                                    sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_perf_protocol_attributes_handler(fwk_id_t service_id,
                                                 const uint32_t *payload)
{
    struct scmi_perf_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
        .attributes =
            SCMI_PERF_PROTOCOL_ATTRIBUTES(true, scmi_perf_ctx.domain_count),
        .statistics_len = 0, /* Unsupported */
        .statistics_address_low = 0, /* Unsupported */
        .statistics_address_high = 0, /* Unsupported */
    };

    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
                                    sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_perf_protocol_message_attributes_handler(fwk_id_t service_id,
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
            .attributes = 0, /* All commands have an attributes value of 0 */
        };
    } else
        return_values.status = SCMI_NOT_FOUND;

    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return FWK_SUCCESS;
}

static int scmi_perf_domain_attributes_handler(fwk_id_t service_id,
                                               const uint32_t *payload)
{
    int status;
    unsigned int agent_id;
    const struct mod_scmi_perf_domain_config *domain;
    const struct scmi_perf_domain_attributes_a2p *parameters;
    uint32_t permissions;
    fwk_id_t domain_id;
    struct mod_dvfs_opp opp;
    struct scmi_perf_domain_attributes_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_perf_domain_attributes_a2p *)payload;

    /* Validate the domain identifier */
    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    domain = &(*scmi_perf_ctx.config->domains)[parameters->domain_id];
    permissions = (*domain->permissions)[agent_id];

    domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id);
    status = scmi_perf_ctx.dvfs_api->get_sustained_opp(domain_id, &opp);
    if (status != FWK_SUCCESS)
        goto exit;

    return_values = (struct scmi_perf_domain_attributes_p2a) {
        .status = SCMI_SUCCESS,
        .attributes = SCMI_PERF_DOMAIN_ATTRIBUTES(
            false, false,
            !!(permissions & MOD_SCMI_PERF_PERMS_SET_LEVEL),
            !!(permissions & MOD_SCMI_PERF_PERMS_SET_LIMITS)
        ),
        .rate_limit = 0, /* Unsupported */
        .sustained_freq = opp.frequency / FWK_KHZ,
        .sustained_perf_level = opp.frequency,
    };

    /* Copy the domain name into the mailbox */
    strncpy((char *)return_values.name,
        fwk_module_get_name(domain_id), sizeof(return_values.name) - 1);

exit:
    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_perf_describe_levels_handler(fwk_id_t service_id,
                                             const uint32_t *payload)
{
    int status;
    size_t max_payload_size;
    const struct scmi_perf_describe_levels_a2p *parameters;
    fwk_id_t domain_id;
    struct scmi_perf_level perf_level;
    unsigned int num_levels, level_index, level_index_max;
    size_t payload_size;
    size_t opp_count;
    struct mod_dvfs_opp opp;
    uint16_t latency;
    struct scmi_perf_describe_levels_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    payload_size = sizeof(return_values);

    status = scmi_perf_ctx.scmi_api->get_max_payload_size(service_id,
                                                          &max_payload_size);
    if (status != FWK_SUCCESS)
        goto exit;

    status = (SCMI_PERF_LEVELS_MAX(max_payload_size) > 0) ?
        FWK_SUCCESS : FWK_E_SIZE;
    if (status != FWK_SUCCESS)
        goto exit;

    parameters = (const struct scmi_perf_describe_levels_a2p *)payload;

    /* Validate the domain identifier */
    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    /* Get the number of operating points for the domain */
    domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id);
    status = scmi_perf_ctx.dvfs_api->get_opp_count(domain_id, &opp_count);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Validate level index */
    level_index = parameters->level_index;
    if (level_index >= opp_count) {
        return_values.status = SCMI_INVALID_PARAMETERS;

        goto exit;
    }

    /* Identify the maximum number of performance levels we can send at once */
    num_levels =
        (SCMI_PERF_LEVELS_MAX(max_payload_size) <
            (opp_count - level_index)) ?
        SCMI_PERF_LEVELS_MAX(max_payload_size) :
            (opp_count - level_index);
    level_index_max = (level_index + num_levels - 1);

    status = scmi_perf_ctx.dvfs_api->get_latency(domain_id, &latency);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Copy DVFS data into returned data structure */
    for (; level_index <= level_index_max; level_index++,
         payload_size += sizeof(perf_level)) {

        status = scmi_perf_ctx.dvfs_api->get_nth_opp(
            domain_id, level_index, &opp);
        if (status != FWK_SUCCESS)
            goto exit;

        if (opp.power != 0)
            perf_level.power_cost = opp.power;
        else
            perf_level.power_cost = opp.voltage;
        perf_level.performance_level = opp.frequency;
        perf_level.attributes = latency;

        status = scmi_perf_ctx.scmi_api->write_payload(service_id, payload_size,
            &perf_level, sizeof(perf_level));
        if (status != FWK_SUCCESS)
            goto exit;
    }

    return_values = (struct scmi_perf_describe_levels_p2a) {
        .status = SCMI_SUCCESS,
        .num_levels = SCMI_PERF_NUM_LEVELS(num_levels,
            (opp_count - level_index_max - 1))
    };

    status = scmi_perf_ctx.scmi_api->write_payload(service_id, 0,
        &return_values, sizeof(return_values));

exit:
    scmi_perf_ctx.scmi_api->respond(service_id,
        (return_values.status == SCMI_SUCCESS) ?
            NULL : &return_values.status,
        (return_values.status == SCMI_SUCCESS) ?
            payload_size : sizeof(return_values.status));

    return status;
}

static int scmi_perf_limits_set_handler(fwk_id_t service_id,
                                        const uint32_t *payload)
{
    int status;
    unsigned int agent_id;
    const struct mod_scmi_perf_domain_config *domain;
    const struct scmi_perf_limits_set_a2p *parameters;
    uint32_t permissions;
    struct scmi_perf_limits_set_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_perf_limits_set_a2p *)payload;

    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Ensure the agent has permission to do this */
    domain = &(*scmi_perf_ctx.config->domains)[parameters->domain_id];
    permissions = (*domain->permissions)[agent_id];
    if (!(permissions & MOD_SCMI_PERF_PERMS_SET_LIMITS)) {
        return_values.status = SCMI_DENIED;

        goto exit;
    }

    if (parameters->range_min > parameters->range_max) {
        return_values.status = SCMI_INVALID_PARAMETERS;

        goto exit;
    }

    status = scmi_perf_ctx.dvfs_api->set_frequency_limits(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id),
        &((struct mod_dvfs_frequency_limits) {
        .minimum = parameters->range_min,
        .maximum = parameters->range_max
        }));

    /*
     * Return immediately to the caller, fire-and-forget.
     */

    if ((status == FWK_SUCCESS) || (status == FWK_PENDING))
        return_values.status = SCMI_SUCCESS;
    else
        return_values.status = SCMI_OUT_OF_RANGE;

exit:
    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_perf_limits_get_handler(fwk_id_t service_id,
                                        const uint32_t *payload)
{
    int status;
    fwk_id_t domain_id;
    const struct scmi_perf_limits_get_a2p *parameters;
    struct scmi_perf_event_parameters *evt_params;
    struct scmi_perf_limits_get_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_perf_limits_get_a2p *)payload;
    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id);

    /* Check if there is already a request pending for this domain */
    if (!fwk_id_is_equal(
            scmi_perf_ctx.perf_ops_table[parameters->domain_id].service_id,
            FWK_ID_NONE)){
        return_values.status = SCMI_BUSY;
        status = FWK_SUCCESS;

        goto exit;
    }

    /* The get_limits request is processed within the event being generated */
    struct fwk_event event = {
        .target_id = fwk_module_id_scmi_perf,
        .id = scmi_perf_get_limits,
    };

    evt_params = (struct scmi_perf_event_parameters *)event.params;
    evt_params->domain_id = domain_id;

    status = fwk_thread_put_event(&event);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_GENERIC_ERROR;

        goto exit;
    }

    /* Store service identifier to indicate there is a pending request */
    scmi_perf_ctx.perf_ops_table[parameters->domain_id].service_id = service_id;

    return FWK_SUCCESS;

exit:
    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        sizeof(return_values.status));

    return status;
}

static int scmi_perf_level_set_handler(fwk_id_t service_id,
                                       const uint32_t *payload)
{
    int status;
    unsigned int agent_id;
    const struct mod_scmi_perf_domain_config *domain;
    const struct scmi_perf_level_set_a2p *parameters;
    uint32_t permissions;
    struct scmi_perf_level_set_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_perf_level_set_a2p *)payload;

    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Ensure the agent has permission to do this */
    domain = &(*scmi_perf_ctx.config->domains)[parameters->domain_id];
    permissions = (*domain->permissions)[agent_id];
    if (!(permissions & MOD_SCMI_PERF_PERMS_SET_LEVEL)) {
        return_values.status = SCMI_DENIED;

        goto exit;
    }

    status = scmi_perf_ctx.dvfs_api->set_frequency(
       FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id),
       (uint64_t)parameters->performance_level);

    /*
     * Return immediately to the caller, fire-and-forget.
     */
    if ((status == FWK_SUCCESS) || (status == FWK_PENDING))
        return_values.status = SCMI_SUCCESS;
    else if (status == FWK_E_RANGE)
        return_values.status = SCMI_OUT_OF_RANGE;

exit:
    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_perf_level_get_handler(fwk_id_t service_id,
                                       const uint32_t *payload)
{
    int status;
    const struct scmi_perf_level_get_a2p *parameters;
    struct scmi_perf_event_parameters *evt_params;
    struct scmi_perf_level_get_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    return_values.status = SCMI_GENERIC_ERROR;

    parameters = (const struct scmi_perf_level_get_a2p *)payload;
    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    /* Check if there is already a request pending for this domain */
    if (!fwk_id_is_equal(
            scmi_perf_ctx.perf_ops_table[parameters->domain_id].service_id,
            FWK_ID_NONE)){
        return_values.status = SCMI_BUSY;
        status = FWK_SUCCESS;

        goto exit;
    }

    /* The get_level request is processed within the event being generated */
    struct fwk_event event = {
        .target_id = fwk_module_id_scmi_perf,
        .id = scmi_perf_get_level,
    };

    evt_params = (struct scmi_perf_event_parameters *)event.params;
    evt_params->domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS,
        parameters->domain_id);

    status = fwk_thread_put_event(&event);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_GENERIC_ERROR;

        goto exit;
    }

    /* Store service identifier to indicate there is a pending request */
    scmi_perf_ctx.perf_ops_table[parameters->domain_id].service_id = service_id;

    return FWK_SUCCESS;

exit:
    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

/*
 * SCMI module -> SCMI performance module interface
 */

static int scmi_perf_get_scmi_protocol_id(fwk_id_t protocol_id,
                                          uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = SCMI_PROTOCOL_ID_PERF;

    return FWK_SUCCESS;
}

static int scmi_perf_message_handler(fwk_id_t protocol_id, fwk_id_t service_id,
    const uint32_t *payload, size_t payload_size, unsigned int message_id)
{
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Performance management protocol table sizes not consistent");
    assert(payload != NULL);

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
    scmi_perf_ctx.scmi_api->respond(service_id, &return_value,
                                    sizeof(return_value));

    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api scmi_perf_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_perf_get_scmi_protocol_id,
    .message_handler = scmi_perf_message_handler
};

/*
 * Static helpers for responding to SCMI.
 */
static void scmi_perf_respond(
    void *return_values,
    fwk_id_t domain_id,
    int size)
{
    int idx = fwk_id_get_element_idx(domain_id);
    fwk_id_t service_id;

    /*
     * The service identifier used for the response is retrieved from the
     * domain operations table.
     */
    service_id = scmi_perf_ctx.perf_ops_table[idx].service_id;

    scmi_perf_ctx.scmi_api->respond(service_id,
        return_values, size);

    /*
     * Set the service identifier to 'none' to indicate the domain is
     * available again.
     */
    scmi_perf_ctx.perf_ops_table[idx].service_id = FWK_ID_NONE;
}

/*
 * Framework handlers
 */
static int scmi_perf_init(fwk_id_t module_id, unsigned int element_count,
                          const void *data)
{
    int return_val;
    int i;
    const struct mod_scmi_perf_config *config =
        (const struct mod_scmi_perf_config *)data;

    if ((config == NULL) || (config->domains == NULL))
        return FWK_E_PARAM;

    return_val = fwk_module_get_element_count(
        FWK_ID_MODULE(FWK_MODULE_IDX_DVFS));
    if (return_val <= 0)
        return FWK_E_SUPPORT;

    scmi_perf_ctx.perf_ops_table = fwk_mm_calloc(return_val,
        sizeof(struct perf_operations));

    scmi_perf_ctx.config = config;
    scmi_perf_ctx.domain_count = return_val;

    /* Initialize table */
    for (i = 0; i < return_val; i++)
        scmi_perf_ctx.perf_ops_table[i].service_id = FWK_ID_NONE;

    return FWK_SUCCESS;
}

static int scmi_perf_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_perf_ctx.scmi_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_DVFS),
        FWK_ID_API(FWK_MODULE_IDX_DVFS, 0), &scmi_perf_ctx.dvfs_api);
}

static int scmi_perf_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    *api = &scmi_perf_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

/*
 * Handle a request for get_level/limits.
 */
static int process_request_event(const struct fwk_event *event)
{
    int status;
    struct mod_dvfs_frequency_limits limits;
    struct scmi_perf_event_parameters *params;
    struct scmi_perf_level_get_p2a return_values_level;
    struct scmi_perf_limits_get_p2a return_values_limits;
    struct mod_dvfs_opp opp;

    /* request event to DVFS HAL */
    if (fwk_id_is_equal(event->id, scmi_perf_get_level)) {
        params = (struct scmi_perf_event_parameters *)event->params;

        status = scmi_perf_ctx.dvfs_api->get_current_opp(params->domain_id,
                                                       &opp);
        if (status == FWK_SUCCESS) {
            /* DVFS value is ready */
            return_values_level = (struct scmi_perf_level_get_p2a) {
                .status = SCMI_SUCCESS,
                .performance_level = (uint32_t)opp.frequency,
            };

            scmi_perf_respond(&return_values_level, params->domain_id,
                              sizeof(return_values_level));

            return status;
        } else if (status == FWK_PENDING) {
            /* DVFS value will be provided through a response event */
            return FWK_SUCCESS;
        } else {
            return_values_level = (struct scmi_perf_level_get_p2a) {
                .status = SCMI_HARDWARE_ERROR,
            };

            scmi_perf_respond(&return_values_level, params->domain_id,
                              sizeof(return_values_level.status));

            return FWK_E_DEVICE;
        }
    }

    if (fwk_id_is_equal(event->id, scmi_perf_get_limits)) {
        params = (struct scmi_perf_event_parameters *)event->params;

        status = scmi_perf_ctx.dvfs_api->get_frequency_limits(params->domain_id,
                                                       &limits);
        if (status == FWK_SUCCESS) {
            /* DVFS value is ready */
            return_values_limits = (struct scmi_perf_limits_get_p2a) {
                .status = SCMI_SUCCESS,
                .range_min = limits.minimum,
                .range_max = limits.maximum,
            };

            scmi_perf_respond(&return_values_limits, params->domain_id,
                              sizeof(return_values_limits));

            return status;
        } else if (status == FWK_PENDING) {
            /* DVFS value will be provided through a response event */
            return FWK_SUCCESS;
        } else {
            return_values_limits = (struct scmi_perf_limits_get_p2a) {
                .status = SCMI_HARDWARE_ERROR,
            };

            scmi_perf_respond(&return_values_limits, params->domain_id,
                              sizeof(return_values_limits.status));

            return FWK_E_DEVICE;
        }
    }

    return FWK_E_PARAM;
}

/*
 * Handle a response event from the HAL which indicates that the
 * requested operation has completed.
 */
static int process_response_event(const struct fwk_event *event)
{
    struct mod_dvfs_params_response *params_level;
    struct scmi_perf_level_get_p2a return_values_level;

    if (fwk_id_is_equal(event->id, mod_dvfs_event_id_get_opp)) {
        params_level = (struct mod_dvfs_params_response *)
            event->params;
        return_values_level = (struct scmi_perf_level_get_p2a) {
            .status = params_level->status,
            .performance_level = params_level->performance_level,
        };
        scmi_perf_respond(&return_values_level, event->source_id,
            (return_values_level.status == SCMI_SUCCESS) ?
            sizeof(return_values_level) :
            sizeof(return_values_level.status));
    }

    return FWK_SUCCESS;
}

static int scmi_perf_process_event(const struct fwk_event *event,
                                    struct fwk_event *resp_event)
{
    /* Request events from SCMI */
    if (fwk_id_get_module_idx(event->source_id) ==
        fwk_id_get_module_idx(fwk_module_id_scmi))
        return process_request_event(event);

    /* Response events from DVFS */
    if (fwk_id_get_module_idx(event->source_id) ==
        fwk_id_get_module_idx(fwk_module_id_dvfs))
        return process_response_event(event);

    return FWK_E_PARAM;
}

/* SCMI Performance Management Protocol Definition */
const struct fwk_module module_scmi_perf = {
    .name = "SCMI Performance Management Protocol",
    .api_count = 1,
    .event_count = SCMI_PERF_EVENT_IDX_COUNT,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_perf_init,
    .bind = scmi_perf_bind,
    .process_bind_request = scmi_perf_process_bind_request,
    .process_event = scmi_perf_process_event,
};
