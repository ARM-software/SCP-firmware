/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI performance protocol/commands handling support.
 */

#include <internal/scmi_perf.h>

#include <mod_dvfs.h>
#include <mod_scmi.h>
#include <mod_scmi_perf.h>

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
#    include "perf_plugins_handler.h"
#endif

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
#    include <mod_resource_perms.h>
#endif

#ifdef BUILD_HAS_MOD_STATISTICS
#    include <mod_stats.h>
#endif

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_string.h>

static const fwk_id_t scmi_perf_get_level = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_SCMI_PERF,
    SCMI_PERF_EVENT_IDX_LEVEL_GET_REQUEST);

struct scmi_perf_event_parameters {
    fwk_id_t domain_id;
};

#define MOD_SCMI_PERF_NOTIFICATION_COUNT 2

static int scmi_perf_protocol_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_perf_protocol_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_perf_protocol_message_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_perf_domain_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_perf_describe_levels_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_perf_level_set_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_perf_level_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_perf_limits_set_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_perf_limits_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
static int scmi_perf_describe_fast_channels(
    fwk_id_t service_id,
    const uint32_t *payload);
#endif
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_perf_limits_notify(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_perf_level_notify(fwk_id_t service_id, const uint32_t *payload);
#endif

static int (
    *handler_table[MOD_SCMI_PERF_COMMAND_COUNT])(fwk_id_t, const uint32_t *) = {
    [MOD_SCMI_PROTOCOL_VERSION] = scmi_perf_protocol_version_handler,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = scmi_perf_protocol_attributes_handler,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_perf_protocol_message_attributes_handler,
    [MOD_SCMI_PERF_DOMAIN_ATTRIBUTES] = scmi_perf_domain_attributes_handler,
    [MOD_SCMI_PERF_DESCRIBE_LEVELS] = scmi_perf_describe_levels_handler,
    [MOD_SCMI_PERF_LIMITS_SET] = scmi_perf_limits_set_handler,
    [MOD_SCMI_PERF_LIMITS_GET] = scmi_perf_limits_get_handler,
    [MOD_SCMI_PERF_LEVEL_SET] = scmi_perf_level_set_handler,
    [MOD_SCMI_PERF_LEVEL_GET] = scmi_perf_level_get_handler,
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    [MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL] = scmi_perf_describe_fast_channels,
#endif
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_PERF_NOTIFY_LIMITS] = scmi_perf_limits_notify,
    [MOD_SCMI_PERF_NOTIFY_LEVEL] = scmi_perf_level_notify
#endif
};

static unsigned int payload_size_table[MOD_SCMI_PERF_COMMAND_COUNT] = {
    [MOD_SCMI_PROTOCOL_VERSION] = 0,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        (unsigned int)sizeof(struct scmi_protocol_message_attributes_a2p),
    [MOD_SCMI_PERF_DOMAIN_ATTRIBUTES] =
        (unsigned int)sizeof(struct scmi_perf_domain_attributes_a2p),
    [MOD_SCMI_PERF_DESCRIBE_LEVELS] =
        (unsigned int)sizeof(struct scmi_perf_describe_levels_a2p),
    [MOD_SCMI_PERF_LEVEL_SET] =
        (unsigned int)sizeof(struct scmi_perf_level_set_a2p),
    [MOD_SCMI_PERF_LEVEL_GET] =
        (unsigned int)sizeof(struct scmi_perf_level_get_a2p),
    [MOD_SCMI_PERF_LIMITS_SET] =
        (unsigned int)sizeof(struct scmi_perf_limits_set_a2p),
    [MOD_SCMI_PERF_LIMITS_GET] =
        (unsigned int)sizeof(struct scmi_perf_limits_get_a2p),
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    [MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL] =
        sizeof(struct scmi_perf_describe_fc_a2p),
#endif
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_PERF_NOTIFY_LIMITS] = sizeof(struct scmi_perf_notify_limits_a2p),
    [MOD_SCMI_PERF_NOTIFY_LEVEL] = sizeof(struct scmi_perf_notify_level_a2p)
#endif
};

static struct scmi_perf_protocol_ctx {
    struct mod_scmi_perf_ctx *scmi_perf_ctx;

    struct mod_scmi_perf_private_api_perf_stub *api_stub;

    /* Pointer to a table of operations */
    struct perf_operations *perf_ops_table;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /* Number of active agents */
    unsigned int agent_count;

    /* SCMI notification API */
    const struct mod_scmi_notification_api *scmi_notification_api;
#endif

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    /* SCMI Resource Permissions API */
    const struct mod_res_permissions_api *res_perms_api;
#endif

#ifdef BUILD_HAS_MOD_STATISTICS
    /* Statistics module API */
    const struct mod_stats_api *stats_api;
#endif

} perf_prot_ctx;

/* This identifier is either:
 * - the element type version of the one built by the perf-plugins-handler
 *      (sub-element type)
 * - or the DVFS domain
 */
static inline fwk_id_t get_dvfs_dependency_id(unsigned int el_idx)
{
#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
    fwk_id_t id;

    id = perf_plugins_get_dependency_id(el_idx);
    return fwk_id_build_element_id(id, el_idx);
#else
    return FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, el_idx);
#endif
}

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS

/*
 * SCMI Resource Permissions handler
 */
static int get_perf_domain_id(const uint32_t *payload, unsigned int *domain_id)
{
    /*
     * Every SCMI Performance message is formatted with the domain ID
     * as the first message element. We will use the perf_limits_get
     * message as a basic format to retrieve the domain ID to avoid
     * unnecessary code.
     */
    const struct scmi_perf_limits_get_a2p *parameters =
        (const struct scmi_perf_limits_get_a2p *)payload;

    if (parameters->domain_id >= perf_prot_ctx.scmi_perf_ctx->domain_count) {
        return FWK_E_PARAM;
    }

    *domain_id = parameters->domain_id;
    return FWK_SUCCESS;
}

static int scmi_perf_permissions_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    unsigned int message_id)
{
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;
    enum mod_res_perms_permissions perms;
    unsigned int agent_id, domain_id;
    int status;

    status = scmi_perf_ctx->scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        return FWK_E_ACCESS;
    }

    if (message_id < (unsigned int)MOD_SCMI_PERF_DOMAIN_ATTRIBUTES) {
        /*
         * PROTOCOL_VERSION, PROTOCOL_ATTRIBUTES & PROTOCOL_MESSAGE_ATRIBUTES
         */
        perms = perf_prot_ctx.res_perms_api->agent_has_protocol_permission(
            agent_id, MOD_SCMI_PROTOCOL_ID_PERF);
        if (perms == MOD_RES_PERMS_ACCESS_ALLOWED) {
            return FWK_SUCCESS;
        }
        return FWK_E_ACCESS;
    }

    status = get_perf_domain_id(payload, &domain_id);
    if (status != FWK_SUCCESS) {
        return FWK_E_PARAM;
    }

    perms = perf_prot_ctx.res_perms_api->agent_has_resource_permission(
        agent_id, MOD_SCMI_PROTOCOL_ID_PERF, message_id, domain_id);

    if (perms == MOD_RES_PERMS_ACCESS_ALLOWED) {
        return FWK_SUCCESS;
    } else {
        return FWK_E_ACCESS;
    }
}

#endif

/*
 * Protocol command handlers
 */

static int scmi_perf_protocol_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_PERF,
    };

    return perf_prot_ctx.scmi_perf_ctx->scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_perf_protocol_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;

#ifdef BUILD_HAS_MOD_STATISTICS
    int status;
#endif
    struct scmi_perf_protocol_attributes_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
        .attributes =
            SCMI_PERF_PROTOCOL_ATTRIBUTES(true, scmi_perf_ctx->domain_count),
    };
    uint32_t addr_low = 0, addr_high = 0, len = 0;

#ifdef BUILD_HAS_MOD_STATISTICS
    status = perf_prot_ctx.stats_api->get_statistics_desc(
        fwk_module_id_scmi_perf, &addr_low, &addr_high, &len);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_GENERIC_ERROR;
    }
#endif

    return_values.statistics_len = len;
    return_values.statistics_address_low = addr_low;
    return_values.statistics_address_high = addr_high;

    return scmi_perf_ctx->scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_perf_protocol_message_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *parameters;
    struct scmi_protocol_message_attributes_p2a return_values;

    parameters = (const struct scmi_protocol_message_attributes_a2p *)payload;

    if ((parameters->message_id < FWK_ARRAY_SIZE(handler_table)) &&
        (handler_table[parameters->message_id] != NULL)) {
        return_values = (struct scmi_protocol_message_attributes_p2a){
            .status = SCMI_SUCCESS,
        };
    } else {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
    }

    return_values.attributes = 0;
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    if (perf_fch_prot_msg_attributes_has_fastchannels(parameters)) {
        return_values.attributes = 1; /* Fast Channel available */
    }
#endif

    return perf_prot_ctx.scmi_perf_ctx->scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
}

static int scmi_perf_domain_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    int status, respond_status;
    unsigned int agent_id;
    const struct scmi_perf_domain_attributes_a2p *parameters;
    uint32_t permissions = 0;
    fwk_id_t domain_id;
    struct mod_dvfs_opp opp;
    struct scmi_perf_domain_attributes_p2a return_values = {
        .status = (int32_t)SCMI_GENERIC_ERROR,
    };
    bool notifications = false;
    bool fast_channels = false;
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;

    parameters = (const struct scmi_perf_domain_attributes_a2p *)payload;

    /* Validate the domain identifier */
    if (parameters->domain_id >= scmi_perf_ctx->domain_count) {
        status = FWK_SUCCESS;
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx->scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

#ifndef BUILD_HAS_MOD_RESOURCE_PERMS
    permissions = ((uint32_t)MOD_SCMI_PERF_PERMS_SET_LIMITS) |
        ((uint32_t)MOD_SCMI_PERF_PERMS_SET_LEVEL);
#else
    status = scmi_perf_permissions_handler(
        service_id, payload, (unsigned int)MOD_SCMI_PERF_LIMITS_SET);
    if (status == FWK_SUCCESS) {
        permissions = (uint8_t)MOD_SCMI_PERF_PERMS_SET_LIMITS;
    }
    status = scmi_perf_permissions_handler(
        service_id, payload, (unsigned int)MOD_SCMI_PERF_LEVEL_SET);
    if (status == FWK_SUCCESS) {
        permissions |= (uint32_t)MOD_SCMI_PERF_PERMS_SET_LEVEL;
    }
#endif

    domain_id = get_dvfs_dependency_id(parameters->domain_id);
    status = scmi_perf_ctx->dvfs_api->get_sustained_opp(domain_id, &opp);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    notifications = true;
#endif
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    fast_channels = perf_fch_domain_attributes_has_fastchannels(parameters);
#endif
    return_values = (struct scmi_perf_domain_attributes_p2a){
        .status = SCMI_SUCCESS,
        .attributes = SCMI_PERF_DOMAIN_ATTRIBUTES(
            (uint32_t)notifications,
            (uint32_t)notifications,
            ((permissions & (uint32_t)MOD_SCMI_PERF_PERMS_SET_LEVEL) !=
             (uint32_t)0) ?
                1U :
                0U,
            ((permissions & (uint32_t)MOD_SCMI_PERF_PERMS_SET_LIMITS) !=
             (uint32_t)0) ?
                1U :
                0U,
            (uint32_t)fast_channels),
        .rate_limit = 0, /* Unsupported */
        .sustained_freq = opp.frequency,
        .sustained_perf_level = opp.level,
    };

    /* Copy the domain name into the mailbox */
    fwk_str_strncpy(
        (char *)return_values.name,
        fwk_module_get_element_name(fwk_id_build_element_id(
            domain_id, fwk_id_get_element_idx(domain_id))),
        sizeof(return_values.name) - 1);

exit:
    respond_status = scmi_perf_ctx->scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));

    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }

    return status;
}

static int scmi_perf_describe_levels_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    int status, respond_status;
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
        .status = (int32_t)SCMI_GENERIC_ERROR,
    };
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;

    payload_size = sizeof(return_values);

    status = scmi_perf_ctx->scmi_api->get_max_payload_size(
        service_id, &max_payload_size);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    status =
        (SCMI_PERF_LEVELS_MAX(max_payload_size) > 0) ? FWK_SUCCESS : FWK_E_SIZE;
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    parameters = (const struct scmi_perf_describe_levels_a2p *)payload;

    /* Validate the domain identifier */
    if (parameters->domain_id >= scmi_perf_ctx->domain_count) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        goto exit;
    }

    /* Get the number of operating points for the domain */
    domain_id = get_dependency_id(parameters->domain_id);
    status = scmi_perf_ctx->dvfs_api->get_opp_count(domain_id, &opp_count);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    /* Validate level index */
    level_index = parameters->level_index;
    if (level_index >= opp_count) {
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;

        goto exit;
    }

    /* Identify the maximum number of performance levels we can send at once */
    if (SCMI_PERF_LEVELS_MAX(max_payload_size) < (opp_count - level_index)) {
        num_levels = (unsigned int)SCMI_PERF_LEVELS_MAX(max_payload_size);
    } else {
        num_levels = (unsigned int)(opp_count - level_index);
    }

    level_index_max = (level_index + num_levels - 1);

    status = scmi_perf_ctx->dvfs_api->get_latency(domain_id, &latency);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    /* Copy DVFS data into returned data structure */
    for (; level_index <= level_index_max;
         level_index++, payload_size += sizeof(perf_level)) {
        status =
            scmi_perf_ctx->dvfs_api->get_nth_opp(domain_id, level_index, &opp);
        if (status != FWK_SUCCESS) {
            goto exit;
        }

        if (opp.power != 0) {
            perf_level.power_cost = opp.power;
        } else {
            perf_level.power_cost = opp.voltage;
        }
        perf_level.performance_level = opp.level;
        perf_level.attributes = latency;

        status = scmi_perf_ctx->scmi_api->write_payload(
            service_id, payload_size, &perf_level, sizeof(perf_level));
        if (status != FWK_SUCCESS) {
            goto exit;
        }
    }

    return_values = (struct scmi_perf_describe_levels_p2a){
        .status = SCMI_SUCCESS,
        .num_levels =
            SCMI_PERF_NUM_LEVELS(num_levels, (opp_count - level_index_max - 1))
    };

    status = scmi_perf_ctx->scmi_api->write_payload(
        service_id, 0, &return_values, sizeof(return_values));

exit:
    respond_status = scmi_perf_ctx->scmi_api->respond(
        service_id,
        (return_values.status == SCMI_SUCCESS) ? NULL : &return_values.status,
        (return_values.status == SCMI_SUCCESS) ? payload_size :
                                                 sizeof(return_values.status));
    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }

    return status;
}

static int scmi_perf_limits_set_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    int status, respond_status;
    unsigned int agent_id;
    const struct scmi_perf_limits_set_a2p *parameters;
    uint32_t range_min, range_max;
    fwk_id_t domain_id;
    struct scmi_perf_limits_set_p2a return_values = {
        .status = (int32_t)SCMI_GENERIC_ERROR,
    };
    enum mod_scmi_perf_policy_status policy_status;
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;

    parameters = (const struct scmi_perf_limits_set_a2p *)payload;

    if (parameters->domain_id >= scmi_perf_ctx->domain_count) {
        status = FWK_SUCCESS;
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx->scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    if (parameters->range_min > parameters->range_max) {
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    domain_id = get_dependency_id(parameters->domain_id);
    range_min = parameters->range_min;
    range_max = parameters->range_max;

    status = scmi_perf_limits_set_policy(
        &policy_status, &range_min, &range_max, agent_id, domain_id);

    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_GENERIC_ERROR;
        goto exit;
    }
    if (policy_status == MOD_SCMI_PERF_SKIP_MESSAGE_HANDLER) {
        return_values.status = (int32_t)SCMI_SUCCESS;
        goto exit;
    }

    status = perf_prot_ctx.api_stub->perf_set_limits(
        domain_id,
        agent_id,
        &((struct mod_scmi_perf_level_limits){ .minimum = range_min,
                                               .maximum = range_max }));

    /*
     * Return immediately to the caller, fire-and-forget.
     */
    if ((status == FWK_SUCCESS) || (status == FWK_PENDING)) {
        return_values.status = (int32_t)SCMI_SUCCESS;
    } else {
        return_values.status = (int32_t)SCMI_OUT_OF_RANGE;
    }

exit:
    respond_status = scmi_perf_ctx->scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }

    return status;
}

static int scmi_perf_limits_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    fwk_id_t domain_id;
    const struct scmi_perf_limits_get_a2p *parameters;
    struct scmi_perf_limits_get_p2a return_values;
    struct scmi_perf_domain_ctx *domain_ctx;

    parameters = (const struct scmi_perf_limits_get_a2p *)payload;
    if (parameters->domain_id >= perf_prot_ctx.scmi_perf_ctx->domain_count) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        goto exit;
    }

    domain_id = get_dependency_id(parameters->domain_id);
    domain_ctx = &perf_prot_ctx.scmi_perf_ctx
                      ->domain_ctx_table[fwk_id_get_element_idx(domain_id)];

    return_values.status = (int32_t)SCMI_SUCCESS;
    return_values.range_min = domain_ctx->level_limits.minimum;
    return_values.range_max = domain_ctx->level_limits.maximum;

exit:
    return perf_prot_ctx.scmi_perf_ctx->scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
}

static int scmi_perf_level_set_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    int status, respond_status;
    unsigned int agent_id;
    const struct scmi_perf_level_set_a2p *parameters;
    fwk_id_t domain_id;
    struct scmi_perf_level_set_p2a return_values = {
        .status = (int32_t)SCMI_GENERIC_ERROR,
    };
    uint32_t perf_level;
    enum mod_scmi_perf_policy_status policy_status;
    parameters = (const struct scmi_perf_level_set_a2p *)payload;
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;

    if (parameters->domain_id >= scmi_perf_ctx->domain_count) {
        status = FWK_SUCCESS;
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx->scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    /*
     * Note that the policy handler may change the performance level
     */
    domain_id = get_dependency_id(parameters->domain_id);
    perf_level = parameters->performance_level;

    status = scmi_perf_level_set_policy(
        &policy_status, &perf_level, agent_id, domain_id);

    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_GENERIC_ERROR;
        goto exit;
    }
    if (policy_status == MOD_SCMI_PERF_SKIP_MESSAGE_HANDLER) {
        return_values.status = (int32_t)SCMI_SUCCESS;
        goto exit;
    }

    status =
        perf_prot_ctx.api_stub->perf_set_level(domain_id, agent_id, perf_level);

    /*
     * Return immediately to the caller, fire-and-forget.
     */
    if ((status == FWK_SUCCESS) || (status == FWK_PENDING)) {
        return_values.status = (int32_t)SCMI_SUCCESS;
    } else if (status == FWK_E_RANGE) {
        return_values.status = (int32_t)SCMI_OUT_OF_RANGE;
    }

exit:
    respond_status = scmi_perf_ctx->scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }

    return status;
}

static int scmi_perf_level_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    int status, respond_status;
    const struct scmi_perf_level_get_a2p *parameters;
    struct scmi_perf_event_parameters *evt_params;
    struct scmi_perf_level_get_p2a return_values;
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;

    parameters = (const struct scmi_perf_level_get_a2p *)payload;
    if (parameters->domain_id >= scmi_perf_ctx->domain_count) {
        status = FWK_SUCCESS;
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        goto exit;
    }

    /* Check if there is already a request pending for this domain */
    if (!fwk_id_is_equal(
            perf_prot_ctx
                .perf_ops_table[fwk_id_get_element_idx(
                    get_dependency_id(parameters->domain_id))]
                .service_id,
            FWK_ID_NONE)) {
        return_values.status = (int32_t)SCMI_BUSY;
        status = FWK_SUCCESS;

        goto exit;
    }

    /* The get_level request is processed within the event being generated */
    struct fwk_event event = {
        .target_id = fwk_module_id_scmi_perf,
        .id = scmi_perf_get_level,
    };

    evt_params = (struct scmi_perf_event_parameters *)event.params;
    evt_params->domain_id = get_dependency_id(parameters->domain_id);

    status = fwk_put_event(&event);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_GENERIC_ERROR;

        goto exit;
    }

    /* Store service identifier to indicate there is a pending request */
    perf_prot_ctx.perf_ops_table[fwk_id_get_element_idx(evt_params->domain_id)]
        .service_id = service_id;

    return FWK_SUCCESS;

exit:
    respond_status = scmi_perf_ctx->scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }

    return status;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_perf_limits_notify(fwk_id_t service_id, const uint32_t *payload)
{
    unsigned int agent_id;
    int status, respond_status;
    unsigned int id;
    const struct scmi_perf_notify_limits_a2p *parameters;
    struct scmi_perf_notify_limits_p2a return_values = {
        .status = (int32_t)SCMI_GENERIC_ERROR,
    };
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;

    parameters = (const struct scmi_perf_notify_limits_a2p *)payload;
    id = parameters->domain_id;
    if (id >= scmi_perf_ctx->domain_count) {
        status = FWK_SUCCESS;
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        goto exit;
    }

    if ((parameters->notify_enable &
         ~SCMI_PERF_NOTIFY_LIMITS_NOTIFY_ENABLE_MASK) != 0x0) {
        status = FWK_SUCCESS;
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;

        goto exit;
    }

    status = scmi_perf_ctx->scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    if (parameters->notify_enable) {
        status = perf_prot_ctx.scmi_notification_api
                     ->scmi_notification_add_subscriber(
                         MOD_SCMI_PROTOCOL_ID_PERF,
                         id,
                         MOD_SCMI_PERF_NOTIFY_LIMITS,
                         service_id);
    } else {
        status = perf_prot_ctx.scmi_notification_api
                     ->scmi_notification_remove_subscriber(
                         MOD_SCMI_PROTOCOL_ID_PERF,
                         agent_id,
                         id,
                         MOD_SCMI_PERF_NOTIFY_LIMITS);
    }
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    return_values.status = (int32_t)SCMI_SUCCESS;

exit:
    respond_status = scmi_perf_ctx->scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }

    return status;
}

static int scmi_perf_level_notify(fwk_id_t service_id, const uint32_t *payload)
{
    unsigned int agent_id;
    int status, respond_status;
    unsigned int id;
    const struct scmi_perf_notify_level_a2p *parameters;
    struct scmi_perf_notify_level_p2a return_values = {
        .status = (int32_t)SCMI_GENERIC_ERROR,
    };
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;

    parameters = (const struct scmi_perf_notify_level_a2p *)payload;
    id = parameters->domain_id;
    if (id >= scmi_perf_ctx->domain_count) {
        status = FWK_SUCCESS;
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        goto exit;
    }

    if ((parameters->notify_enable &
         ~SCMI_PERF_NOTIFY_LEVEL_NOTIFY_ENABLE_MASK) != 0x0) {
        status = FWK_SUCCESS;
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;

        goto exit;
    }

    id = parameters->domain_id;

    status = scmi_perf_ctx->scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    if (parameters->notify_enable) {
        status = perf_prot_ctx.scmi_notification_api
                     ->scmi_notification_add_subscriber(
                         MOD_SCMI_PROTOCOL_ID_PERF,
                         id,
                         MOD_SCMI_PERF_NOTIFY_LEVEL,
                         service_id);
    } else {
        status = perf_prot_ctx.scmi_notification_api
                     ->scmi_notification_remove_subscriber(
                         MOD_SCMI_PROTOCOL_ID_PERF,
                         agent_id,
                         id,
                         MOD_SCMI_PERF_NOTIFY_LEVEL);
    }
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    return_values.status = (int32_t)SCMI_SUCCESS;

exit:
    respond_status = scmi_perf_ctx->scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }

    return status;
}
#endif

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS

/*
 * Note that the Fast Channel doorbell is not supported in this
 * implementation.
 */
static int scmi_perf_describe_fast_channels(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    return perf_fch_describe_fast_channels(service_id, payload);
}
#endif

/*
 * SCMI Performance Policy Handlers
 */
FWK_WEAK int scmi_perf_limits_set_policy(
    enum mod_scmi_perf_policy_status *policy_status,
    uint32_t *range_min,
    uint32_t *range_max,
    unsigned int agent_id,
    fwk_id_t domain_id)
{
    *policy_status = MOD_SCMI_PERF_EXECUTE_MESSAGE_HANDLER;

    return FWK_SUCCESS;
}

FWK_WEAK int scmi_perf_level_set_policy(
    enum mod_scmi_perf_policy_status *policy_status,
    uint32_t *level,
    unsigned int agent_id,
    fwk_id_t domain_id)
{
    *policy_status = MOD_SCMI_PERF_EXECUTE_MESSAGE_HANDLER;

    return FWK_SUCCESS;
}

/*
 * SCMI module -> SCMI performance module interface
 */

static int scmi_perf_get_scmi_protocol_id(
    fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_PERF;

    return FWK_SUCCESS;
}

static int scmi_perf_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    int32_t return_value;
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    int status;
#endif

    static_assert(
        FWK_ARRAY_SIZE(handler_table) == FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Performance management protocol table sizes not consistent");
    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return_value = (int32_t)SCMI_NOT_FOUND;
        goto error;
    }

    if (handler_table[message_id] == NULL) {
        return_value = (int32_t)SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        return_value = (int32_t)SCMI_PROTOCOL_ERROR;
        goto error;
    }

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    status = scmi_perf_permissions_handler(service_id, payload, message_id);
    if (status != FWK_SUCCESS) {
        if (status == FWK_E_PARAM) {
            return_value = (int32_t)SCMI_NOT_FOUND;
        } else {
            return_value = (int32_t)SCMI_DENIED;
        }
        goto error;
    }
#endif

    return handler_table[message_id](service_id, payload);

error:
    return perf_prot_ctx.scmi_perf_ctx->scmi_api->respond(
        service_id, &return_value, sizeof(return_value));
}

static struct mod_scmi_to_protocol_api scmi_perf_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_perf_get_scmi_protocol_id,
    .message_handler = scmi_perf_message_handler
};

/*
 * Static helpers for responding to SCMI.
 */
static void scmi_perf_respond(void *return_values, fwk_id_t domain_id, int size)
{
    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;
    int idx = (int)fwk_id_get_element_idx(domain_id);
    fwk_id_t service_id;
    int respond_status;

    /*
     * The service identifier used for the response is retrieved from the
     * domain operations table.
     */
    service_id = perf_prot_ctx.perf_ops_table[idx].service_id;

    respond_status =
        scmi_perf_ctx->scmi_api->respond(service_id, return_values, size);
    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }

    /*
     * Set the service identifier to 'none' to indicate the domain is
     * available again.
     */
    perf_prot_ctx.perf_ops_table[idx].service_id = FWK_ID_NONE;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
void perf_prot_ops_notify_level(
    unsigned int domain_idx,
    uint32_t level,
    uint32_t cookie)
{
    int status;
    struct scmi_perf_level_changed level_changed;

    level_changed.agent_id = (uint32_t)cookie;
    level_changed.domain_id = (uint32_t)domain_idx;
    level_changed.performance_level = level;

    status = perf_prot_ctx.scmi_notification_api->scmi_notification_notify(
        MOD_SCMI_PROTOCOL_ID_PERF,
        MOD_SCMI_PERF_NOTIFY_LEVEL,
        SCMI_PERF_LEVEL_CHANGED,
        &level_changed,
        sizeof(level_changed));
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }
}

void perf_prot_ops_notify_limits(
    unsigned int domain_idx,
    uint32_t range_min,
    uint32_t range_max)
{
    struct scmi_perf_limits_changed limits_changed;
    int status;

    limits_changed.agent_id = (uint32_t)0;
    limits_changed.domain_id = (uint32_t)domain_idx;
    limits_changed.range_min = range_min;
    limits_changed.range_max = range_max;

    status = perf_prot_ctx.scmi_notification_api->scmi_notification_notify(
        MOD_SCMI_PROTOCOL_ID_PERF,
        MOD_SCMI_PERF_NOTIFY_LIMITS,
        SCMI_PERF_LIMITS_CHANGED,
        &limits_changed,
        sizeof(limits_changed));
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }
}
#endif

#ifdef BUILD_HAS_MOD_STATISTICS
int perf_prot_ops_update_stats(fwk_id_t domain_id, uint32_t level)
{
    int idx = (int)fwk_id_get_element_idx(domain_id);
    size_t level_id;
    int status;

    status = perf_prot_ctx.scmi_perf_ctx->dvfs_api->get_level_id(
        domain_id, level, &level_id);
    if (status == FWK_SUCCESS) {
        status = perf_prot_ctx.stats_api->update_domain(
            fwk_module_id_scmi_perf,
            FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_PERF, idx),
            level_id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
        }
    } else {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }

    return status;
}
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_init_notifications(int domains)
{
    int status;

    status = perf_prot_ctx.scmi_perf_ctx->scmi_api->get_agent_count(
        &perf_prot_ctx.agent_count);
    if (status != FWK_SUCCESS) {
        return status;
    }

    fwk_assert(perf_prot_ctx.agent_count != 0u);

    status = perf_prot_ctx.scmi_notification_api->scmi_notification_init(
        MOD_SCMI_PROTOCOL_ID_PERF,
        perf_prot_ctx.agent_count,
        domains,
        MOD_SCMI_PERF_NOTIFICATION_COUNT);

    return status;
}
#endif

#ifdef BUILD_HAS_MOD_STATISTICS
static int scmi_perf_stats_start(void)
{
    const struct mod_scmi_perf_domain_config *domain;
    int status = FWK_SUCCESS;
    int stats_domains = 0;
    unsigned int i;

    struct mod_scmi_perf_ctx *scmi_perf_ctx = perf_prot_ctx.scmi_perf_ctx;

    if (!scmi_perf_ctx->config->stats_enabled) {
        return FWK_E_SUPPORT;
    }

    /* Count how many domains have statistics */
    for (i = 0; i < scmi_perf_ctx->domain_count; i++) {
        domain = &(*scmi_perf_ctx->config->domains)[i];
        if (domain->stats_collected) {
            stats_domains++;
        }
    }

    status = perf_prot_ctx.stats_api->init_stats(
        fwk_module_id_scmi_perf, scmi_perf_ctx->domain_count, stats_domains);

    if (status != FWK_SUCCESS) {
        return status;
    }

    for (i = 0; i < scmi_perf_ctx->domain_count; i++) {
        domain = &(*scmi_perf_ctx->config->domains)[i];
        /* Add this domain to track statistics when needed */
        if (domain->stats_collected) {
            fwk_id_t domain_id;
            size_t opp_count;

            domain_id = get_dependency_id(i);
            status =
                scmi_perf_ctx->dvfs_api->get_opp_count(domain_id, &opp_count);

            if (status != FWK_SUCCESS) {
                return status;
            }

            status = perf_prot_ctx.stats_api->add_domain(
                fwk_module_id_scmi_perf,
                FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_PERF, i),
                (int)opp_count);

            if (status != FWK_SUCCESS) {
                return status;
            }
        }
    }

    return perf_prot_ctx.stats_api->start_stats(fwk_module_id_scmi_perf);
}
#endif

int perf_prot_ops_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data,
    struct mod_scmi_perf_ctx *mod_ctx,
    struct mod_scmi_perf_private_api_perf_stub *api)
{
    uint32_t i;

    perf_prot_ctx.scmi_perf_ctx = mod_ctx;
    perf_prot_ctx.api_stub = api;

    perf_prot_ctx.perf_ops_table = fwk_mm_calloc(
        perf_prot_ctx.scmi_perf_ctx->config->perf_doms_count,
        sizeof(struct perf_operations));

    /* Initialize table */
    for (i = 0; i < perf_prot_ctx.scmi_perf_ctx->domain_count; i++) {
        perf_prot_ctx.perf_ops_table[i].service_id = FWK_ID_NONE;
    }

    return FWK_SUCCESS;
}

int perf_prot_ops_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &perf_prot_ctx.res_perms_api);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_NOTIFICATION),
        &perf_prot_ctx.scmi_notification_api);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

#ifdef BUILD_HAS_MOD_STATISTICS
    if (perf_prot_ctx.scmi_perf_ctx->config->stats_enabled) {
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_STATISTICS),
            FWK_ID_API(FWK_MODULE_IDX_STATISTICS, MOD_STATS_API_IDX_STATS),
            &perf_prot_ctx.stats_api);
        if (status != FWK_SUCCESS) {
            return FWK_E_PANIC;
        }
    }
#endif

    return status;
}

int perf_prot_ops_start(fwk_id_t id)
{
    int status = FWK_SUCCESS;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    status =
        scmi_init_notifications((int)perf_prot_ctx.scmi_perf_ctx->domain_count);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

#ifdef BUILD_HAS_MOD_STATISTICS
    status = scmi_perf_stats_start();
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

    return status;
}

void perf_prot_ops_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **prot_bind_request_api)
{
    *prot_bind_request_api = &scmi_perf_mod_scmi_to_protocol_api;
}

/*
 * Handle a request for get_level/limits.
 */
static int process_request_event(const struct fwk_event *event)
{
    int status;
    struct scmi_perf_event_parameters *params;
    struct scmi_perf_level_get_p2a return_values_level;
    struct mod_dvfs_opp opp;

    /* request event to DVFS HAL */
    if (fwk_id_is_equal(event->id, scmi_perf_get_level)) {
        params = (struct scmi_perf_event_parameters *)event->params;

        status = perf_prot_ctx.scmi_perf_ctx->dvfs_api->get_current_opp(
            params->domain_id, &opp);
        if (status == FWK_SUCCESS) {
            /* DVFS value is ready */
            return_values_level = (struct scmi_perf_level_get_p2a){
                .status = SCMI_SUCCESS,
                .performance_level = opp.level,
            };

            scmi_perf_respond(
                &return_values_level,
                params->domain_id,
                (int)sizeof(return_values_level));

            return status;
        } else if (status == FWK_PENDING) {
            /* DVFS value will be provided through a response event */
            return FWK_SUCCESS;
        } else {
            return_values_level = (struct scmi_perf_level_get_p2a){
                .status = SCMI_HARDWARE_ERROR,
            };

            scmi_perf_respond(
                &return_values_level,
                params->domain_id,
                (int)sizeof(return_values_level.status));

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
        params_level = (struct mod_dvfs_params_response *)event->params;
        return_values_level = (struct scmi_perf_level_get_p2a){
            .status = params_level->status,
            .performance_level = params_level->performance_level,
        };
        scmi_perf_respond(
            &return_values_level,
            event->source_id,
            (return_values_level.status == SCMI_SUCCESS) ?
                (int)sizeof(return_values_level) :
                (int)sizeof(return_values_level.status));
    }

    return FWK_SUCCESS;
}

int perf_prot_ops_process_events(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    /* Request events from SCMI */
    if (fwk_id_get_module_idx(event->source_id) ==
        fwk_id_get_module_idx(fwk_module_id_scmi)) {
        return process_request_event(event);
    }

    /* Response events from DVFS */
    if (fwk_id_get_module_idx(event->source_id) ==
        fwk_id_get_module_idx(fwk_module_id_dvfs)) {
        return process_response_event(event);
    }

    return FWK_E_PARAM;
}

// EOF
