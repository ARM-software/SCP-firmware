/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI power domain management protocol support.
 */

#include <internal/scmi_power_domain.h>

#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_scmi_power_domain.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
#    include <mod_resource_perms.h>
#endif

#ifdef BUILD_HAS_MOD_DEBUG
#    include <mod_debug.h>
#endif

#define MOD_SCMI_PD_NOTIFICATION_COUNT 2

struct scmi_pd_operations {
    /*
     * Service identifier currently requesting operation.
     * A 'none' value means that there is no pending request.
     */
    fwk_id_t service_id;

    /* Track agent requesting the pd operation */
    unsigned int agent_id;
};

struct mod_scmi_pd_ctx {
    /* Number of power domains */
    unsigned int domain_count;

    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* Power domain module API */
    const struct mod_pd_restricted_api *pd_api;

#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    /* Module config */
    const struct mod_scmi_pd_config *config;
#endif

#ifdef BUILD_HAS_MOD_DEBUG
    /* Debug module API */
    const struct mod_debug_api *debug_api;

    /* Debug device identifier */
    fwk_id_t debug_id;

    /* Debug Power Domain element identifier */
    fwk_id_t debug_pd_id;

    /* Holds debug state change notification state */
    bool debug_pd_state_notification_enabled;
#endif

    /* Pointer to a table of scmi_pd operations */
    struct scmi_pd_operations *ops;

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    /* SCMI Resource Permissions API */
    const struct mod_res_permissions_api *res_perms_api;
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /* Number of active agents */
    unsigned int agent_count;

    /* SCMI notification API */
    const struct mod_scmi_notification_api *scmi_notification_api;
#endif
};

static int scmi_pd_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_protocol_message_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_power_domain_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_power_state_set_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_power_state_get_handler(fwk_id_t service_id,
    const uint32_t *payload);

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_pd_power_state_notify_handler(
    enum scmi_pd_command_id command_id,
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_power_state_changed_notify_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_power_state_change_requested_notify_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
#endif

enum scmi_pd_event_idx {
    /* Event to handle sync set state */
    SCMI_PD_EVENT_IDX_SET_STATE,
#ifdef BUILD_HAS_MOD_DEBUG
    /* Event used prior to send a set_enabled request to debug HAL. */
    SCMI_PD_EVENT_IDX_DEBUG_SET,

    /* Event used prior to send a get_enabled request to debug HAL. */
    SCMI_PD_EVENT_IDX_DEBUG_GET,
#endif
    SCMI_PD_EVENT_IDX_COUNT,
};

struct event_request_params {
    unsigned int pd_power_state;
    fwk_id_t pd_id;
};

#ifdef BUILD_HAS_MOD_DEBUG
static const fwk_id_t mod_scmi_pd_event_id_dbg_enable_set =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
                      SCMI_PD_EVENT_IDX_DEBUG_SET);

static const fwk_id_t mod_scmi_pd_event_id_dbg_enable_get =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
                      SCMI_PD_EVENT_IDX_DEBUG_GET);
#endif

static const fwk_id_t mod_scmi_pd_event_id_set_request = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
    SCMI_PD_EVENT_IDX_SET_STATE);

/*
 * Internal variables
 */

static struct mod_scmi_pd_ctx scmi_pd_ctx;

static handler_table_t handler_table[MOD_SCMI_PD_POWER_COMMAND_COUNT] = {
    [MOD_SCMI_PROTOCOL_VERSION] = scmi_pd_protocol_version_handler,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = scmi_pd_protocol_attributes_handler,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_pd_protocol_message_attributes_handler,
    [MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES] =
        scmi_pd_power_domain_attributes_handler,
    [MOD_SCMI_PD_POWER_STATE_SET] = scmi_pd_power_state_set_handler,
    [MOD_SCMI_PD_POWER_STATE_GET] = scmi_pd_power_state_get_handler,

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_PD_POWER_STATE_NOTIFY] =
        scmi_pd_power_state_changed_notify_handler,
    [MOD_SCMI_PD_POWER_STATE_CHANGE_REQUESTED_NOTIFY] =
        scmi_pd_power_state_change_requested_notify_handler,
#endif
};

static size_t payload_size_table[MOD_SCMI_PD_POWER_COMMAND_COUNT] = {
    [MOD_SCMI_PROTOCOL_VERSION] = 0,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        (unsigned int)sizeof(struct scmi_protocol_message_attributes_a2p),
    [MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES] =
        (unsigned int)sizeof(struct scmi_pd_power_domain_attributes_a2p),
    [MOD_SCMI_PD_POWER_STATE_SET] =
        (unsigned int)sizeof(struct scmi_pd_power_state_set_a2p),
    [MOD_SCMI_PD_POWER_STATE_GET] =
        (unsigned int)sizeof(struct scmi_pd_power_state_get_a2p),

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_PD_POWER_STATE_NOTIFY] =
        sizeof(struct scmi_pd_power_state_notify_a2p),
    [MOD_SCMI_PD_POWER_STATE_CHANGE_REQUESTED_NOTIFY] =
        sizeof(struct scmi_pd_power_state_notify_a2p),
#endif
};

static uint32_t pd_state_to_scmi_dev_state[MOD_PD_STATE_COUNT] = {
    [MOD_PD_STATE_OFF] =
        SCMI_PD_DEVICE_STATE_ID_OFF | SCMI_PD_DEVICE_STATE_TYPE,
    [MOD_PD_STATE_ON] = SCMI_PD_DEVICE_STATE_ID_ON,
    /* In case of more supported device states review the map functions */
};

/*
 * Helpers
 */

static bool ops_is_busy(fwk_id_t pd_id)
{
    unsigned int pd_idx = fwk_id_get_element_idx(pd_id);

    return !fwk_id_is_equal(scmi_pd_ctx.ops[pd_idx].service_id, FWK_ID_NONE);
}

static void ops_set_busy(fwk_id_t pd_id, fwk_id_t service_id)
{
    unsigned int pd_idx = fwk_id_get_element_idx(pd_id);

    scmi_pd_ctx.ops[pd_idx].service_id = service_id;
}

static fwk_id_t ops_get_service(fwk_id_t pd_id)
{
    unsigned int pd_idx = fwk_id_get_element_idx(pd_id);

    return scmi_pd_ctx.ops[pd_idx].service_id;
}

static void ops_set_agent_id(fwk_id_t pd_id, unsigned int agent_id)
{
    unsigned int pd_idx = fwk_id_get_element_idx(pd_id);
    scmi_pd_ctx.ops[pd_idx].agent_id = agent_id;
}

#ifdef BUILD_HAS_MOD_DEBUG
static unsigned int ops_get_agent_id(fwk_id_t pd_id)
{
    unsigned int pd_idx = fwk_id_get_element_idx(pd_id);

    return scmi_pd_ctx.ops[pd_idx].agent_id;
}
#endif

static void ops_set_idle(fwk_id_t pd_id)
{
    unsigned int pd_idx = fwk_id_get_element_idx(pd_id);

    scmi_pd_ctx.ops[pd_idx].service_id = FWK_ID_NONE;
}

#if defined(BUILD_HAS_MOD_DEBUG) && defined(BUILD_HAS_SCMI_NOTIFICATIONS)
static unsigned int find_agent_scmi_pd_index(
    unsigned int agent_id,
    unsigned int power_domain_index)
{
#    ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    const struct mod_scmi_pd_agent_config *agent_ctx =
        &scmi_pd_ctx.config->agent_config_table[agent_id];
    fwk_id_t pd_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, power_domain_index);

    /* Find the relative power domain index for the agent. */
    for (unsigned int i = 0; i < agent_ctx->domain_count; ++i) {
        if (agent_ctx->domains[i] == power_domain_index) {
            return i;
        }
    }
#    endif
    return power_domain_index;
}
#endif

static int scmi_pd_get_domain_id(
    unsigned int agent_id,
    unsigned int req_domain_idx,
    fwk_id_t *out_domain_id)
{
    unsigned int domain_idx;
#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    if (req_domain_idx >=
        scmi_pd_ctx.config->agent_config_table[agent_id].domain_count) {
        return FWK_E_RANGE;
    }
    domain_idx = scmi_pd_ctx.config->agent_config_table[agent_id]
                     .domains[req_domain_idx];
#else
    if (req_domain_idx >= scmi_pd_ctx.domain_count) {
        return FWK_E_RANGE;
    }
    domain_idx = req_domain_idx;
#endif
    *out_domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_idx);

    if (!fwk_module_is_valid_element_id(*out_domain_id)) {
        return FWK_E_RANGE;
    }

    return FWK_SUCCESS;
}

/*
 * Power domain management protocol implementation
 */

static int pd_state_to_scmi_device_state(unsigned int pd_state,
                                         uint32_t *scmi_state)
{
    if (pd_state == MOD_PD_STATE_OFF || pd_state == MOD_PD_STATE_ON) {
        *scmi_state = pd_state_to_scmi_dev_state[pd_state];
    } else {
        *scmi_state = pd_state;
    }

    return FWK_SUCCESS;
}

static int scmi_pd_protocol_version_handler(fwk_id_t service_id,
                                            const uint32_t *payload)
{
    int respond_status;
    struct scmi_protocol_version_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_POWER_DOMAIN,
    };

    respond_status = scmi_pd_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));

    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
    }

    return FWK_SUCCESS;
}

static int scmi_pd_protocol_attributes_handler(fwk_id_t service_id,
                                               const uint32_t *payload)
{
    int status;
    unsigned int agent_id;
    struct scmi_pd_protocol_attributes_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
    };

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        goto exit;
    }

#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    return_values.attributes =
        scmi_pd_ctx.config->agent_config_table[agent_id].domain_count;
#else
    return_values.attributes = scmi_pd_ctx.domain_count;
#endif

exit:
    return scmi_pd_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_pd_power_domain_attributes_handler(fwk_id_t service_id,
                                                   const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    int respond_status;
    const struct scmi_pd_power_domain_attributes_a2p *parameters;
    enum mod_pd_type pd_type;
    unsigned int domain_idx;
    fwk_id_t pd_id;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;
    struct scmi_pd_power_domain_attributes_p2a return_values = {
        .status = (int32_t)SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_pd_power_domain_attributes_a2p *)payload;

    domain_idx = parameters->domain_id;
    if (domain_idx > UINT16_MAX) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    status = scmi_pd_get_domain_id(agent_id, parameters->domain_id, &pd_id);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.scmi_api->get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    switch (pd_type) {
    case MOD_PD_TYPE_CORE:
        /*
         * For core power domains, the POWER_STATE_SET command is supported
         * only asynchronously for the PSCI agent. In all other cases, reply
         * that the command is not supported either synchronously nor
         * asynchronously.
         */
        if (agent_type == SCMI_AGENT_TYPE_PSCI) {
            return_values.attributes = SCMI_PD_POWER_STATE_SET_ASYNC;
        }
        break;

    case MOD_PD_TYPE_CLUSTER:
        /*
         * For cluster power domains, the POWER_STATE_SET command is supported
         * only synchronously for the PSCI agent. In all other cases, reply
         * that the command is not supported either synchronously nor
         * asynchronously.
         */
        if (agent_type == SCMI_AGENT_TYPE_PSCI) {
            return_values.attributes = SCMI_PD_POWER_STATE_SET_SYNC;
        }
        break;

    case MOD_PD_TYPE_DEVICE_DEBUG:
    case MOD_PD_TYPE_DEVICE:
        /*
         * Support only synchronous POWER_STATE_SET for devices for any agent.
         * Notifications are also supported for this power domain type.
         */
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
        return_values.attributes = SCMI_PD_POWER_STATE_SET_SYNC |
            SCMI_PD_POWER_STATE_CHANGE_NOTIFICATIONS;
#else
        return_values.attributes = SCMI_PD_POWER_STATE_SET_SYNC;
#endif
        break;

    case MOD_PD_TYPE_SYSTEM:
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        /* Fallthrough. */

    default:
        goto exit;
    }

    fwk_str_strncpy(
        (char *)return_values.name,
        fwk_module_get_element_name(pd_id),
        sizeof(return_values.name) - 1);

    return_values.status = (int32_t)SCMI_SUCCESS;

exit:
    respond_status = scmi_pd_ctx.scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));

    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
    }

    return status;
}

static int scmi_pd_protocol_message_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *parameters;
    struct scmi_protocol_message_attributes_p2a return_values = {
        .status = (int32_t)SCMI_NOT_FOUND,
    };

    parameters = (const struct scmi_protocol_message_attributes_a2p *)
                  payload;

    if ((parameters->message_id < FWK_ARRAY_SIZE(handler_table)) &&
        (handler_table[parameters->message_id] != NULL)) {
        return_values.status = (int32_t)SCMI_SUCCESS;
    }

    return scmi_pd_ctx.scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
}

#ifdef BUILD_HAS_MOD_DEBUG
static void scmi_pd_power_state_notify(
    enum scmi_pd_command_id command_id,
    enum scmi_pd_notification_id notification_message_id,
    unsigned int domain_id,
    unsigned int agent_id,
    uint32_t power_state)
{
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    struct scmi_pd_power_state_notification_message_p2a message;
    int status;

    message.agent_id = agent_id;
    message.domain_id = find_agent_scmi_pd_index(agent_id, domain_id);
    message.power_state = power_state;

    status = scmi_pd_ctx.scmi_notification_api->scmi_notification_notify(
        MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
        command_id,
        notification_message_id,
        &message,
        sizeof(message));
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
    }
#endif
}
#endif

static int32_t scmi_pd_agent_check(fwk_id_t service_id, unsigned int *agent_idx)
{
    int status;
    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, agent_idx);
    if (status != FWK_SUCCESS) {
        return (int32_t)SCMI_INVALID_PARAMETERS;
    }

    return (int32_t)SCMI_SUCCESS;
}

static int32_t scmi_pd_power_state_set_parameters_check(
    const struct scmi_pd_power_state_set_a2p *scmi_params,
    unsigned int agent_id,
    fwk_id_t *pd_id)
{
    int status;

    if (((scmi_params->flags & ~SCMI_PD_POWER_STATE_SET_FLAGS_MASK) != 0x0U) ||
        ((scmi_params->power_state &
          ~SCMI_PD_POWER_STATE_SET_POWER_STATE_MASK) != 0x0U)) {
        return (int32_t)SCMI_INVALID_PARAMETERS;
    }

    if (scmi_params->domain_id > UINT16_MAX) {
        return (int32_t)SCMI_NOT_FOUND;
    }

    status = scmi_pd_get_domain_id(agent_id, scmi_params->domain_id, pd_id);
    if (status != FWK_SUCCESS) {
        return (int32_t)SCMI_NOT_FOUND;
    }

    return (int32_t)SCMI_SUCCESS;
}

static int32_t scmi_pd_power_state_set_type_check(
    fwk_id_t service_id,
    fwk_id_t pd_id,
    unsigned int *agent_idx,
    enum mod_pd_type *pd_type)
{
    enum scmi_agent_type agent_type;
    int status;

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, agent_idx);
    if (status != FWK_SUCCESS) {
        return (int32_t)SCMI_INVALID_PARAMETERS;
    }

    status = scmi_pd_ctx.scmi_api->get_agent_type(*agent_idx, &agent_type);
    if (status != FWK_SUCCESS) {
        return (int32_t)SCMI_INVALID_PARAMETERS;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, pd_type);
    if (status != FWK_SUCCESS) {
        return (int32_t)SCMI_INVALID_PARAMETERS;
    }

    if (((*pd_type == MOD_PD_TYPE_CORE) || (*pd_type == MOD_PD_TYPE_CLUSTER)) &&
        (agent_type != SCMI_AGENT_TYPE_PSCI)) {
        return (int32_t)SCMI_NOT_SUPPORTED;
    }

    return (int32_t)SCMI_SUCCESS;
}

static int scmi_pd_power_state_set_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_pd_power_state_set_a2p *scmi_params;
    struct scmi_pd_power_state_set_p2a return_values;
    fwk_id_t pd_id;
    enum mod_pd_type pd_type;
    unsigned int agent_idx;
    int status;
    int32_t scmi_return;
    uint32_t power_state;
    enum mod_scmi_pd_policy_status policy_status;
    bool is_sync;

    scmi_params = (const struct scmi_pd_power_state_set_a2p *)payload;

    /* Agent checking */
    scmi_return = scmi_pd_agent_check(service_id, &agent_idx);
    if (scmi_return != SCMI_SUCCESS) {
        return_values.status = scmi_return;
        return scmi_pd_ctx.scmi_api->respond(
            service_id, &return_values, sizeof(return_values.status));
    }

    /* Parameters checking */
    scmi_return = scmi_pd_power_state_set_parameters_check(
        scmi_params, agent_idx, &pd_id);
    if (scmi_return != SCMI_SUCCESS) {
        return_values.status = scmi_return;
        return scmi_pd_ctx.scmi_api->respond(
            service_id, &return_values, sizeof(return_values.status));
    }

    /* Type checking */
    scmi_return = scmi_pd_power_state_set_type_check(
        service_id, pd_id, &agent_idx, &pd_type);
    if (scmi_return != SCMI_SUCCESS) {
        return_values.status = scmi_return;
        return scmi_pd_ctx.scmi_api->respond(
            service_id, &return_values, sizeof(return_values.status));
    }

    is_sync =
        ((scmi_params->flags & SCMI_PD_POWER_STATE_SET_ASYNC_FLAG_MASK) ==
         (uint32_t)0);

    /* Cluster and Device_debug pd types supports sync requests only*/
    if (((pd_type == MOD_PD_TYPE_CLUSTER) ||
         (pd_type == MOD_PD_TYPE_DEVICE_DEBUG)) &&
        (!is_sync)) {
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;
        return scmi_pd_ctx.scmi_api->respond(
            service_id, &return_values, sizeof(return_values.status));
    } else if (pd_type == MOD_PD_TYPE_CORE) {
        /*
         * Async/sync flag is ignored for core power domains as stated
         * by the specification.
         */
        is_sync = false;
    } else {
        /* Other pd types if any */
    }

    power_state = scmi_params->power_state;
    status = scmi_pd_power_state_set_policy(
        &policy_status, &power_state, agent_idx, pd_id);

    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_GENERIC_ERROR;
        return scmi_pd_ctx.scmi_api->respond(
            service_id, &return_values, sizeof(return_values.status));
    }

    if (policy_status == MOD_SCMI_PD_SKIP_MESSAGE_HANDLER) {
        return_values.status = (int32_t)SCMI_SUCCESS;
        return scmi_pd_ctx.scmi_api->respond(
            service_id, &return_values, sizeof(return_values));
    }

    if (!is_sync) {
        /*
         * For a power domain that is managed asynchronously, schedule the
         * request and respond to the agent immediately.
         */
        status = scmi_pd_ctx.pd_api->set_state(pd_id, false, power_state);
        if (status == FWK_SUCCESS) {
            return_values.status = (int32_t)SCMI_SUCCESS;
            return scmi_pd_ctx.scmi_api->respond(
                service_id, &return_values, sizeof(return_values));
        } else {
            return_values.status = (int32_t)SCMI_GENERIC_ERROR;
            return scmi_pd_ctx.scmi_api->respond(
                service_id, &return_values, sizeof(return_values.status));
        }
    }

    /* Sync request handling */
    if (ops_is_busy(pd_id)) {
        return_values.status = (int32_t)SCMI_BUSY;
        int respond_status = scmi_pd_ctx.scmi_api->respond(
            service_id, &return_values, sizeof(return_values.status));

        if (respond_status != FWK_SUCCESS) {
            FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
        }
        return FWK_E_BUSY;
    }
    ops_set_busy(pd_id, service_id);
    ops_set_agent_id(pd_id, agent_idx);

    /* Send the event to handle the request in the scmi_pd context */
    struct fwk_event event = {
        .target_id = fwk_module_id_scmi_power_domain,
        .id = mod_scmi_pd_event_id_set_request,
    };

#ifdef BUILD_HAS_MOD_DEBUG
    if (pd_type == MOD_PD_TYPE_DEVICE_DEBUG) {
        event.id = mod_scmi_pd_event_id_dbg_enable_set;
    }
#endif

    struct event_request_params *event_params;

    event_params = (struct event_request_params *)event.params;
    event_params->pd_id = pd_id;
    event_params->pd_power_state = power_state;
    return fwk_put_event(&event);
}

static int scmi_pd_power_state_get_handler(fwk_id_t service_id,
                                           const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    int respond_status;
    const struct scmi_pd_power_state_get_a2p *parameters;
    unsigned int agent_idx;
    fwk_id_t pd_id;
    struct scmi_pd_power_state_get_p2a return_values = {
        .status = (int32_t)SCMI_GENERIC_ERROR
    };
    enum mod_pd_type pd_type;
    unsigned int pd_power_state;
    unsigned int power_state;
#ifdef BUILD_HAS_MOD_DEBUG
    struct fwk_event event;
    struct event_request_params *event_params;
#endif

    parameters = (const struct scmi_pd_power_state_get_a2p *)payload;

    if (parameters->domain_id > UINT16_MAX) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        goto exit;
    }

    if (scmi_pd_agent_check(service_id, &agent_idx) != SCMI_SUCCESS) {
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    status = scmi_pd_get_domain_id(agent_idx, parameters->domain_id, &pd_id);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS) {
        goto exit;
    }

    switch (pd_type) {
    case MOD_PD_TYPE_CORE:
    case MOD_PD_TYPE_CLUSTER:
        status = scmi_pd_ctx.pd_api->get_state(pd_id, &power_state);
        break;

    case MOD_PD_TYPE_DEVICE_DEBUG:
#ifdef BUILD_HAS_MOD_DEBUG
        event = (struct fwk_event){
            .target_id = fwk_module_id_scmi_power_domain,
            .id = mod_scmi_pd_event_id_dbg_enable_get,
        };

        event_params = (struct event_request_params *)event.params;

        event_params->pd_id = pd_id;

        status = fwk_put_event(&event);
        if (status != FWK_SUCCESS) {
            break;
        }

        ops_set_busy(pd_id, service_id);

        return FWK_SUCCESS;
#endif
    case MOD_PD_TYPE_DEVICE:

        status = scmi_pd_ctx.pd_api->get_state(pd_id, &pd_power_state);
        if (status != FWK_SUCCESS) {
            goto exit;
        }

        status = pd_state_to_scmi_device_state(pd_power_state,
                                               (uint32_t *) &power_state);
        break;

    case MOD_PD_TYPE_SYSTEM:
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        /* Fallthrough. */

    default:
        goto exit;
    }

    if (status == FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_SUCCESS;
        return_values.power_state = power_state;
    }

exit:
    respond_status = scmi_pd_ctx.scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));

    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
    }

    return status;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_pd_power_state_notify_handler(
    enum scmi_pd_command_id command_id,
    fwk_id_t service_id,
    const uint32_t *payload)
{
    unsigned int agent_id;
    enum mod_pd_type pd_type;
    int status;
    fwk_id_t pd_id;
    const struct scmi_pd_power_state_notify_a2p *parameters;
    struct scmi_pd_power_state_notify_p2a return_values = {
        .status = (int32_t)SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_pd_power_state_notify_a2p *)payload;

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_get_domain_id(agent_id, parameters->domain_id, &pd_id);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        goto exit;
    }

    if ((parameters->notify_enable & ~SCMI_PD_NOTIFY_ENABLE_MASK) != 0x0) {
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    /* We are supporting SCMI notifications only for DEVICE type
     * power domains. Core and cluster changes power states very frequently
     * hence notifying power states of core/cluster is cpu intensive
     * and not much useful to any agent as notified state can become
     * invalid soon after notification. See SCMI spec v2 4.3.3.1
     * "that notified power states might not match those requested by the
     * agent that is notified"
     */
    if (pd_type != MOD_PD_TYPE_DEVICE && pd_type != MOD_PD_TYPE_DEVICE_DEBUG) {
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;
        goto exit;
    }

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        goto exit;
    }

    if (parameters->notify_enable) {
        status =
            scmi_pd_ctx.scmi_notification_api->scmi_notification_add_subscriber(
                MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
                parameters->domain_id,
                command_id,
                service_id);
    } else {
        status = scmi_pd_ctx.scmi_notification_api
                     ->scmi_notification_remove_subscriber(
                         MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
                         agent_id,
                         parameters->domain_id,
                         command_id);
    }
    if (status != FWK_SUCCESS) {
        return_values.status = (int32_t)SCMI_GENERIC_ERROR;
        goto exit;
    }

    return_values.status = (int32_t)SCMI_SUCCESS;

exit:
    return scmi_pd_ctx.scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
}

static int scmi_pd_power_state_changed_notify_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    return scmi_pd_power_state_notify_handler(
        MOD_SCMI_PD_POWER_STATE_NOTIFY, service_id, payload);
}

static int scmi_pd_power_state_change_requested_notify_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    return scmi_pd_power_state_notify_handler(
        MOD_SCMI_PD_POWER_STATE_CHANGE_REQUESTED_NOTIFY, service_id, payload);
}
#endif

/*
 * SCMI Power Domain Policy Handlers
 */
FWK_WEAK int scmi_pd_power_state_set_policy(
    enum mod_scmi_pd_policy_status *policy_status,
    uint32_t *state,
    unsigned int agent_id,
    fwk_id_t pd_id)
{
    int status;
    enum mod_pd_type pd_type;

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS) {
        return status;
    }

    *policy_status = MOD_SCMI_PD_EXECUTE_MESSAGE_HANDLER;

    if ((pd_type == MOD_PD_TYPE_DEVICE) ||
        (pd_type == MOD_PD_TYPE_DEVICE_DEBUG)) {
        uint32_t dev_state_idx;
        uint32_t dev_state_table_size = sizeof(pd_state_to_scmi_dev_state) /
            sizeof(pd_state_to_scmi_dev_state[0]);
        uint32_t dev_state = *state;
        /*
         * Convert the device SCMI power state sent by the agent,
         * into the internal power domain state as defined by the
         * pd_state_to_scmi_dev_state table.
         */
        for (dev_state_idx = 0; dev_state_idx < dev_state_table_size;
             dev_state_idx++) {
            if (pd_state_to_scmi_dev_state[dev_state_idx] == dev_state) {
                *state = dev_state_idx;
                break;
            }
        }
    }

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS

/*
 * SCMI Resource Permissions handler
 */

static inline unsigned int get_scmi_pd_index(
    unsigned int agent_id,
    unsigned int param_id)
{
#    ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    return scmi_pd_ctx.config->agent_config_table[agent_id].domains[param_id];
#    else
    return param_id;
#    endif
}

static unsigned int get_pd_domain_id(
    const uint32_t *payload,
    unsigned int agent_id,
    unsigned int message_id)
{
    const struct scmi_pd_power_state_set_a2p *params_set;
    const struct scmi_pd_power_state_get_a2p *params_get;

    enum scmi_pd_command_id message_id_type =
        (enum scmi_pd_command_id)message_id;

    switch (message_id_type) {
    case MOD_SCMI_PD_POWER_STATE_SET:
        params_set = (const struct scmi_pd_power_state_set_a2p *)payload;
        return get_scmi_pd_index(agent_id, params_set->domain_id);

    default:
        /*
         * Every SCMI Power Domains message apart from power_state_set
         * is formatted with the domain ID as the first message element.
         * We will use the power_state_set message as a basic format to
         * retrieve the domain ID to avoid unnecessary code.
         */
        params_get = (const struct scmi_pd_power_state_get_a2p *)payload;
        return get_scmi_pd_index(agent_id, params_get->domain_id);
    }
}

static int scmi_pd_permissions_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    enum mod_res_perms_permissions perms;
    unsigned int agent_id, domain_id;
    enum mod_pd_type pd_type;
    fwk_id_t pd_id;
    int status;

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        return (int32_t)SCMI_GENERIC_ERROR;
    }

    domain_id = get_pd_domain_id(payload, agent_id, message_id);
    if (domain_id > UINT16_MAX) {
        return (int32_t)SCMI_NOT_FOUND;
    }

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_id);
    if (!fwk_module_is_valid_element_id(pd_id)) {
        return (int32_t)SCMI_NOT_FOUND;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS) {
        return (int32_t)SCMI_GENERIC_ERROR;
    }

    perms = scmi_pd_ctx.res_perms_api->agent_has_resource_permission(
        agent_id, MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN, message_id, domain_id);

    if (perms == MOD_RES_PERMS_ACCESS_ALLOWED) {
        return (int32_t)SCMI_SUCCESS;
    }

    return (int32_t)SCMI_DENIED;
}

#endif

/*
 * SCMI module -> SCMI power module interface
 */
static int scmi_pd_get_scmi_protocol_id(fwk_id_t protocol_id,
                                        uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN;

    return FWK_SUCCESS;
}

static int scmi_pd_message_handler(fwk_id_t protocol_id, fwk_id_t service_id,
    const uint32_t *payload, size_t payload_size, unsigned int message_id)
{
    int validation_result;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Power domain management protocol table sizes not consistent");

    validation_result = scmi_pd_ctx.scmi_api->scmi_message_validation(
        MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
        service_id,
        payload,
        payload_size,
        message_id,
        payload_size_table,
        (unsigned int)MOD_SCMI_PD_POWER_COMMAND_COUNT,
        handler_table);

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    if ((message_id >= MOD_SCMI_MESSAGE_ID_ATTRIBUTE) &&
        (validation_result == SCMI_SUCCESS)) {
        validation_result = scmi_pd_permissions_handler(
            service_id, payload, payload_size, message_id);
    }
#endif

    if (validation_result == SCMI_SUCCESS) {
        return handler_table[message_id](service_id, payload);
    }

    return scmi_pd_ctx.scmi_api->respond(
        service_id, &validation_result, sizeof(validation_result));
}

static struct mod_scmi_to_protocol_api scmi_pd_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_pd_get_scmi_protocol_id,
    .message_handler = scmi_pd_message_handler
};

/*
 * Framework handlers
 */

static int scmi_pd_init(fwk_id_t module_id, unsigned int element_count,
                        const void *data)
{
#if defined(BUILD_HAS_MOD_DEBUG) || defined(BUILD_HAS_AGENT_LOGICAL_DOMAIN)
    struct mod_scmi_pd_config *config = (struct mod_scmi_pd_config *)data;
#endif

    if (element_count != 0) {
        return FWK_E_SUPPORT;
    }

    scmi_pd_ctx.domain_count =
        (unsigned int)fwk_module_get_element_count(fwk_module_id_power_domain);
    if (scmi_pd_ctx.domain_count <= 1) {
        return FWK_E_SUPPORT;
    }

    /* Do not expose SYSTEM domain (always the last one) to agents and ... */
    scmi_pd_ctx.domain_count--;
    /* ... and expose no more than 0xFFFF number of domains. */
    if (scmi_pd_ctx.domain_count > UINT16_MAX) {
        scmi_pd_ctx.domain_count = UINT16_MAX;
    }

#ifdef BUILD_HAS_MOD_DEBUG
    if (config == NULL) {
        return FWK_E_PARAM;
    }

    if (fwk_module_is_valid_element_id(config->debug_id) ||
        fwk_module_is_valid_element_id(config->debug_pd_id)) {
        scmi_pd_ctx.debug_pd_id = config->debug_pd_id;
        scmi_pd_ctx.debug_id = config->debug_id;
    } else {
        return FWK_E_DATA;
    }
#endif

    /* Allocate a table of scmi_pd operations */
    scmi_pd_ctx.ops =
        fwk_mm_calloc(scmi_pd_ctx.domain_count,
        sizeof(struct scmi_pd_operations));
    if (scmi_pd_ctx.ops == NULL) {
        return FWK_E_NOMEM;
    }

    /* Initialize table */
    for (unsigned int i = 0; i < scmi_pd_ctx.domain_count; i++) {
        scmi_pd_ctx.ops[i].service_id = FWK_ID_NONE;
    }

#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    scmi_pd_ctx.config = config;
#endif

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_init_notifications(unsigned int domains)
{
    int status;

    status = scmi_pd_ctx.scmi_api->get_agent_count(&scmi_pd_ctx.agent_count);
    if (status != FWK_SUCCESS) {
        return status;
    }
    fwk_assert(scmi_pd_ctx.agent_count != 0u);

    status = scmi_pd_ctx.scmi_notification_api->scmi_notification_init(
        MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
        scmi_pd_ctx.agent_count,
        domains,
        MOD_SCMI_PD_NOTIFICATION_COUNT);

    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}
#endif

static int scmi_pd_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1) {
        return FWK_SUCCESS;
    }

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_pd_ctx.scmi_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_NOTIFICATION),
        &scmi_pd_ctx.scmi_notification_api);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

#ifdef BUILD_HAS_MOD_DEBUG
    status = fwk_module_bind(scmi_pd_ctx.debug_id,
        FWK_ID_API(FWK_MODULE_IDX_DEBUG, MOD_DEBUG_API_IDX_HAL),
        &scmi_pd_ctx.debug_api);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &scmi_pd_ctx.res_perms_api);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

    return fwk_module_bind(fwk_module_id_power_domain, mod_pd_api_id_restricted,
        &scmi_pd_ctx.pd_api);
}

static int scmi_pd_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                        fwk_id_t api_id, const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI))) {
        return FWK_E_ACCESS;
    }

    *api = &scmi_pd_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

static int process_request_event(const struct fwk_event *event)
{
    fwk_id_t pd_id;
    int status;
    int respond_status;
#ifdef BUILD_HAS_MOD_DEBUG
    bool dbg_enabled;
    bool state_get;
    struct scmi_pd_power_state_get_p2a retval_get = {
        .status = SCMI_GENERIC_ERROR
    };
#endif
    fwk_id_t service_id;
    struct scmi_pd_power_state_set_p2a retval_set = {
        .status = (int32_t)SCMI_GENERIC_ERROR
    };
    struct event_request_params *params =
        (struct event_request_params *)event->params;

    pd_id = params->pd_id;

    switch ((enum scmi_pd_event_idx)fwk_id_get_event_idx(event->id)) {
#ifdef BUILD_HAS_MOD_DEBUG
    case SCMI_PD_EVENT_IDX_DEBUG_GET:
        state_get = true;

        status = scmi_pd_ctx.debug_api->get_enabled(scmi_pd_ctx.debug_id,
                                                    &dbg_enabled,
                                                    SCP_DEBUG_USER_AP);
        if (status != FWK_SUCCESS) {
            break;
        }

        retval_get.status = SCMI_SUCCESS;
        retval_get.power_state =
            dbg_enabled ? pd_state_to_scmi_dev_state[MOD_PD_STATE_ON]
                        : pd_state_to_scmi_dev_state[MOD_PD_STATE_OFF];

        break;

    case SCMI_PD_EVENT_IDX_DEBUG_SET:
        state_get = false;
        status = scmi_pd_ctx.debug_api->set_enabled(
                scmi_pd_ctx.debug_id,
                params->pd_power_state == MOD_PD_STATE_ON,
                SCP_DEBUG_USER_AP);

        if (status == FWK_SUCCESS) {
            retval_set.status = SCMI_SUCCESS;
        }

        break;
#endif
    case SCMI_PD_EVENT_IDX_SET_STATE:
        status =
            scmi_pd_ctx.pd_api->set_state(pd_id, true, params->pd_power_state);

        if (status != FWK_SUCCESS) {
            /* There was an error, so send the SCMI response to note this.*/
            retval_set.status = (int32_t)SCMI_GENERIC_ERROR;
            service_id = ops_get_service(pd_id);
            respond_status = scmi_pd_ctx.scmi_api->respond(
                service_id, &retval_set, sizeof(retval_set.status));

            if (respond_status != FWK_SUCCESS) {
                FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
                return respond_status;
            }
        }

        return status;

    default:
        return FWK_E_PARAM;
    }

#ifdef BUILD_HAS_MOD_DEBUG
    if (status != FWK_PENDING) {
        service_id = ops_get_service(pd_id);

        respond_status = scmi_pd_ctx.scmi_api->respond(
            service_id,
            state_get ? (void *)&retval_get : (void *)&retval_set,
            state_get ? sizeof(retval_get) : sizeof(retval_set));

        if (respond_status != FWK_SUCCESS) {
            FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
        }

        ops_set_idle(pd_id);
    }

    return status;
#endif
}

static int process_response_event(const struct fwk_event *event)
{
    int respond_status;
    fwk_id_t service_id;
    unsigned int module_idx;
    struct scmi_pd_power_state_set_p2a retval_set = {
        .status = (int32_t)SCMI_GENERIC_ERROR
    };
#ifdef BUILD_HAS_MOD_DEBUG
    struct scmi_pd_power_state_get_p2a retval_get = { .status =
                                                          SCMI_GENERIC_ERROR };
#endif

    module_idx = fwk_id_get_module_idx(event->source_id);

    if (module_idx == fwk_id_get_module_idx(fwk_module_id_power_domain)) {
        struct pd_set_state_response *params =
            (struct pd_set_state_response *)event->params;

        service_id = ops_get_service(event->source_id);

        if (params->status == FWK_SUCCESS) {
            retval_set.status = (int32_t)SCMI_SUCCESS;
        }

        respond_status = scmi_pd_ctx.scmi_api->respond(
            service_id, &retval_set, sizeof(retval_set));

        if (respond_status != FWK_SUCCESS) {
            FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
        }

        ops_set_idle(event->source_id);
        return FWK_SUCCESS;

#ifdef BUILD_HAS_MOD_DEBUG
    } else if (module_idx == fwk_id_get_module_idx(fwk_module_id_debug)) {
        /* Responses from Debug module */
        struct mod_debug_response_params *params =
            (struct mod_debug_response_params *)event->params;

        if (params->status == FWK_SUCCESS) {
            retval_set.status = SCMI_SUCCESS;
        }

        /*
         * We know this event comes from the DEBUG HAL for now so we use the
         * corresponding pd_id.
         */
        service_id = ops_get_service(scmi_pd_ctx.debug_pd_id);

        if (fwk_id_get_event_idx(event->id) ==
            MOD_DEBUG_PUBLIC_EVENT_IDX_REQ_ENABLE_GET) {
            retval_get.power_state =
                params->enabled ? pd_state_to_scmi_dev_state[MOD_PD_STATE_ON]
                                : pd_state_to_scmi_dev_state[MOD_PD_STATE_OFF];
            respond_status = scmi_pd_ctx.scmi_api->respond(
                service_id, (void *)&retval_get, sizeof(retval_get));
            if (respond_status != FWK_SUCCESS) {
                FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
            }
        } else {
            respond_status = scmi_pd_ctx.scmi_api->respond(
                service_id, (void *)&retval_set, sizeof(retval_set));
            if (respond_status != FWK_SUCCESS) {
                FWK_LOG_DEBUG("[SCMI-power] %s @%d", __func__, __LINE__);
            }
        }
        ops_set_idle(scmi_pd_ctx.debug_pd_id);
        return FWK_SUCCESS;
#endif

    } else {
        return FWK_E_PARAM;
    }
}

static int scmi_pd_process_event(const struct fwk_event *event,
                                 struct fwk_event *resp_event)
{
    if (event->is_response) {
        return process_response_event(event);
    } else {
        return process_request_event(event);
    }
}

#ifdef BUILD_HAS_MOD_DEBUG
static int scmi_pd_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    unsigned int domain_id;
    unsigned int agent_id;
    uint32_t state;
    struct mod_pd_power_state_transition_notification_params *event_params =
        ((struct mod_pd_power_state_transition_notification_params *)
            event->params);

    if (fwk_id_is_equal(
            event->id, mod_pd_notification_id_power_state_transition)) {
        domain_id = fwk_id_get_element_idx(event->source_id);
        state = event_params->state;
        agent_id = ops_get_agent_id(event->source_id);
        scmi_pd_power_state_notify(
            MOD_SCMI_PD_POWER_STATE_NOTIFY,
            SCMI_POWER_STATE_CHANGED,
            domain_id,
            agent_id,
            state);
    }

    return FWK_SUCCESS;
}
#endif

static int scmi_pd_start(fwk_id_t id)
{
    int status = FWK_SUCCESS;
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    status = scmi_init_notifications(scmi_pd_ctx.domain_count);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif
    return status;
}

/* SCMI Power Domain Management Protocol Definition */
const struct fwk_module module_scmi_power_domain = {
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_pd_init,
    .bind = scmi_pd_bind,
    .start = scmi_pd_start,
    .process_bind_request = scmi_pd_process_bind_request,
    .event_count = (unsigned int)SCMI_PD_EVENT_IDX_COUNT,
    .process_event = scmi_pd_process_event,
#ifdef BUILD_HAS_MOD_DEBUG
    .process_notification = scmi_pd_process_notification,
#endif
};
