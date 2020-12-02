/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI power domain management protocol support.
 */

#include <internal/scmi_power_domain.h>

#include <mod_power_domain.h>
#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
#    include <mod_resource_perms.h>
#endif
#include <mod_scmi.h>
#include <mod_scmi_power_domain.h>

#ifdef BUILD_HAS_MOD_DEBUG
#    include <mod_debug.h>
#endif

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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

struct scmi_pd_ctx {
    /* Number of power domains */
    unsigned int domain_count;

    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* Power domain module API */
    const struct mod_pd_restricted_api *pd_api;
#if BUILD_HAS_MOD_DEBUG
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

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    /* SCMI Resource Permissions API */
    const struct mod_res_permissions_api *res_perms_api;
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /* Number of active agents */
    int agent_count;

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

#ifdef BUILD_HAS_MOD_DEBUG
enum scmi_clock_event_idx {
    /* Event used prior to send a set_enabled request to debug HAL. */
    SCMI_PD_EVENT_IDX_DEBUG_SET,

    /* Event used prior to send a get_enabled request to debug HAL. */
    SCMI_PD_EVENT_IDX_DEBUG_GET,
    SCMI_PD_EVENT_IDX_COUNT,
};

struct event_request_params {
    unsigned int pd_power_state;
    fwk_id_t pd_id;
};

static const fwk_id_t mod_scmi_pd_event_id_dbg_enable_set =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
                      SCMI_PD_EVENT_IDX_DEBUG_SET);

static const fwk_id_t mod_scmi_pd_event_id_dbg_enable_get =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
                      SCMI_PD_EVENT_IDX_DEBUG_GET);
#endif
/*
 * Internal variables
 */

static struct scmi_pd_ctx scmi_pd_ctx;

static int (*handler_table[])(fwk_id_t, const uint32_t *) = {
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

static unsigned int payload_size_table[] = {
    [MOD_SCMI_PROTOCOL_VERSION] = 0,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        sizeof(struct scmi_protocol_message_attributes_a2p),
    [MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES] =
        sizeof(struct scmi_pd_power_domain_attributes_a2p),
    [MOD_SCMI_PD_POWER_STATE_SET] = sizeof(struct scmi_pd_power_state_set_a2p),
    [MOD_SCMI_PD_POWER_STATE_GET] = sizeof(struct scmi_pd_power_state_get_a2p),

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_PD_POWER_STATE_NOTIFY] =
        sizeof(struct scmi_pd_power_state_notify_a2p),
    [MOD_SCMI_PD_POWER_STATE_CHANGE_REQUESTED_NOTIFY] =
        sizeof(struct scmi_pd_power_state_notify_a2p),
#endif
};

static uint32_t pd_state_to_scmi_dev_state[] = {
    [MOD_PD_STATE_OFF] = SCMI_PD_DEVICE_STATE_ID_OFF |
                         SCMI_PD_DEVICE_STATE_TYPE,
    [MOD_PD_STATE_ON] = SCMI_PD_DEVICE_STATE_ID_ON,
    /* In case of more supported device states review the map functions */
};

/*
 * Helpers
 */

#ifdef BUILD_HAS_MOD_DEBUG
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

static unsigned int ops_get_agent_id(fwk_id_t pd_id)
{
    unsigned int pd_idx = fwk_id_get_element_idx(pd_id);

    return scmi_pd_ctx.ops[pd_idx].agent_id;
}

static void ops_set_idle(fwk_id_t pd_id)
{
    unsigned int pd_idx = fwk_id_get_element_idx(pd_id);

    scmi_pd_ctx.ops[pd_idx].service_id = FWK_ID_NONE;
}
#endif

/*
 * Power domain management protocol implementation
 */

static int scmi_device_state_to_pd_state(uint32_t scmi_state,
                                         unsigned int *pd_state)
{
    uint32_t scmi_state_id;
    bool ctx_lost;

    ctx_lost = !!(scmi_state & SCMI_PD_DEVICE_STATE_TYPE);
    scmi_state_id = scmi_state & SCMI_PD_DEVICE_STATE_ID_MASK;

    if (scmi_state_id == SCMI_PD_DEVICE_STATE_ID) {
        if (ctx_lost)
            *pd_state = MOD_PD_STATE_OFF;
        else
            *pd_state = MOD_PD_STATE_ON;
    } else {
        /* Implementation Defined state */
        *pd_state = scmi_state;
    }

    return FWK_SUCCESS;
}

static int pd_state_to_scmi_device_state(unsigned int pd_state,
                                         uint32_t *scmi_state)
{
    if (pd_state == MOD_PD_STATE_OFF || pd_state == MOD_PD_STATE_ON)
        *scmi_state = pd_state_to_scmi_dev_state[pd_state];
    else
        *scmi_state = pd_state;

    return FWK_SUCCESS;
}

static int scmi_pd_protocol_version_handler(fwk_id_t service_id,
                                            const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_POWER_DOMAIN,
    };

    scmi_pd_ctx.scmi_api->respond(service_id,
                                  &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_pd_protocol_attributes_handler(fwk_id_t service_id,
                                               const uint32_t *payload)
{
    struct scmi_pd_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
    };

    return_values.attributes = scmi_pd_ctx.domain_count;

    scmi_pd_ctx.scmi_api->respond(service_id,
                                  &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_pd_power_domain_attributes_handler(fwk_id_t service_id,
                                                   const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    const struct scmi_pd_power_domain_attributes_a2p *parameters;
    enum mod_pd_type pd_type;
    unsigned int domain_idx;
    fwk_id_t pd_id;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;
    struct scmi_pd_power_domain_attributes_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_pd_power_domain_attributes_a2p *)payload;

    domain_idx = parameters->domain_id;
    if (domain_idx > UINT16_MAX) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_idx);
    if (!fwk_module_is_valid_element_id(pd_id)) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_pd_ctx.scmi_api->get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS)
        goto exit;

    switch (pd_type) {
    case MOD_PD_TYPE_CORE:
        /*
         * For core power domains, the POWER_STATE_SET command is supported
         * only asynchronously for the PSCI agent. In all other cases, reply
         * that the command is not supported either synchronously nor
         * asynchronously.
         */
        if (agent_type == SCMI_AGENT_TYPE_PSCI)
            return_values.attributes = SCMI_PD_POWER_STATE_SET_ASYNC;
        break;

    case MOD_PD_TYPE_CLUSTER:
        /*
         * For cluster power domains, the POWER_STATE_SET command is supported
         * only synchronously for the PSCI agent. In all other cases, reply
         * that the command is not supported either synchronously nor
         * asynchronously.
         */
        if (agent_type == SCMI_AGENT_TYPE_PSCI)
            return_values.attributes = SCMI_PD_POWER_STATE_SET_SYNC;
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
        return_values.status = SCMI_NOT_FOUND;
        /* Fallthrough. */

    default:
        goto exit;
    }

    strncpy((char *)return_values.name, fwk_module_get_name(pd_id),
            sizeof(return_values.name) - 1);

    return_values.status = SCMI_SUCCESS;

exit:
    scmi_pd_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_pd_protocol_message_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *parameters;
    struct scmi_protocol_message_attributes_p2a return_values = {
        .status = SCMI_NOT_FOUND,
    };

    parameters = (const struct scmi_protocol_message_attributes_a2p *)
                  payload;

    if ((parameters->message_id < FWK_ARRAY_SIZE(handler_table)) &&
        (handler_table[parameters->message_id] != NULL))
        return_values.status = SCMI_SUCCESS;

    scmi_pd_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return FWK_SUCCESS;
}

static int scmi_power_scp_set_core_state(fwk_id_t pd_id,
                                         uint32_t composite_state)
{
    int status;

    status = scmi_pd_ctx.pd_api->set_state_async(pd_id, false, composite_state);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            "[SCMI:power] Failed to send core set request (error %s (%d))",
            fwk_status_str(status),
            status);
    }

    return status;
}

static void scmi_pd_power_state_notify(
    enum scmi_pd_command_id command_id,
    enum scmi_pd_notification_id notification_message_id,
    unsigned int domain_id,
    unsigned int agent_id,
    uint32_t power_state)
{
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    struct scmi_pd_power_state_notification_message_p2a message;

    message.agent_id = agent_id;
    message.domain_id = domain_id;
    message.power_state = power_state;

    scmi_pd_ctx.scmi_notification_api->scmi_notification_notify(
        MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
        command_id,
        notification_message_id,
        &message,
        sizeof(message));
#endif
}

static int scmi_pd_power_state_set_handler(fwk_id_t service_id,
                                           const uint32_t *payload)
{
    int status;
    const struct scmi_pd_power_state_set_a2p *parameters;
    bool is_sync;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;
    unsigned int domain_idx;
    fwk_id_t pd_id;
    unsigned int pd_power_state;
    struct scmi_pd_power_state_set_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };
    enum mod_pd_type pd_type;
    uint32_t power_state;
    enum mod_scmi_pd_policy_status policy_status;

    parameters = (const struct scmi_pd_power_state_set_a2p *)payload;

    is_sync = !(parameters->flags & SCMI_PD_POWER_STATE_SET_ASYNC_FLAG_MASK);

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_pd_ctx.scmi_api->get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    domain_idx = parameters->domain_id;
    if (domain_idx > UINT16_MAX) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    if (((parameters->flags & ~SCMI_PD_POWER_STATE_SET_FLAGS_MASK) != 0x0) ||
        ((parameters->power_state &
          ~SCMI_PD_POWER_STATE_SET_POWER_STATE_MASK) != 0x0)) {
        status = FWK_SUCCESS;
        return_values.status = SCMI_INVALID_PARAMETERS;

        goto exit;
    }

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_idx);
    if (!fwk_module_is_valid_element_id(pd_id)) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS)
        goto exit;

    if (((pd_type == MOD_PD_TYPE_CORE) ||
         (pd_type == MOD_PD_TYPE_CLUSTER)) &&
        (agent_type != SCMI_AGENT_TYPE_PSCI)) {

         return_values.status = SCMI_NOT_SUPPORTED;
         goto exit;
    }

    power_state = parameters->power_state;
    status = scmi_pd_power_state_set_policy(&policy_status, &power_state,
        agent_id, pd_id);

    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_GENERIC_ERROR;
        goto exit;
    }
    if (policy_status == MOD_SCMI_PD_SKIP_MESSAGE_HANDLER) {
        return_values.status = SCMI_SUCCESS;
        goto exit;
    }

    switch (pd_type) {
    case MOD_PD_TYPE_CORE:
        /*
         * Async/sync flag is ignored for core power domains as stated
         * by the specification.
         */
        status = scmi_power_scp_set_core_state(pd_id, power_state);
        if (status == FWK_E_PARAM)
            return_values.status = SCMI_INVALID_PARAMETERS;
        break;

    case MOD_PD_TYPE_CLUSTER:
        if (!is_sync) {
            return_values.status = SCMI_NOT_SUPPORTED;
            goto exit;
        }

        status = scmi_pd_ctx.pd_api->set_state(pd_id, power_state);
        break;


    case MOD_PD_TYPE_DEVICE_DEBUG:
#ifdef BUILD_HAS_MOD_DEBUG
        if (!is_sync) {
            return_values.status = SCMI_NOT_SUPPORTED;
            goto exit;
        }

        status = scmi_device_state_to_pd_state(power_state,
                                               &pd_power_state);
        if (status != FWK_SUCCESS) {
            status = FWK_SUCCESS;
            return_values.status = SCMI_INVALID_PARAMETERS;
            goto exit;
        }

        if (ops_is_busy(pd_id)) {
            status = FWK_E_BUSY;

            break;
        }

        struct fwk_event event = {
            .target_id = fwk_module_id_scmi_power_domain,
            .id = mod_scmi_pd_event_id_dbg_enable_set,
        };

        struct event_request_params *event_params =
            (struct event_request_params *)(void *)event.params;
        event_params->pd_power_state = pd_power_state;
        event_params->pd_id = pd_id;

        scmi_pd_power_state_notify(
            MOD_SCMI_PD_POWER_STATE_CHANGE_REQUESTED_NOTIFY,
            SCMI_POWER_STATE_CHANGE_REQUESTED,
            domain_idx,
            agent_id,
            power_state);

        ops_set_agent_id(pd_id, agent_id);

        status = fwk_thread_put_event(&event);
        if (status != FWK_SUCCESS)
            break;

        if (scmi_pd_ctx.debug_pd_state_notification_enabled == false) {
            status = fwk_notification_subscribe(
                mod_pd_notification_id_power_state_transition,
                pd_id,
                fwk_module_id_scmi_power_domain);
            if (status != FWK_SUCCESS)
                break;
            else
                scmi_pd_ctx.debug_pd_state_notification_enabled = true;
        }

        ops_set_busy(pd_id, service_id);

        return FWK_SUCCESS;
#endif /* BUILD_HAS_MOD_DEBUG */
    case MOD_PD_TYPE_DEVICE:
        if (!is_sync) {
            return_values.status = SCMI_NOT_SUPPORTED;
            goto exit;
        }

        status = scmi_device_state_to_pd_state(power_state,
                                               &pd_power_state);
        if (status != FWK_SUCCESS) {
            status = FWK_SUCCESS;
            return_values.status = SCMI_INVALID_PARAMETERS;
            goto exit;
        }

        /* We are supporting SCMI notifications only for DEVICE type
         * power domains. Core and cluster changes power states very frequently
         * hence notifying power states of core/cluster is cpu intensive
         * and not very much useful to any agent as notified state can become
         * invalid soon after notification. See SCMI spec v2 4.3.3.1
         * "that notified power states might not match those requested by the
         * agent that is notified"
         */
        scmi_pd_power_state_notify(
            MOD_SCMI_PD_POWER_STATE_CHANGE_REQUESTED_NOTIFY,
            SCMI_POWER_STATE_CHANGE_REQUESTED,
            domain_idx,
            agent_id,
            power_state);

        status = scmi_pd_ctx.pd_api->set_state(pd_id, pd_power_state);

        if (status == FWK_SUCCESS) {
            scmi_pd_power_state_notify(
                MOD_SCMI_PD_POWER_STATE_NOTIFY,
                SCMI_POWER_STATE_CHANGED,
                domain_idx,
                agent_id,
                power_state);
        }

        break;

    case MOD_PD_TYPE_SYSTEM:
        return_values.status = SCMI_NOT_FOUND;
        /* Fallthrough. */

    default:
        goto exit;
    }

    if (status == FWK_SUCCESS)
        return_values.status = SCMI_SUCCESS;

exit:
    scmi_pd_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_pd_power_state_get_handler(fwk_id_t service_id,
                                           const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    const struct scmi_pd_power_state_get_a2p *parameters;
    unsigned int domain_idx;
    fwk_id_t pd_id;
    struct scmi_pd_power_state_get_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };
    enum mod_pd_type pd_type;
    unsigned int pd_power_state;
    unsigned int power_state;
#ifdef BUILD_HAS_MOD_DEBUG
    struct fwk_event event;
    struct event_request_params *event_params;
#endif

    parameters = (const struct scmi_pd_power_state_get_a2p *)payload;

    domain_idx = parameters->domain_id;
    if (domain_idx > UINT16_MAX) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_idx);
    if (!fwk_module_is_valid_element_id(pd_id)) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS)
        goto exit;

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

        event_params = (struct event_request_params *)(void *)event.params;

        event_params->pd_id = pd_id;

        status = fwk_thread_put_event(&event);
        if (status != FWK_SUCCESS)
            break;

        ops_set_busy(pd_id, service_id);

        return FWK_SUCCESS;
#endif
    case MOD_PD_TYPE_DEVICE:

        status = scmi_pd_ctx.pd_api->get_state(pd_id, &pd_power_state);
        if (status != FWK_SUCCESS)
            goto exit;

        status = pd_state_to_scmi_device_state(pd_power_state,
                                               (uint32_t *) &power_state);
        break;

    case MOD_PD_TYPE_SYSTEM:
        return_values.status = SCMI_NOT_FOUND;
        /* Fallthrough. */

    default:
        goto exit;
    }

    if (status == FWK_SUCCESS) {
        return_values.status = SCMI_SUCCESS;
        return_values.power_state = power_state;
    }

exit:
    scmi_pd_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

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
    unsigned int domain_idx;
    fwk_id_t pd_id;
    const struct scmi_pd_power_state_notify_a2p *parameters;
    struct scmi_pd_power_state_notify_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_pd_power_state_notify_a2p *)payload;
    domain_idx = parameters->domain_id;
    if (domain_idx >= scmi_pd_ctx.domain_count) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_idx);
    if (!fwk_module_is_valid_element_id(pd_id)) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_NOT_FOUND;
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
        return_values.status = SCMI_NOT_SUPPORTED;
        goto exit;
    }

    if ((parameters->notify_enable & ~SCMI_PD_NOTIFY_ENABLE_MASK) != 0x0) {
        return_values.status = SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    if (parameters->notify_enable) {
        scmi_pd_ctx.scmi_notification_api->scmi_notification_add_subscriber(
            MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
            domain_idx,
            command_id,
            service_id);
    } else {
        scmi_pd_ctx.scmi_notification_api->scmi_notification_remove_subscriber(
            MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
            agent_id,
            domain_idx,
            command_id);
    }

    return_values.status = SCMI_SUCCESS;

exit:
    scmi_pd_ctx.scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));

    return FWK_SUCCESS;
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
    *policy_status = MOD_SCMI_PD_EXECUTE_MESSAGE_HANDLER;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS

/*
 * SCMI Resource Permissions handler
 */
static unsigned int get_pd_domain_id(
    const uint32_t *payload,
    unsigned int message_id)
{
    const struct scmi_pd_power_state_set_a2p *params_set;
    const struct scmi_pd_power_state_get_a2p *params_get;

    switch (message_id) {
    case MOD_SCMI_PD_POWER_STATE_SET:
        params_set = (const struct scmi_pd_power_state_set_a2p *)payload;
        return params_set->domain_id;

    default:
        /*
         * Every SCMI Power Domains message apart from power_state_set
         * is formatted with the domain ID as the first message element.
         * We will use the power_state_set message as a basic format to
         * retrieve the domain ID to avoid unnecessary code.
         */
        params_get = (const struct scmi_pd_power_state_get_a2p *)payload;
        return params_get->domain_id;
    }
}

static int scmi_pd_permissions_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id,
    int32_t *return_values)
{
    enum mod_res_perms_permissions perms;
    unsigned int agent_id, domain_id;
    enum mod_pd_type pd_type;
    fwk_id_t pd_id;
    int status;

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        *return_values = SCMI_GENERIC_ERROR;
        return FWK_E_PARAM;
    }

    if (message_id < 3) {
        perms = scmi_pd_ctx.res_perms_api->agent_has_protocol_permission(
            agent_id, MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN);
        if (perms == MOD_RES_PERMS_ACCESS_ALLOWED)
            return FWK_SUCCESS;
        return FWK_E_ACCESS;
    }

    domain_id = get_pd_domain_id(payload, message_id);
    if (domain_id > UINT16_MAX) {
        *return_values = SCMI_NOT_FOUND;
        return FWK_E_ACCESS;
    }

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_id);
    if (!fwk_module_is_valid_element_id(pd_id)) {
        *return_values = SCMI_NOT_FOUND;
        return FWK_E_ACCESS;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS) {
        *return_values = SCMI_GENERIC_ERROR;
        return FWK_E_ACCESS;
    }

    perms = scmi_pd_ctx.res_perms_api->agent_has_resource_permission(
        agent_id, MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN, message_id, domain_id);

    if (perms == MOD_RES_PERMS_ACCESS_ALLOWED)
        return FWK_SUCCESS;

    *return_values = SCMI_DENIED;
    return FWK_E_ACCESS;
}

#endif

/*
 * SCMI module -> SCMI power module interface
 */
static int scmi_pd_get_scmi_protocol_id(fwk_id_t protocol_id,
                                        uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN;

    return FWK_SUCCESS;
}

static int scmi_pd_message_handler(fwk_id_t protocol_id, fwk_id_t service_id,
    const uint32_t *payload, size_t payload_size, unsigned int message_id)
{
    int32_t return_value;
#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    int status;
#endif

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Power domain management protocol table sizes not consistent");
    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return_value = SCMI_NOT_FOUND;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    status = scmi_pd_permissions_handler(
        service_id, payload, payload_size, message_id, &return_value);
    if (status != FWK_SUCCESS)
        goto error;
#endif

    return handler_table[message_id](service_id, payload);

error:
    scmi_pd_ctx.scmi_api->respond(service_id,
                                  &return_value, sizeof(return_value));
    return FWK_SUCCESS;
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
#ifdef BUILD_HAS_MOD_DEBUG
    struct mod_scmi_pd_config *config = (struct mod_scmi_pd_config *)data;
#endif

    if (element_count != 0)
        return FWK_E_SUPPORT;

    scmi_pd_ctx.domain_count = fwk_module_get_element_count(
        fwk_module_id_power_domain);
    if (scmi_pd_ctx.domain_count <= 1)
        return FWK_E_SUPPORT;

    /* Do not expose SYSTEM domain (always the last one) to agents and ... */
    scmi_pd_ctx.domain_count--;
    /* ... and expose no more than 0xFFFF number of domains. */
    if (scmi_pd_ctx.domain_count > UINT16_MAX)
        scmi_pd_ctx.domain_count = UINT16_MAX;

#ifdef BUILD_HAS_MOD_DEBUG
    if (config == NULL)
        return FWK_E_PARAM;

    if (fwk_module_is_valid_element_id(config->debug_id) ||
        fwk_module_is_valid_element_id(config->debug_pd_id)) {
        scmi_pd_ctx.debug_pd_id = config->debug_pd_id;
        scmi_pd_ctx.debug_id = config->debug_id;
    } else
        return FWK_E_DATA;
#endif

    /* Allocate a table of scmi_pd operations */
    scmi_pd_ctx.ops =
        fwk_mm_calloc(scmi_pd_ctx.domain_count,
        sizeof(struct scmi_pd_operations));
    if (scmi_pd_ctx.ops == NULL)
        return FWK_E_NOMEM;

    /* Initialize table */
    for (unsigned int i = 0; i < scmi_pd_ctx.domain_count; i++)
        scmi_pd_ctx.ops[i].service_id = FWK_ID_NONE;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_init_notifications(unsigned int domains)
{
    int status;

    status = scmi_pd_ctx.scmi_api->get_agent_count(&scmi_pd_ctx.agent_count);
    if (status != FWK_SUCCESS)
        return status;
    fwk_assert(scmi_pd_ctx.agent_count != 0);

    status = scmi_pd_ctx.scmi_notification_api->scmi_notification_init(
        MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
        scmi_pd_ctx.agent_count,
        domains,
        MOD_SCMI_PD_NOTIFICATION_COUNT);

    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}
#endif

static int scmi_pd_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_pd_ctx.scmi_api);
    if (status != FWK_SUCCESS)
        return status;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_NOTIFICATION),
        &scmi_pd_ctx.scmi_notification_api);
    if (status != FWK_SUCCESS)
        return status;
#endif

#ifdef BUILD_HAS_MOD_DEBUG
    status = fwk_module_bind(scmi_pd_ctx.debug_id,
        FWK_ID_API(FWK_MODULE_IDX_DEBUG, MOD_DEBUG_API_IDX_HAL),
        &scmi_pd_ctx.debug_api);
    if (status != FWK_SUCCESS)
        return status;
#endif

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &scmi_pd_ctx.res_perms_api);
    if (status != FWK_SUCCESS)
        return status;
#endif

    return fwk_module_bind(fwk_module_id_power_domain, mod_pd_api_id_restricted,
        &scmi_pd_ctx.pd_api);
}

static int scmi_pd_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                        fwk_id_t api_id, const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    *api = &scmi_pd_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_MOD_DEBUG
static int process_request_event(const struct fwk_event *event)
{
    struct event_request_params *params;
    int status;
    fwk_id_t service_id;
    bool state_get;
    bool dbg_enabled;

    struct scmi_pd_power_state_get_p2a retval_get = {
        .status = SCMI_GENERIC_ERROR
    };

    struct scmi_pd_power_state_set_p2a retval_set = {
        .status = SCMI_GENERIC_ERROR
    };

    params = (struct event_request_params *)(void *)event->params;

    /*
     * Internal events:
     * Only events for handling the debug are allowed for now.
     */
    switch (fwk_id_get_event_idx(event->id)) {
    case SCMI_PD_EVENT_IDX_DEBUG_GET:
        state_get = true;

        status = scmi_pd_ctx.debug_api->get_enabled(scmi_pd_ctx.debug_id,
                                                    &dbg_enabled,
                                                    SCP_DEBUG_USER_AP);
        if (status != FWK_SUCCESS)
            break;

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

        if (status == FWK_SUCCESS)
            retval_set.status = SCMI_SUCCESS;

        break;

    default:
        return FWK_E_PARAM;
    }

    if (status != FWK_PENDING) {
        service_id = ops_get_service(params->pd_id);

        scmi_pd_ctx.scmi_api->respond(
            service_id,
            state_get ? (void *)&retval_get : (void *)&retval_set,
            state_get ? sizeof(retval_get) : sizeof(retval_set));

        ops_set_idle(params->pd_id);
    }

    return status;
}

static int process_response_event(const struct fwk_event *event, bool get)
{
    fwk_id_t service_id;

    struct scmi_pd_power_state_get_p2a retval_get = {
        .status = SCMI_GENERIC_ERROR
    };

    struct scmi_pd_power_state_set_p2a retval_set = {
        .status = SCMI_GENERIC_ERROR
    };

    struct mod_debug_response_params *params =
        (struct mod_debug_response_params *)(void *)event->params;

    /*
     * We know this event comes from the DEBUG HAL for now so we use the
     * corresponding pd_id.
     */
    service_id = ops_get_service(scmi_pd_ctx.debug_pd_id);

    if (params->status == FWK_SUCCESS) {
        retval_set.status = SCMI_SUCCESS;

        if (get) {
            retval_get.power_state =
                params->enabled ? pd_state_to_scmi_dev_state[MOD_PD_STATE_ON]
                                : pd_state_to_scmi_dev_state[MOD_PD_STATE_OFF];
        }
    }

    scmi_pd_ctx.scmi_api->respond(
        service_id,
        get ? (void *)&retval_get : (void *)&retval_set,
        get ? sizeof(retval_get) : sizeof(retval_set));

    ops_set_idle(service_id);

    return FWK_SUCCESS;
}

static int scmi_pd_process_event(const struct fwk_event *event,
                                 struct fwk_event *resp_event)
{
    unsigned int module_idx;
    int status;

    module_idx = fwk_id_get_module_idx(event->source_id);

    if (module_idx == fwk_id_get_module_idx(fwk_module_id_scmi))
        return process_request_event(event);

    if (module_idx == fwk_id_get_module_idx(fwk_module_id_debug)) {
        /* Responses from Debug module */
        switch (fwk_id_get_event_idx(event->id)) {
        case MOD_DEBUG_PUBLIC_EVENT_IDX_REQ_ENABLE_SET:
            status = process_response_event(event, false);
            break;

        case MOD_DEBUG_PUBLIC_EVENT_IDX_REQ_ENABLE_GET:
            status = process_response_event(event, true);
            break;

        default:
            status = FWK_E_PARAM;
            break;
        }

        return status;
    }
    return FWK_E_PARAM;
}

static int scmi_pd_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    unsigned int domain_id;
    unsigned int agent_id;
    uint32_t state;
    struct mod_pd_power_state_transition_notification_params *event_params =
        ((struct mod_pd_power_state_transition_notification_params *)(void *)
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
    if (status != FWK_SUCCESS)
        return status;
#endif
    return status;
}

/* SCMI Power Domain Management Protocol Definition */
const struct fwk_module module_scmi_power_domain = {
    .name = "SCMI Power Domain Management Protocol",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_pd_init,
    .bind = scmi_pd_bind,
    .start = scmi_pd_start,
    .process_bind_request = scmi_pd_process_bind_request,
#ifdef BUILD_HAS_MOD_DEBUG
    .event_count = SCMI_PD_EVENT_IDX_COUNT,
    .process_event = scmi_pd_process_event,
    .process_notification = scmi_pd_process_notification,
#endif
};
