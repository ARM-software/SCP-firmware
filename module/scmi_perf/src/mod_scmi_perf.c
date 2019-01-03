/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI performance domain management protocol support.
 */

#include <string.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <internal/scmi.h>
#include <internal/scmi_perf.h>
#include <mod_dvfs.h>
#include <mod_scmi.h>
#include <mod_scmi_perf.h>

struct scmi_perf_ctx {
    /* SCMI Performance Module Configuration */
    const struct mod_scmi_perf_config *config;

    /* Number of power domains */
    unsigned int domain_count;

    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* DVFS module API */
    const struct mod_dvfs_domain_api *dvfs_api;
};

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

static struct scmi_perf_ctx scmi_perf_ctx;

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
    unsigned int message_id;
    struct scmi_protocol_message_attributes_p2a return_values;

    parameters =
        (const struct scmi_protocol_message_attributes_a2p *)payload;
    message_id = parameters->message_id;

    if ((message_id < FWK_ARRAY_SIZE(handler_table)) &&
        (handler_table[message_id] != NULL)) {
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
    struct scmi_perf_domain_attributes_p2a return_values;
    uint32_t permissions;
    fwk_id_t domain_id;
    struct mod_dvfs_opp opp;

    return_values.status = SCMI_GENERIC_ERROR;

    /* Validate the domain identifier */
    parameters = (const struct scmi_perf_domain_attributes_a2p *)payload;
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

    domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id),
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
    strncpy((char *)return_values.name, fwk_module_get_name(domain_id),
        sizeof(return_values.name) - 1);

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
    struct scmi_perf_describe_levels_p2a return_values;
    fwk_id_t domain_id;
    struct scmi_perf_level perf_level;
    unsigned int num_levels, level_index, level_index_max;
    size_t payload_size;
    size_t opp_count;
    struct mod_dvfs_opp opp;
    uint16_t latency;

    return_values.status = SCMI_GENERIC_ERROR;
    payload_size = sizeof(return_values);

    status = scmi_perf_ctx.scmi_api->get_max_payload_size(service_id,
                                                          &max_payload_size);
    if (status != FWK_SUCCESS)
        goto exit;

    status = (SCMI_PERF_LEVELS_MAX(max_payload_size) > 0) ?
        FWK_SUCCESS : FWK_E_SIZE;
    assert(status == FWK_SUCCESS);
    if (status != FWK_SUCCESS)
        goto exit;


    /* Validate the domain identifier */
    parameters = (const struct scmi_perf_describe_levels_a2p *)payload;
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
    struct scmi_perf_limits_set_p2a return_values;
    uint32_t permissions;

    return_values.status = SCMI_GENERIC_ERROR;

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

    /* Execute the transition asynchronously */
    status = scmi_perf_ctx.dvfs_api->set_frequency_limits(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id),
        &((struct mod_dvfs_frequency_limits) {
           .minimum = parameters->range_min,
           .maximum = parameters->range_max
          }));
    if (status != FWK_SUCCESS) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_OUT_OF_RANGE;

        goto exit;
    }

    return_values = (struct scmi_perf_limits_set_p2a) {
        .status = SCMI_SUCCESS,
    };

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
    const struct scmi_perf_limits_get_a2p *parameters;
    struct scmi_perf_limits_get_p2a return_values;
    struct mod_dvfs_frequency_limits limits;

    return_values.status = SCMI_GENERIC_ERROR;

    parameters = (const struct scmi_perf_limits_get_a2p *)payload;
    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx.dvfs_api->get_frequency_limits(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id), &limits);
    if (status != FWK_SUCCESS)
        goto exit;

    return_values = (struct scmi_perf_limits_get_p2a) {
        .status = SCMI_SUCCESS,
        .range_min = (uint32_t)limits.minimum,
        .range_max = (uint32_t)limits.maximum
    };

exit:
    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_perf_level_set_handler(fwk_id_t service_id,
                                       const uint32_t *payload)
{
    int status;
    unsigned int agent_id;
    const struct mod_scmi_perf_domain_config *domain;
    const struct scmi_perf_level_set_a2p *parameters;
    struct scmi_perf_level_set_p2a return_values;
    uint32_t permissions;

    return_values.status = SCMI_GENERIC_ERROR;

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

    /* Execute the transition synchronously */
    status = scmi_perf_ctx.dvfs_api->set_frequency(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id),
        parameters->performance_level);

    if (status == FWK_E_RANGE) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_OUT_OF_RANGE;
    } else if (status != FWK_SUCCESS) {
        return_values.status = SCMI_GENERIC_ERROR;
    } else {
        return_values = (struct scmi_perf_level_set_p2a) {
            .status = SCMI_SUCCESS,
        };
    }

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
    struct scmi_perf_level_get_p2a return_values;
    struct mod_dvfs_opp opp;

    return_values.status = SCMI_GENERIC_ERROR;

    parameters = (const struct scmi_perf_level_get_a2p *)payload;
    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx.dvfs_api->get_current_opp(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id), &opp);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Return the frequency as the performance level */
    return_values = (struct scmi_perf_level_get_p2a) {
        .status = SCMI_SUCCESS,
        .performance_level = (uint32_t)opp.frequency,
    };

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
    int status;

    status = fwk_module_check_call(protocol_id);
    if (status != FWK_SUCCESS)
        return status;

    *scmi_protocol_id = SCMI_PROTOCOL_ID_PERF;

    return FWK_SUCCESS;
}

static int scmi_perf_message_handler(fwk_id_t protocol_id, fwk_id_t service_id,
    const uint32_t *payload, size_t payload_size, unsigned int message_id)
{
    int status;
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Performance management protocol table sizes not consistent");
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
    scmi_perf_ctx.scmi_api->respond(service_id, &return_value,
                                    sizeof(return_value));

    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api scmi_perf_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_perf_get_scmi_protocol_id,
    .message_handler = scmi_perf_message_handler
};

/*
 * Framework handlers
 */
static int scmi_perf_init(fwk_id_t module_id, unsigned int element_count,
                          const void *data)
{
    int return_val;
    const struct mod_scmi_perf_config *config =
        (const struct mod_scmi_perf_config *)data;

    if ((config == NULL) || (config->domains == NULL))
        return FWK_E_PARAM;

    return_val = fwk_module_get_element_count(
        FWK_ID_MODULE(FWK_MODULE_IDX_DVFS));
    if (return_val <= 0)
        return FWK_E_SUPPORT;

    scmi_perf_ctx.config = config;
    scmi_perf_ctx.domain_count = return_val;

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
/* SCMI Performance Management Protocol Definition */
const struct fwk_module module_scmi_perf = {
    .name = "SCMI Performance Management Protocol",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_perf_init,
    .bind = scmi_perf_bind,
    .process_bind_request = scmi_perf_process_bind_request,
};
