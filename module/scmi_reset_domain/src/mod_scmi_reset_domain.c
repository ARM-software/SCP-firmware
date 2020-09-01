/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Reset Domain management protocol support.
 */

#include <internal/scmi_reset_domain.h>

#include <mod_reset_domain.h>
#include <mod_scmi.h>
#include <mod_scmi_reset_domain.h>

#include <fwk_assert.h>
#include <fwk_attributes.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <string.h>

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
#    include <mod_resource_perms.h>
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
#    define MOD_SCMI_RESET_DOMAIN_NOTIFICATION_COUNT 1
#endif

struct scmi_rd_ctx {
    /*! SCMI Reset Module Configuration */
    const struct mod_scmi_reset_domain_config *config;

    /* Bound module APIs */
    const struct mod_scmi_from_protocol_api *scmi_api;
    const struct mod_reset_domain_api *reset_api;

    /*! Number of reset domains available on platform */
    uint8_t plat_reset_domain_count;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /*! SCMI notification_id */
    fwk_id_t notification_id;

    /* SCMI notification API */
    const struct mod_scmi_notification_api *scmi_notification_api;
#endif

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    /* SCMI Resource Permissions API */
    const struct mod_res_permissions_api *res_perms_api;
#endif
};

static int protocol_version_handler(fwk_id_t service_id,
                                    const uint32_t *payload);

static int protocol_attributes_handler(fwk_id_t service_id,
                                       const uint32_t *payload);
static int protocol_message_attributes_handler(fwk_id_t service_id,
                                               const uint32_t *payload);
static int reset_attributes_handler(fwk_id_t service_id,
                                    const uint32_t *payload);
static int reset_request_handler(fwk_id_t service_id,
                                 const uint32_t *payload);
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int reset_notify_handler(fwk_id_t service_id,
                                const uint32_t *payload);
#endif

/*
 * Internal variables
 */

static struct scmi_rd_ctx scmi_rd_ctx;

static int (*msg_handler_table[])(fwk_id_t, const uint32_t *) = {
    [MOD_SCMI_PROTOCOL_VERSION] = protocol_version_handler,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = protocol_attributes_handler,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
         protocol_message_attributes_handler,
    [MOD_SCMI_RESET_DOMAIN_ATTRIBUTES] = reset_attributes_handler,
    [MOD_SCMI_RESET_REQUEST] = reset_request_handler,
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_RESET_NOTIFY] = reset_notify_handler,
#endif
};

static unsigned int payload_size_table[] = {
    [MOD_SCMI_PROTOCOL_VERSION] = 0,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
         sizeof(struct scmi_protocol_message_attributes_a2p),
    [MOD_SCMI_RESET_DOMAIN_ATTRIBUTES] =
         sizeof(struct scmi_reset_domain_attributes_a2p),
    [MOD_SCMI_RESET_REQUEST] = sizeof(struct scmi_reset_domain_request_a2p),
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_RESET_NOTIFY] = sizeof(struct scmi_reset_domain_notify_a2p),
#endif
};

/*
 * Reset domain management protocol implementation
 */

static int protocol_version_handler(fwk_id_t service_id,
                                    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a outmsg = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_RESET_DOMAIN,
    };

    scmi_rd_ctx.scmi_api->respond(service_id, &outmsg, sizeof(outmsg));

    return FWK_SUCCESS;
}

static int protocol_attributes_handler(fwk_id_t service_id,
                                       const uint32_t *payload)
{
    struct scmi_reset_domain_protocol_attributes_p2a outmsg = {
        .status = SCMI_SUCCESS,
    };
    int status = 0;
    unsigned int agent_id = 0;

    status = scmi_rd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= scmi_rd_ctx.config->agent_count)
        return FWK_E_PARAM;

    outmsg.attributes = scmi_rd_ctx.config->
                            agent_table[agent_id].agent_domain_count;

    scmi_rd_ctx.scmi_api->respond(service_id, &outmsg, sizeof(outmsg));

    return FWK_SUCCESS;
}

static int protocol_message_attributes_handler(fwk_id_t service_id,
                                               const uint32_t *payload)
{
    struct scmi_protocol_message_attributes_p2a outmsg = {
        .status = SCMI_NOT_FOUND,
    };
    size_t outmsg_size = sizeof(outmsg.status);
    struct scmi_protocol_message_attributes_a2p params = { 0 };

    params = *(const struct scmi_protocol_message_attributes_a2p *)payload;

    if ((params.message_id < FWK_ARRAY_SIZE(msg_handler_table)) &&
        msg_handler_table[params.message_id]) {
        outmsg.status = SCMI_SUCCESS;
        outmsg_size = sizeof(outmsg);
    }

    scmi_rd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);

    return FWK_SUCCESS;
}

/*
 * Given a service identifier, retrieve a agent identifier
 */
static int get_agent_id(fwk_id_t service_id, unsigned int* agent_id)
{
    int status;

    status = scmi_rd_ctx.scmi_api->get_agent_id(service_id, agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (*agent_id >= scmi_rd_ctx.config->agent_count)
        return FWK_E_PARAM;

    return FWK_SUCCESS;
}

/*
 * Given a service identifier, retrieve a pointer to its agent's
 * \c mod_scmi_reset_domain_agent structure within the agent table.
 */
static int get_agent_entry(fwk_id_t service_id,
                           const struct mod_scmi_reset_domain_agent **agent)
{
    int status = 0;
    unsigned int agent_id = 0;

    status = get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    *agent = &scmi_rd_ctx.config->agent_table[agent_id];

    return FWK_SUCCESS;
}

static int get_reset_device(fwk_id_t service_id,
                            unsigned int domain_id,
                            const struct mod_scmi_reset_domain_device **device)
{
    int status;
    const struct mod_scmi_reset_domain_agent *agent_entry = NULL;

    status = get_agent_entry(service_id, &agent_entry);
    if (status != FWK_SUCCESS)
        return status;

    if (domain_id >= agent_entry->agent_domain_count)
        return FWK_E_RANGE;

    *device = &agent_entry->device_table[domain_id];

    fwk_assert(fwk_module_is_valid_element_id((*device)->element_id));

    if (!fwk_module_is_valid_element_id((*device)->element_id))
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

/*
 * Reset Request Policy Handler
 */
FWK_WEAK int scmi_reset_domain_reset_request_policy(
    enum mod_scmi_reset_domain_policy_status *policy_status,
    enum mod_reset_domain_mode *mode,
    uint32_t *reset_state,
    uint32_t agent_id,
    uint32_t domain_id)
{
    *policy_status = MOD_SCMI_RESET_DOMAIN_EXECUTE_MESSAGE_HANDLER;

    return FWK_SUCCESS;
}


static int reset_attributes_handler(fwk_id_t service_id,
                                    const uint32_t *payload)
{
    const struct mod_scmi_reset_domain_device *reset_device = NULL;
    struct mod_reset_domain_dev_config *reset_dev_config = NULL;
    struct scmi_reset_domain_attributes_a2p params = { 0 };
    struct scmi_reset_domain_attributes_p2a outmsg = {
        .status = SCMI_GENERIC_ERROR,
    };
    size_t outmsg_size = sizeof(outmsg.status);
    int status = FWK_SUCCESS;

    params = *(const struct scmi_reset_domain_attributes_a2p *)payload;

    status = get_reset_device(service_id, params.domain_id, &reset_device);
    if (status != FWK_SUCCESS) {
        outmsg.status = SCMI_NOT_FOUND;
        goto exit;
    }

    reset_dev_config = (struct mod_reset_domain_dev_config *)
                           fwk_module_get_data(reset_device->element_id);
    /*
     * Currently: no support for async reset.
     */
    outmsg.flags &= ~SCMI_RESET_DOMAIN_ATTR_ASYNC;

    if (reset_dev_config->capabilities & MOD_RESET_DOMAIN_CAP_NOTIFICATION)
        outmsg.flags |= SCMI_RESET_DOMAIN_ATTR_NOTIF;

    outmsg.latency = reset_dev_config->latency;

    strncpy((char *)outmsg.name, fwk_module_get_name(reset_device->element_id),
            sizeof(outmsg.name) - 1);

    outmsg.status = SCMI_SUCCESS;
    outmsg_size = sizeof(outmsg);

exit:
    scmi_rd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);

    return status;
}

static int reset_request_handler(fwk_id_t service_id,
                                 const uint32_t *payload)
{
    int status;
    unsigned int agent_id;
    struct mod_reset_domain_dev_config *reset_dev_config;
    struct scmi_reset_domain_request_a2p params = { 0 };
    struct scmi_reset_domain_request_p2a outmsg = {
        .status = SCMI_NOT_FOUND
    };
    enum mod_reset_domain_mode mode = MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT;

    size_t outmsg_size = sizeof(outmsg.status);
    const struct mod_reset_domain_api *reset_api = scmi_rd_ctx.reset_api;
    const struct mod_scmi_reset_domain_device *reset_device = NULL;
    enum mod_scmi_reset_domain_policy_status policy_status;
    uint32_t reset_state;

    params = *(const struct scmi_reset_domain_request_a2p *)payload;

    status = scmi_rd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    if (params.domain_id >= scmi_rd_ctx.plat_reset_domain_count) {
        status = FWK_E_PARAM;
        goto exit;
    }

    status = get_reset_device(service_id, params.domain_id, &reset_device);
    if (status != FWK_SUCCESS)
        goto exit;

    reset_dev_config = (struct mod_reset_domain_dev_config *)
                       fwk_module_get_data(reset_device->element_id);

    /* Check if explicit assert is requested.
     * Note, valid request in flags
     *       b000 Explicit de-assert request.
     *       b001 Auto Reset request.
     *       b010 Explicit reset request.
     *       b101 Auto Reset Async.
     */
    if (!(params.flags & SCMI_RESET_DOMAIN_AUTO)) {
        /* If auto reset is not requested then check if device supports explicit
         * assert/de-assert reset.
         */
        if (!(reset_dev_config->modes &
            (MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT |
            MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT))) {

            outmsg.status = SCMI_NOT_SUPPORTED;
            goto exit;
        } else {
           if (params.flags & SCMI_RESET_DOMAIN_EXPLICIT)
               mode = MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT;
        }
    } else {
        mode = SCMI_RESET_DOMAIN_AUTO;
    }

    /* Handle async reset request. */
    if (params.flags & SCMI_RESET_DOMAIN_ASYNC) {
        /* Async reset request is valid only in auto reset mode
         */
        if (!(params.flags & SCMI_RESET_DOMAIN_AUTO)) {
            outmsg.status = SCMI_INVALID_PARAMETERS;
            goto exit;
        }

        /* Return not supported as associated device does not support
         * Async reset.
         */
        if (!(reset_dev_config->modes &
            MOD_RESET_DOMAIN_MODE_AUTO_RESET_ASYNC)) {

            outmsg.status = SCMI_NOT_SUPPORTED;
            goto exit;
        } else {
            mode |= MOD_RESET_DOMAIN_MODE_AUTO_RESET_ASYNC;
        }
    }

    reset_state = params.reset_state;
    status = scmi_reset_domain_reset_request_policy(&policy_status,
        &mode, &reset_state, agent_id, params.domain_id);

    if (status != FWK_SUCCESS) {
        outmsg.status = SCMI_GENERIC_ERROR;
        goto exit;
    }
    if (policy_status == MOD_SCMI_RESET_DOMAIN_SKIP_MESSAGE_HANDLER) {
        outmsg.status = SCMI_SUCCESS;
        goto exit;
    }

    outmsg.status = SCMI_NOT_SUPPORTED;
    status = reset_api->set_reset_state(reset_device->element_id,
                                        mode,
                                        reset_state,
                                        (uintptr_t)agent_id);
    if (status != FWK_SUCCESS) {
        if (status == FWK_E_STATE)
            outmsg.status = SCMI_HARDWARE_ERROR;
        goto exit;
    }

    outmsg.status = SCMI_SUCCESS;
    outmsg_size = sizeof(outmsg);

exit:
    scmi_rd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);

    return status;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int reset_notify_handler(fwk_id_t service_id,
                                const uint32_t *payload)
{
    unsigned int agent_id;
    int status;
    unsigned int domain_id;
    const struct scmi_reset_domain_notify_a2p *parameters;
    struct scmi_reset_domain_notify_p2a outmsg = {
        .status = SCMI_GENERIC_ERROR,
    };

    status = get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    parameters = (const struct scmi_reset_domain_notify_a2p *)payload;

    domain_id = parameters->domain_id;
    if (domain_id >= scmi_rd_ctx.config->
        agent_table[agent_id].agent_domain_count) {
        status = FWK_SUCCESS;
        outmsg.status = SCMI_NOT_FOUND;
        goto exit;
    }

    if (parameters->notify_enable)
        scmi_rd_ctx.scmi_notification_api->scmi_notification_add_subscriber(
            MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN,
            domain_id,
            MOD_SCMI_RESET_NOTIFY,
            service_id);
    else
        scmi_rd_ctx.scmi_notification_api->scmi_notification_remove_subscriber(
            MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN,
            agent_id,
            domain_id,
            MOD_SCMI_RESET_NOTIFY);

    outmsg.status = SCMI_SUCCESS;

exit:
    scmi_rd_ctx.scmi_api->respond(service_id, &outmsg,
                                  (outmsg.status == SCMI_SUCCESS) ?
                                  sizeof(outmsg) :
                                  sizeof(outmsg.status));

    return status;
}

static void scmi_reset_issued_notify(uint32_t domain_id,
                                     uint32_t reset_state,
                                     uintptr_t cookie)
{
    struct scmi_reset_domain_issued_p2a reset_issued = {
         .agent_id = (uint32_t)cookie
    };

    reset_issued.domain_id = domain_id;
    reset_issued.reset_state = reset_state;

    scmi_rd_ctx.scmi_notification_api->scmi_notification_notify(
        MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN,
        MOD_SCMI_RESET_NOTIFY,
        MOD_SCMI_RESET_ISSUED,
        &reset_issued,
        sizeof(reset_issued));
}
#endif

/*
 * SCMI Resource Permissions handler
 */
#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
static unsigned int get_reset_domain_id(const uint32_t *payload)
{
    struct scmi_reset_domain_request_a2p *params;

    params = (struct scmi_reset_domain_request_a2p *)payload;
    return params->domain_id;
}

static int scmi_reset_domain_permissions_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    enum mod_res_perms_permissions perms;
    unsigned int agent_id, domain_id;
    int status;

    status = scmi_rd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return FWK_E_ACCESS;

    if (message_id < 3) {
        perms = scmi_rd_ctx.res_perms_api->agent_has_protocol_permission(
            agent_id, MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN);
        if (perms == MOD_RES_PERMS_ACCESS_ALLOWED)
            return FWK_SUCCESS;
        return FWK_E_ACCESS;
    }

    domain_id = get_reset_domain_id(payload);

    perms = scmi_rd_ctx.res_perms_api->agent_has_resource_permission(
        agent_id, MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN, message_id, domain_id);

    if (perms == MOD_RES_PERMS_ACCESS_ALLOWED)
        return FWK_SUCCESS;
    else
        return FWK_E_ACCESS;
}
#endif

/*
 * SCMI module -> SCMI reset module interface
 */
static int scmi_reset_get_scmi_protocol_id(fwk_id_t protocol_id,
                                           uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN;

    return FWK_SUCCESS;
}

static int scmi_reset_message_handler(fwk_id_t protocol_id,
                                      fwk_id_t service_id,
                                      const uint32_t *payload,
                                      size_t payload_size,
                                      unsigned int message_id)
{
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(msg_handler_table) ==
                  FWK_ARRAY_SIZE(payload_size_table),
                  "[SCMI] reset domain protocol table sizes not consistent");

    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(msg_handler_table)) {
        return_value = SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    if (scmi_reset_domain_permissions_handler(
            service_id, payload, payload_size, message_id) != FWK_SUCCESS) {
        return_value = SCMI_DENIED;
        goto error;
    }
#endif

    return msg_handler_table[message_id](service_id, payload);

error:
    scmi_rd_ctx.scmi_api->respond(service_id,
                                  &return_value, sizeof(return_value));
    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api scmi_reset_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_reset_get_scmi_protocol_id,
    .message_handler = scmi_reset_message_handler
};

/*
 * Framework handlers
 */

static int scmi_reset_init(fwk_id_t module_id,
                           unsigned int element_count,
                           const void *data)
{
    const struct mod_scmi_reset_domain_config *config;

    config = (const struct mod_scmi_reset_domain_config *)data;

    if ((config == NULL) || (config->agent_table == NULL))
        return FWK_E_PARAM;

    scmi_rd_ctx.config = config;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_reset_init_notifications(void)
{

    fwk_assert(scmi_rd_ctx.config->agent_count != 0);

    return scmi_rd_ctx.scmi_notification_api->scmi_notification_init(
        MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN,
        scmi_rd_ctx.config->agent_count,
        scmi_rd_ctx.plat_reset_domain_count,
        MOD_SCMI_RESET_DOMAIN_NOTIFICATION_COUNT);
}
#endif

static int scmi_reset_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
                             FWK_ID_API(FWK_MODULE_IDX_SCMI,
                                        MOD_SCMI_API_IDX_PROTOCOL),
                             &scmi_rd_ctx.scmi_api);
    if (status != FWK_SUCCESS)
        return status;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_NOTIFICATION),
        &scmi_rd_ctx.scmi_notification_api);
    if (status != FWK_SUCCESS)
        return status;
#endif

    scmi_rd_ctx.plat_reset_domain_count = fwk_module_get_element_count(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESET_DOMAIN));
    if (scmi_rd_ctx.plat_reset_domain_count == 0)
        return FWK_E_SUPPORT;


#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &scmi_rd_ctx.res_perms_api);
    if (status != FWK_SUCCESS)
        return status;
#endif

    return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_RESET_DOMAIN),
                           FWK_ID_API(FWK_MODULE_IDX_RESET_DOMAIN, 0),
                           &scmi_rd_ctx.reset_api);
}

static int scmi_reset_process_bind_request(fwk_id_t source_id,
                                           fwk_id_t target_id,
                                           fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_SCMI_RESET_DOMAIN_PROTOCOL_API:
        *api = &scmi_reset_mod_scmi_to_protocol_api;
        break;

    default:
        return FWK_E_ACCESS;
    }

    return FWK_SUCCESS;
}

#if defined(BUILD_HAS_SCMI_NOTIFICATIONS) && defined(BUILD_HAS_NOTIFICATION)
static int scmi_reset_process_notification(const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct mod_reset_domain_notification_event_params* params =
        (struct mod_reset_domain_notification_event_params*)event->params;

    if (!fwk_id_is_equal(scmi_rd_ctx.notification_id,
                         event->id))
        return FWK_E_SUPPORT;

    scmi_reset_issued_notify(params->domain_id, params->reset_state,
                             params->cookie);

    return FWK_SUCCESS;
}

static int scmi_reset_start(fwk_id_t id)
{
    int status;
    struct mod_reset_domain_config *config;
    config = (struct mod_reset_domain_config*)fwk_module_get_data(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESET_DOMAIN)
        );

    scmi_rd_ctx.notification_id = config->notification_id;

    status = scmi_reset_init_notifications();
    if (status != FWK_SUCCESS)
        return status;

    return fwk_notification_subscribe(
        scmi_rd_ctx.notification_id,
        FWK_ID_MODULE(FWK_MODULE_IDX_RESET_DOMAIN),
        id);
}
#endif

/* SCMI Reset Domain Management Protocol Definition */
const struct fwk_module module_scmi_reset_domain = {
    .name = "SCMI Reset Domain Management Protocol",
    .api_count = MOD_SCMI_RESET_DOMAIN_API_COUNT,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_reset_init,
    .bind = scmi_reset_bind,
    .process_bind_request = scmi_reset_process_bind_request,
#if defined(BUILD_HAS_SCMI_NOTIFICATIONS) && defined(BUILD_HAS_NOTIFICATION)
    .start = scmi_reset_start,
    .process_notification = scmi_reset_process_notification,
#endif
};
