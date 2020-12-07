/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI performance domain management protocol support.
 */

#include <internal/scmi_perf.h>

#include <mod_dvfs.h>
#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
#    include <mod_resource_perms.h>
#endif
#include <mod_scmi.h>
#include <mod_scmi_perf.h>
#ifdef BUILD_HAS_STATISTICS
#include <mod_stats.h>
#endif
#include <mod_timer.h>

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

#define MOD_SCMI_PERF_NOTIFICATION_COUNT 2

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
#ifdef BUILD_HAS_FAST_CHANNELS
static int scmi_perf_describe_fast_channels(
    fwk_id_t service_id, const uint32_t *payload);
#endif
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_perf_limits_notify(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_perf_level_notify(
    fwk_id_t service_id, const uint32_t *payload);
#endif

static int (*handler_table[])(fwk_id_t, const uint32_t *) = {
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
#ifdef BUILD_HAS_FAST_CHANNELS
    [MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL] = scmi_perf_describe_fast_channels,
#endif
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_PERF_NOTIFY_LIMITS] = scmi_perf_limits_notify,
    [MOD_SCMI_PERF_NOTIFY_LEVEL] = scmi_perf_level_notify
#endif
};

static unsigned int payload_size_table[] = {
    [MOD_SCMI_PROTOCOL_VERSION] = 0,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        sizeof(struct scmi_protocol_message_attributes_a2p),
    [MOD_SCMI_PERF_DOMAIN_ATTRIBUTES] =
        sizeof(struct scmi_perf_domain_attributes_a2p),
    [MOD_SCMI_PERF_DESCRIBE_LEVELS] =
        sizeof(struct scmi_perf_describe_levels_a2p),
    [MOD_SCMI_PERF_LEVEL_SET] = sizeof(struct scmi_perf_level_set_a2p),
    [MOD_SCMI_PERF_LEVEL_GET] = sizeof(struct scmi_perf_level_get_a2p),
    [MOD_SCMI_PERF_LIMITS_SET] = sizeof(struct scmi_perf_limits_set_a2p),
    [MOD_SCMI_PERF_LIMITS_GET] = sizeof(struct scmi_perf_limits_get_a2p),
#ifdef BUILD_HAS_FAST_CHANNELS
    [MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL] =
        sizeof(struct scmi_perf_describe_fc_a2p),
#endif
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_PERF_NOTIFY_LIMITS] = sizeof(struct scmi_perf_notify_limits_a2p),
    [MOD_SCMI_PERF_NOTIFY_LEVEL] = sizeof(struct scmi_perf_notify_level_a2p)
#endif
};

#ifdef BUILD_HAS_FAST_CHANNELS
static unsigned int fast_channel_elem_size[] = {
    [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] = sizeof(uint32_t),
    [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
        sizeof(struct mod_scmi_perf_fast_channel_limit),
    [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] = sizeof(uint32_t),
    [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
        sizeof(struct mod_scmi_perf_fast_channel_limit)
};
#endif

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

#ifdef BUILD_HAS_STATISTICS
    /* Statistics module API */
    const struct mod_stats_api *stats_api;
#endif

    /* Pointer to a table of operations */
    struct perf_operations *perf_ops_table;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /* Number of active agents */
    int agent_count;

    /* SCMI notification API */
    const struct mod_scmi_notification_api *scmi_notification_api;
#endif
#ifdef BUILD_HAS_FAST_CHANNELS
    /* Alarm API for fast channels */
    const struct mod_timer_alarm_api *fc_alarm_api;

    /* Alarm for fast channels */
    fwk_id_t fast_channels_alarm_id;

    /* Fast Channels Polling Rate Limit */
    uint32_t fast_channels_rate_limit;
#endif

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    /* SCMI Resource Permissions API */
    const struct mod_res_permissions_api *res_perms_api;
#endif
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

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS

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

    if (parameters->domain_id >= scmi_perf_ctx.domain_count)
        return FWK_E_PARAM;

    *domain_id = parameters->domain_id;
    return FWK_SUCCESS;
}

static int scmi_perf_permissions_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    unsigned int message_id)
{
    enum mod_res_perms_permissions perms;
    unsigned int agent_id, domain_id;
    int status;

    status = scmi_perf_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return FWK_E_ACCESS;

    if (message_id < 3) {
        perms = scmi_perf_ctx.res_perms_api->agent_has_protocol_permission(
            agent_id, MOD_SCMI_PROTOCOL_ID_PERF);
        if (perms == MOD_RES_PERMS_ACCESS_ALLOWED)
            return FWK_SUCCESS;
        return FWK_E_ACCESS;
    }

    status = get_perf_domain_id(payload, &domain_id);
    if (status != FWK_SUCCESS)
        return FWK_E_PARAM;

    perms = scmi_perf_ctx.res_perms_api->agent_has_resource_permission(
        agent_id, MOD_SCMI_PROTOCOL_ID_PERF, message_id, domain_id);

    if (perms == MOD_RES_PERMS_ACCESS_ALLOWED)
        return FWK_SUCCESS;
    else
        return FWK_E_ACCESS;
}

#endif

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
    };
    uint32_t addr_low = 0, addr_high = 0, len = 0;

#ifdef BUILD_HAS_STATISTICS
    scmi_perf_ctx.stats_api->get_statistics_desc(
        fwk_module_id_scmi_perf, &addr_low, &addr_high, &len);
#endif

    return_values.statistics_len = len;
    return_values.statistics_address_low = addr_low;
    return_values.statistics_address_high = addr_high;

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
        };
    } else
        return_values.status = SCMI_NOT_FOUND;

    return_values.attributes = 0;
#ifdef BUILD_HAS_FAST_CHANNELS
    if ((parameters->message_id <= MOD_SCMI_PERF_LEVEL_GET) &&
        (parameters->message_id >= MOD_SCMI_PERF_LIMITS_SET))
        return_values.attributes = 1; /* Fast Channel available */
#endif

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
    const struct scmi_perf_domain_attributes_a2p *parameters;
    uint32_t permissions;
    fwk_id_t domain_id;
    struct mod_dvfs_opp opp;
    struct scmi_perf_domain_attributes_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };
    bool notifications = false;
    bool fast_channels = false;

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

#ifndef BUILD_HAS_RESOURCE_PERMISSIONS
    permissions =
        MOD_SCMI_PERF_PERMS_SET_LIMITS | MOD_SCMI_PERF_PERMS_SET_LEVEL;
#else
    status = scmi_perf_permissions_handler(
        service_id, payload, MOD_SCMI_PERF_LIMITS_SET);
    if (status == FWK_SUCCESS)
        permissions = MOD_SCMI_PERF_PERMS_SET_LIMITS;
    status = scmi_perf_permissions_handler(
        service_id, payload, MOD_SCMI_PERF_LEVEL_SET);
    if (status == FWK_SUCCESS)
        permissions |= MOD_SCMI_PERF_PERMS_SET_LEVEL;
#endif

    domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id);
    status = scmi_perf_ctx.dvfs_api->get_sustained_opp(domain_id, &opp);
    if (status != FWK_SUCCESS)
        goto exit;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    notifications = true;
#endif
#ifdef BUILD_HAS_FAST_CHANNELS
    const struct mod_scmi_perf_domain_config *domain =
        &(*scmi_perf_ctx.config->domains)[parameters->domain_id];

    if (domain->fast_channels_addr_scp != 0x0)
        fast_channels = true;
#endif
    return_values = (struct scmi_perf_domain_attributes_p2a){
        .status = SCMI_SUCCESS,
        .attributes = SCMI_PERF_DOMAIN_ATTRIBUTES(
            notifications,
            notifications,
            !!(permissions & MOD_SCMI_PERF_PERMS_SET_LEVEL),
            !!(permissions & MOD_SCMI_PERF_PERMS_SET_LIMITS),
            fast_channels),
        .rate_limit = 0, /* Unsupported */
        .sustained_freq = opp.frequency,
        .sustained_perf_level = opp.level,
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
        perf_level.performance_level = opp.level;
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
    const struct scmi_perf_limits_set_a2p *parameters;
    uint32_t range_min, range_max;
    fwk_id_t domain_id;
    struct scmi_perf_limits_set_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };
    enum mod_scmi_perf_policy_status policy_status;

    parameters = (const struct scmi_perf_limits_set_a2p *)payload;

    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    if (parameters->range_min > parameters->range_max) {
        return_values.status = SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id);
    range_min = parameters->range_min;
    range_max = parameters->range_max;

    status = scmi_perf_limits_set_policy(&policy_status, &range_min,
        &range_max, agent_id, domain_id);

    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_GENERIC_ERROR;
        goto exit;
    }
    if (policy_status == MOD_SCMI_PERF_SKIP_MESSAGE_HANDLER) {
        return_values.status = SCMI_SUCCESS;
        goto exit;
    }
    status = scmi_perf_ctx.dvfs_api->set_level_limits(
        domain_id,
        agent_id,
        &((struct mod_dvfs_level_limits){ .minimum = range_min,
                                          .maximum = range_max }));

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

    evt_params = (struct scmi_perf_event_parameters *)(void *)event.params;
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
    const struct scmi_perf_level_set_a2p *parameters;
    fwk_id_t domain_id;
    struct scmi_perf_level_set_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };
    uint32_t perf_level;
    enum mod_scmi_perf_policy_status policy_status;
    parameters = (const struct scmi_perf_level_set_a2p *)payload;

    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    status = scmi_perf_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    /*
     * Note that the policy handler may change the performance level
     */
    domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, parameters->domain_id);
    perf_level = parameters->performance_level;

    status = scmi_perf_level_set_policy(&policy_status, &perf_level, agent_id,
        domain_id);

    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_GENERIC_ERROR;
        goto exit;
    }
    if (policy_status == MOD_SCMI_PERF_SKIP_MESSAGE_HANDLER) {
        return_values.status = SCMI_SUCCESS;
        goto exit;
    }
    status = scmi_perf_ctx.dvfs_api->set_level(domain_id, agent_id, perf_level);

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

    evt_params = (struct scmi_perf_event_parameters *)(void *)event.params;
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

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_perf_limits_notify(fwk_id_t service_id,
                                   const uint32_t *payload)
{
    unsigned int agent_id;
    int status;
    unsigned int id;
    const struct scmi_perf_notify_limits_a2p *parameters;
    struct scmi_perf_notify_limits_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_perf_notify_limits_a2p *)(void *)payload;
    id = parameters->domain_id;
    if (id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    if ((parameters->notify_enable &
         ~SCMI_PERF_NOTIFY_LIMITS_NOTIFY_ENABLE_MASK) != 0x0) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_INVALID_PARAMETERS;

        goto exit;
    }

    status = scmi_perf_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    if (parameters->notify_enable)
        scmi_perf_ctx.scmi_notification_api->scmi_notification_add_subscriber(
            MOD_SCMI_PROTOCOL_ID_PERF,
            id,
            MOD_SCMI_PERF_NOTIFY_LIMITS,
            service_id);
    else
        scmi_perf_ctx.scmi_notification_api
            ->scmi_notification_remove_subscriber(
                MOD_SCMI_PROTOCOL_ID_PERF,
                agent_id,
                id,
                MOD_SCMI_PERF_NOTIFY_LIMITS);

    return_values.status = SCMI_SUCCESS;

exit:
    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_perf_level_notify(fwk_id_t service_id,
                                   const uint32_t *payload)
{
    unsigned int agent_id;
    int status;
    unsigned int id;
    const struct scmi_perf_notify_level_a2p *parameters;
    struct scmi_perf_notify_level_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_perf_notify_level_a2p *)(void *)payload;
    id = parameters->domain_id;
    if (id >= scmi_perf_ctx.domain_count) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    if ((parameters->notify_enable &
         ~SCMI_PERF_NOTIFY_LEVEL_NOTIFY_ENABLE_MASK) != 0x0) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_INVALID_PARAMETERS;

        goto exit;
    }

    id = parameters->domain_id;

    status = scmi_perf_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    if (parameters->notify_enable)
        scmi_perf_ctx.scmi_notification_api->scmi_notification_add_subscriber(
            MOD_SCMI_PROTOCOL_ID_PERF,
            id,
            MOD_SCMI_PERF_NOTIFY_LEVEL,
            service_id);
    else
        scmi_perf_ctx.scmi_notification_api
            ->scmi_notification_remove_subscriber(
                MOD_SCMI_PROTOCOL_ID_PERF,
                agent_id,
                id,
                MOD_SCMI_PERF_NOTIFY_LEVEL);

    return_values.status = SCMI_SUCCESS;

exit:
    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}
#endif

#ifdef BUILD_HAS_FAST_CHANNELS

/*
 * Note that the Fast Channel doorbell is not supported in this
 * implementation.
 */
static int scmi_perf_describe_fast_channels(fwk_id_t service_id,
                                            const uint32_t *payload)
{
    const struct mod_scmi_perf_domain_config *domain;
    const struct scmi_perf_describe_fc_a2p *parameters;
    struct scmi_perf_describe_fc_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };
    uint32_t chan_size, chan_index;

    parameters = (const struct scmi_perf_describe_fc_a2p *)(void *)payload;

    /* Validate the domain identifier */
    if (parameters->domain_id >= scmi_perf_ctx.domain_count) {
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    domain = &(*scmi_perf_ctx.config->domains)[parameters->domain_id];

    if (domain->fast_channels_addr_scp == 0x0) {
        return_values.status = SCMI_NOT_SUPPORTED;

        goto exit;
    }

    if (parameters->message_id >= MOD_SCMI_PERF_COMMAND_COUNT) {
        return_values.status = SCMI_NOT_FOUND;

        goto exit;
    }

    switch (parameters->message_id) {
    case MOD_SCMI_PERF_LEVEL_GET:
        chan_index = MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET;
        chan_size =
            fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET];
        break;

    case MOD_SCMI_PERF_LEVEL_SET:
        chan_index = MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET;
        chan_size =
            fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET];
        break;

    case MOD_SCMI_PERF_LIMITS_SET:
        chan_index = MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET;
        chan_size =
            fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET];
        break;

    case MOD_SCMI_PERF_LIMITS_GET:
        chan_index = MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET;
        chan_size =
            fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET];
        break;

    default:
        return_values.status = SCMI_NOT_SUPPORTED;
        goto exit;

    }
    if (domain->fast_channels_addr_ap == 0x0 ||
        domain->fast_channels_addr_ap[chan_index] == 0x0) {
        return_values.status = SCMI_NOT_SUPPORTED;
        goto exit;
    }
    return_values.status = SCMI_SUCCESS;
    return_values.attributes = 0; /* Doorbell not supported */
    return_values.rate_limit = scmi_perf_ctx.fast_channels_rate_limit;
    return_values.chan_addr_low =
        (uint32_t)(domain->fast_channels_addr_ap[chan_index] & ~0UL);
    return_values.chan_addr_high =
        (domain->fast_channels_addr_ap[chan_index] >> 32);
    return_values.chan_size = chan_size;

exit:
    scmi_perf_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return FWK_SUCCESS;
}

/*
 * Fast Channel Polling
 */
static void fast_channel_callback(uintptr_t param)
{
    const struct mod_scmi_perf_domain_config *domain;
    struct mod_scmi_perf_fast_channel_limit *set_limit;
    uint32_t *set_level;
    unsigned int i;

    for (i = 0; i < scmi_perf_ctx.domain_count; i++) {
        domain = &(*scmi_perf_ctx.config->domains)[i];
        if (domain->fast_channels_addr_scp != 0x0) {
            set_limit = (struct mod_scmi_perf_fast_channel_limit
                             *)((uintptr_t)domain->fast_channels_addr_scp
                                    [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET]);
            set_level =
                (uint32_t *)((uintptr_t)domain->fast_channels_addr_scp
                                 [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET]);
            /*
             * Check for set_level
             */
            if (set_level != 0x0 && *set_level != 0) {
                scmi_perf_ctx.dvfs_api->set_level(
                    FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, i), 0, *set_level);
            }
            if (set_limit != 0) {
                if ((set_limit->range_max == 0) && (set_limit->range_min == 0))
                    continue;
                scmi_perf_ctx.dvfs_api->set_level_limits(
                    FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, i),
                    0,
                    &((struct mod_dvfs_level_limits){
                        .minimum = set_limit->range_min,
                        .maximum = set_limit->range_max,
                    }));
            }
        }
    }
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

static int scmi_perf_get_scmi_protocol_id(fwk_id_t protocol_id,
                                          uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = MOD_SCMI_PROTOCOL_ID_PERF;

    return FWK_SUCCESS;
}

static int scmi_perf_message_handler(fwk_id_t protocol_id, fwk_id_t service_id,
    const uint32_t *payload, size_t payload_size, unsigned int message_id)
{
    int32_t return_value;
#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    int status;
#endif

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Performance management protocol table sizes not consistent");
    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return_value = SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    status = scmi_perf_permissions_handler(service_id, payload, message_id);
    if (status != FWK_SUCCESS) {
        if (status == FWK_E_PARAM)
            return_value = SCMI_NOT_FOUND;
        else
            return_value = SCMI_DENIED;
        goto error;
    }
#endif

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
 * A domain limits range has been updated. Depending on the system
 * configuration we may send an SCMI notification to the agents which
 * have registered for these notifications and/or update the associated
 * fast channels.
 */
static void scmi_perf_notify_limits_updated(
    fwk_id_t domain_id,
    uintptr_t cookie,
    uint32_t range_min,
    uint32_t range_max)
{
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    struct scmi_perf_limits_changed limits_changed;
#endif
    int idx;
    const struct mod_scmi_perf_domain_config *domain;
    struct mod_scmi_perf_fast_channel_limit *get_limit;

    idx = fwk_id_get_element_idx(domain_id);

    domain = &(*scmi_perf_ctx.config->domains)[idx];
    if (domain->fast_channels_addr_scp != 0x0) {
        get_limit = (struct mod_scmi_perf_fast_channel_limit
                         *)((uintptr_t)domain->fast_channels_addr_scp
                                [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET]);
        if (get_limit != 0x0) { /* note: get_limit may not be defined */
            get_limit->range_max = range_max;
            get_limit->range_min = range_min;
        }
    }

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    limits_changed.agent_id = (uint32_t)cookie;
    limits_changed.domain_id = idx;
    limits_changed.range_min = range_min;
    limits_changed.range_max = range_max;

    scmi_perf_ctx.scmi_notification_api->scmi_notification_notify(
        MOD_SCMI_PROTOCOL_ID_PERF,
        MOD_SCMI_PERF_NOTIFY_LIMITS,
        SCMI_PERF_LIMITS_CHANGED,
        &limits_changed,
        sizeof(limits_changed));
#endif
}

/*
 * A domain performance level has been updated. Depending on the system
 * configuration we may send an SCMI notification to the agents which
 * have registered for these notifications and/or update the associated
 * fast channels.
 */
static void scmi_perf_notify_level_updated(
    fwk_id_t domain_id,
    uintptr_t cookie,
    uint32_t level)
{
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    struct scmi_perf_level_changed level_changed;
#endif
    int idx;
    const struct mod_scmi_perf_domain_config *domain;
    uint32_t *get_level;
#ifdef BUILD_HAS_STATISTICS
    size_t level_id;
    int status;
#endif

    idx = fwk_id_get_element_idx(domain_id);

#ifdef BUILD_HAS_STATISTICS
    status = scmi_perf_ctx.dvfs_api->get_level_id(domain_id, level, &level_id);
    if (status == FWK_SUCCESS)
        scmi_perf_ctx.stats_api->update_domain(fwk_module_id_scmi_perf,
            FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_PERF, idx), level_id);
#endif

    domain = &(*scmi_perf_ctx.config->domains)[idx];
    if (domain->fast_channels_addr_scp != 0x0) {
        get_level = (uint32_t *)((uintptr_t)domain->fast_channels_addr_scp
                                     [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET]);
        if (get_level != 0x0) /* note: get_level may not be defined */
            *get_level = level;
    }

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    level_changed.agent_id = (uint32_t)cookie;
    level_changed.domain_id = idx;
    level_changed.performance_level = level;

    scmi_perf_ctx.scmi_notification_api->scmi_notification_notify(
        MOD_SCMI_PROTOCOL_ID_PERF,
        MOD_SCMI_PERF_NOTIFY_LEVEL,
        SCMI_PERF_LEVEL_CHANGED,
        &level_changed,
        sizeof(level_changed));
#endif
}

static struct mod_dvfs_perf_updated_api perf_update_api = {
    .notify_limits_updated = scmi_perf_notify_limits_updated,
    .notify_level_updated = scmi_perf_notify_level_updated,
};

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
#ifdef BUILD_HAS_FAST_CHANNELS
    scmi_perf_ctx.fast_channels_alarm_id = config->fast_channels_alarm_id;
    if (config->fast_channels_rate_limit < SCMI_PERF_FC_MIN_RATE_LIMIT)
        scmi_perf_ctx.fast_channels_rate_limit = SCMI_PERF_FC_MIN_RATE_LIMIT;
    else
        scmi_perf_ctx.fast_channels_rate_limit =
            config->fast_channels_rate_limit;
#endif

    /* Initialize table */
    for (i = 0; i < return_val; i++)
        scmi_perf_ctx.perf_ops_table[i].service_id = FWK_ID_NONE;


    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_init_notifications(int domains)
{
    int status;

    status = scmi_perf_ctx.scmi_api->get_agent_count(
        &scmi_perf_ctx.agent_count);
    if (status != FWK_SUCCESS)
        return status;

    fwk_assert(scmi_perf_ctx.agent_count != 0);

    status = scmi_perf_ctx.scmi_notification_api->scmi_notification_init(
        MOD_SCMI_PROTOCOL_ID_PERF,
        scmi_perf_ctx.agent_count,
        domains,
        MOD_SCMI_PERF_NOTIFICATION_COUNT);

    return status;
}
#endif

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

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_NOTIFICATION),
        &scmi_perf_ctx.scmi_notification_api);
    if (status != FWK_SUCCESS)
        return status;
#endif

#ifdef BUILD_HAS_FAST_CHANNELS
    if (!fwk_id_is_equal(scmi_perf_ctx.config->fast_channels_alarm_id,
        FWK_ID_NONE)) {
        status = fwk_module_bind(scmi_perf_ctx.config->fast_channels_alarm_id,
            MOD_TIMER_API_ID_ALARM, &scmi_perf_ctx.fc_alarm_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }
#endif

#ifdef BUILD_HAS_STATISTICS
    if (scmi_perf_ctx.config->stats_enabled) {
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_STATISTICS),
            FWK_ID_API(FWK_MODULE_IDX_STATISTICS, MOD_STATS_API_IDX_STATS),
            &scmi_perf_ctx.stats_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }
#endif

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &scmi_perf_ctx.res_perms_api);
    if (status != FWK_SUCCESS)
        return status;
#endif

    return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_DVFS),
        FWK_ID_API(FWK_MODULE_IDX_DVFS, 0), &scmi_perf_ctx.dvfs_api);
}

static int scmi_perf_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_SCMI_PERF_PROTOCOL_API:
        *api = &scmi_perf_mod_scmi_to_protocol_api;
        break;

    case MOD_SCMI_PERF_DVFS_UPDATE_API:
        *api = &perf_update_api;
        break;

    default:
        return FWK_E_ACCESS;
    }

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_STATISTICS
static int scmi_perf_stats_start(void)
{
    const struct mod_scmi_perf_domain_config *domain;
    int status = FWK_SUCCESS;
    int stats_domains = 0;
    unsigned int i;

    if (!scmi_perf_ctx.config->stats_enabled)
        return FWK_E_SUPPORT;

    /* Count how many domains have statistics */
    for (i = 0; i < scmi_perf_ctx.domain_count; i++) {
        domain = &(*scmi_perf_ctx.config->domains)[i];
        if (domain->stats_collected)
            stats_domains++;
    }

    status = scmi_perf_ctx.stats_api->init_stats(fwk_module_id_scmi_perf,
        scmi_perf_ctx.domain_count, stats_domains);

    if (status != FWK_SUCCESS)
        return status;

    for (i = 0; i < scmi_perf_ctx.domain_count; i++) {
        domain = &(*scmi_perf_ctx.config->domains)[i];
        /* Add this domain to track statistics when needed */
        if (domain->stats_collected) {
            fwk_id_t domain_id;
            size_t opp_count;

            domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, i);
            status = scmi_perf_ctx.dvfs_api->get_opp_count(domain_id,
                &opp_count);

            if (status != FWK_SUCCESS)
                return status;

            status = scmi_perf_ctx.stats_api->add_domain(
                fwk_module_id_scmi_perf,
                FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_PERF, i),
                (int)opp_count);

            if (status != FWK_SUCCESS)
                return status;
        }
    }

    return scmi_perf_ctx.stats_api->start_stats(fwk_module_id_scmi_perf);
}
#endif

static int scmi_perf_start(fwk_id_t id)
{
    int status = FWK_SUCCESS;

#ifdef BUILD_HAS_FAST_CHANNELS

    const struct mod_scmi_perf_domain_config *domain;
    unsigned int i, j;
    void *fc_elem;
    uint32_t fc_interval_msecs;

    /*
     * Set up the Fast Channel polling if required
     */
    if (!fwk_id_is_equal(scmi_perf_ctx.config->fast_channels_alarm_id,
        FWK_ID_NONE)) {
        if (scmi_perf_ctx.config->fast_channels_rate_limit <
            SCMI_PERF_FC_MIN_RATE_LIMIT)
            fc_interval_msecs = (uint32_t)SCMI_PERF_FC_MIN_RATE_LIMIT / 1000;
        else
            fc_interval_msecs = (uint32_t)
            scmi_perf_ctx.config->fast_channels_rate_limit / 1000;
        status = scmi_perf_ctx.fc_alarm_api->start(
            scmi_perf_ctx.config->fast_channels_alarm_id,
            fc_interval_msecs, MOD_TIMER_ALARM_TYPE_PERIODIC,
            fast_channel_callback, (uintptr_t)0);
        if (status != FWK_SUCCESS)
            return status;
    }

    for (i = 0; i < scmi_perf_ctx.domain_count; i++) {
        domain = &(*scmi_perf_ctx.config->domains)[i];
        if (domain->fast_channels_addr_scp != 0x0) {
            for (j = 0; j < MOD_SCMI_PERF_FAST_CHANNEL_ADDR_INDEX_COUNT; j++) {
                fc_elem = (void *)(uintptr_t)domain->fast_channels_addr_scp[j];
                if (fc_elem != 0x0)
                    memset(fc_elem, 0, fast_channel_elem_size[j]);
            }
        }
    }
#endif

#ifdef BUILD_HAS_STATISTICS
    status = scmi_perf_stats_start();
    if (status != FWK_SUCCESS)
        return status;
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    status = scmi_init_notifications(scmi_perf_ctx.domain_count);
    if (status != FWK_SUCCESS)
        return status;
#endif

    return status;
}

/*
 * Handle a request for get_level/limits.
 */
static int process_request_event(const struct fwk_event *event)
{
    int status;
    struct mod_dvfs_level_limits limits;
    struct scmi_perf_event_parameters *params;
    struct scmi_perf_level_get_p2a return_values_level;
    struct scmi_perf_limits_get_p2a return_values_limits;
    struct mod_dvfs_opp opp;

    /* request event to DVFS HAL */
    if (fwk_id_is_equal(event->id, scmi_perf_get_level)) {
        params = (struct scmi_perf_event_parameters *)(void *)event->params;

        status = scmi_perf_ctx.dvfs_api->get_current_opp(params->domain_id,
                                                       &opp);
        if (status == FWK_SUCCESS) {
            /* DVFS value is ready */
            return_values_level = (struct scmi_perf_level_get_p2a){
                .status = SCMI_SUCCESS,
                .performance_level = opp.level,
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
        params = (struct scmi_perf_event_parameters *)(void *)event->params;

        status = scmi_perf_ctx.dvfs_api->get_level_limits(
            params->domain_id, &limits);
        if (status == FWK_SUCCESS) {
            /* DVFS value is ready */
            return_values_limits = (struct scmi_perf_limits_get_p2a){
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
            (void *)event->params;
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
    .api_count = MOD_SCMI_PERF_API_COUNT,
    .event_count = SCMI_PERF_EVENT_IDX_COUNT,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_perf_init,
    .bind = scmi_perf_bind,
    .start = scmi_perf_start,
    .process_bind_request = scmi_perf_process_bind_request,
    .process_event = scmi_perf_process_event,
};
