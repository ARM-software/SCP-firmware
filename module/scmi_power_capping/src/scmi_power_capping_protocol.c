/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI power capping and monitoring protocol completer.
 */
#include "internal/scmi_power_capping.h"
#include "internal/scmi_power_capping_fast_channels.h"
#include "internal/scmi_power_capping_protocol.h"
#include "mod_power_allocator.h"
#include "mod_power_coordinator.h"
#include "mod_power_meter.h"

#include <mod_scmi.h>
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
#    include <mod_resource_perms.h>
#endif

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_string.h>

#include <stdbool.h>

#define MOD_SCMI_POWER_CAPPING_NOTIFICATION_COUNT 1

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
struct pcapping_protocol_event_parameters {
    uint32_t domain_idx;
    fwk_id_t service_id;
};
#endif

/*
 * SCMI Power Capping message handlers.
 */

static int scmi_power_capping_protocol_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_power_capping_protocol_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_power_capping_protocol_msg_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_power_capping_domain_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_power_capping_cap_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_power_capping_cap_set_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_power_capping_pai_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_power_capping_pai_set_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_power_capping_measurements_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_power_capping_cap_notify_handler(
    fwk_id_t cap_service_id,
    const uint32_t *payload);
static int scmi_power_capping_measurements_notify_handler(
    fwk_id_t cap_service_id,
    const uint32_t *payload);
#endif
#ifdef BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS
static int scmi_power_capping_describe_fast_channel_handler(
    fwk_id_t cap_service_id,
    const uint32_t *payload);
#endif
/*
 * Internal variables.
 */

static struct {
    /* Number of power capping domains */
    unsigned int power_capping_domain_count;

    /* Table of power capping domain ctxs */
    struct mod_scmi_power_capping_domain_context
        *power_capping_domain_ctx_table;

    /* SCMI protocol module to SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /*  Power management related APIs. */
    const struct mod_scmi_power_capping_power_apis *power_management_apis;
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    /* SCMI Resource Permissions API */
    const struct mod_res_permissions_api *res_perms_api;
#endif
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /* SCMI notification API */
    const struct mod_scmi_notification_api *scmi_notification_api;
#endif
} pcapping_protocol_ctx;

static const fwk_id_t pcapping_protocol_cap_notification =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_POWER_ALLOCATOR,
        MOD_POWER_ALLOCATOR_NOTIFICATION_IDX_CAP_CHANGED);
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static const fwk_id_t pcapping_protocol_pai_notification =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_POWER_COORDINATOR,
        MOD_POWER_COORDINATOR_NOTIFICATION_IDX_PERIOD_CHANGED);
static const fwk_id_t pcapping_protocol_power_measurements_notification =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_POWER_METER,
        MOD_POWER_METER_NOTIFICATION_IDX_MEASUREMENTS_CHANGED);
#endif

static int (*handler_table[MOD_SCMI_POWER_CAPPING_COMMAND_COUNT])(
    fwk_id_t,
    const uint32_t *) = {
    [MOD_SCMI_PROTOCOL_VERSION] = scmi_power_capping_protocol_version_handler,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] =
        scmi_power_capping_protocol_attributes_handler,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_power_capping_protocol_msg_attributes_handler,
    [MOD_SCMI_POWER_CAPPING_DOMAIN_ATTRIBUTES] =
        scmi_power_capping_domain_attributes_handler,
    [MOD_SCMI_POWER_CAPPING_CAP_GET] = scmi_power_capping_cap_get_handler,
    [MOD_SCMI_POWER_CAPPING_CAP_SET] = scmi_power_capping_cap_set_handler,
    [MOD_SCMI_POWER_CAPPING_PAI_GET] = scmi_power_capping_pai_get_handler,
    [MOD_SCMI_POWER_CAPPING_PAI_SET] = scmi_power_capping_pai_set_handler,
    [MOD_SCMI_POWER_CAPPING_MEASUREMENTS_GET] =
        scmi_power_capping_measurements_get_handler,
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_POWER_CAPPING_CAP_NOTIFY] = scmi_power_capping_cap_notify_handler,
    [MOD_SCMI_POWER_CAPPING_MEASUREMENTS_NOTIFY] =
        scmi_power_capping_measurements_notify_handler,
#endif
#ifdef BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS
    [MOD_SCMI_POWER_CAPPING_DESCRIBE_FAST_CHANNEL] =
        scmi_power_capping_describe_fast_channel_handler,
#endif
};

static unsigned int payload_size_table[MOD_SCMI_POWER_CAPPING_COMMAND_COUNT] = {
    [MOD_SCMI_PROTOCOL_VERSION] = 0,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        sizeof(struct scmi_protocol_message_attributes_a2p),
    [MOD_SCMI_POWER_CAPPING_DOMAIN_ATTRIBUTES] =
        sizeof(struct scmi_power_capping_domain_attributes_a2p),
    [MOD_SCMI_POWER_CAPPING_CAP_GET] =
        sizeof(struct scmi_power_capping_cap_get_a2p),
    [MOD_SCMI_POWER_CAPPING_CAP_SET] =
        sizeof(struct scmi_power_capping_cap_set_a2p),
    [MOD_SCMI_POWER_CAPPING_PAI_GET] =
        sizeof(struct scmi_power_capping_pai_get_a2p),
    [MOD_SCMI_POWER_CAPPING_PAI_SET] =
        sizeof(struct scmi_power_capping_pai_set_a2p),
    [MOD_SCMI_POWER_CAPPING_MEASUREMENTS_GET] =
        sizeof(struct scmi_power_capping_measurements_get_a2p),
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [MOD_SCMI_POWER_CAPPING_CAP_NOTIFY] =
        sizeof(struct scmi_power_capping_cap_notify_a2p),
    [MOD_SCMI_POWER_CAPPING_MEASUREMENTS_NOTIFY] =
        sizeof(struct scmi_power_capping_measurements_notify_a2p),
#endif
#ifdef BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS
    [MOD_SCMI_POWER_CAPPING_DESCRIBE_FAST_CHANNEL] =
        sizeof(struct scmi_power_capping_describe_fc_a2p),
#endif
};

static_assert(
    FWK_ARRAY_SIZE(handler_table) == FWK_ARRAY_SIZE(payload_size_table),
    "[SCMI] Power capping protocol table sizes not consistent");

/*
 * Helper functions
 */

/*
 * Static helper function to process protocol, message and resource permissions.
 */
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
static int scmi_power_capping_permissions_handler(
    unsigned int message_id,
    fwk_id_t service_id,
    const uint32_t *payload);
#endif

/*
 * Static helper function to check if a message is implemented.
 */
static bool scmi_power_capping_is_msg_implemented(
    fwk_id_t service_id,
    uint32_t message_id)
{
    return (
        (message_id < FWK_ARRAY_SIZE(handler_table)) &&
        (handler_table[message_id] != NULL));
}

/*
 * Static helper function to get domain context
 */
static struct mod_scmi_power_capping_domain_context *get_domain_ctx(
    unsigned int domain_idx)
{
    return &pcapping_protocol_ctx.power_capping_domain_ctx_table[domain_idx];
}

/*
 * Static helper function to populate domain attributes return values.
 */
static inline void scmi_power_capping_populate_domain_attributes(
    struct scmi_power_capping_domain_attributes_p2a *return_values,
    unsigned int domain_idx)
{
    const struct mod_scmi_power_capping_domain_context *domain_ctx;
    const struct mod_scmi_power_capping_domain_config *config;

    domain_ctx = get_domain_ctx(domain_idx);
    config = domain_ctx->config;

    return_values->attributes = SCMI_POWER_CAPPING_DOMAIN_ATTRIBUTES(
        domain_ctx->cap_config_support,
        domain_ctx->pai_config_support,
        config->power_cap_unit);
    return_values->min_pai = config->min_pai;
    return_values->max_pai = config->max_pai;
    return_values->pai_step = config->pai_step;

#ifdef BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS
    return_values->attributes |= SCMI_POWER_CAPPING_DOMAIN_FCH_SUPPORT(
        pcapping_fast_channel_get_domain_supp(domain_idx));
#endif
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    return_values->attributes |=
        SCMI_POWER_CAPPING_DOMAIN_CAP_PAI_CHANGE_NOTIF_SUPPORT(
            config->cap_pai_change_notification_support);
    return_values->attributes |=
        SCMI_POWER_CAPPING_DOMAIN_MEASUREMENTS_NOTIF_SUPPORT(
            config->power_measurements_change_notification_support);
#endif
    return_values->min_power_cap = config->min_power_cap;
    return_values->max_power_cap = config->max_power_cap;
    return_values->power_cap_step = config->power_cap_step;
    return_values->max_sustainable_power = config->max_sustainable_power;
    return_values->parent_id = config->parent_idx;
}

/*
 * Static helper to handle error scmi responses
 */
static int scmi_power_capping_respond_error(
    fwk_id_t service_id,
    enum scmi_error scmi_error)
{
    int return_status = (int)scmi_error;
    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_status, sizeof(return_status));
}

/*
 * Static helper to check domain configuration
 */
static int pcapping_protocol_check_domain_configuration(
    const struct mod_scmi_power_capping_domain_config *config)
{
    if ((config->min_power_cap == (uint32_t)0) ||
        (config->max_power_cap == (uint32_t)0) ||
        (config->min_pai == (uint32_t)0) || (config->max_pai == (uint32_t)0)) {
        return FWK_E_DATA;
    }

    if (config->min_pai != config->max_pai) {
        if (config->pai_step == (uint32_t)0) {
            return FWK_E_DATA;
        }
    }

    if (config->min_power_cap != config->max_power_cap) {
        if (config->power_cap_step == (uint32_t)0) {
            return FWK_E_DATA;
        }
    }

    return FWK_SUCCESS;
}

static void pcapping_protocol_set_cap_config_support(
    struct mod_scmi_power_capping_domain_context *domain_ctx,
    const struct mod_scmi_power_capping_domain_config *config)
{
    domain_ctx->cap_config_support =
        (config->min_power_cap != config->max_power_cap);

    domain_ctx->pai_config_support = (config->min_pai != config->max_pai);
}

/*
 * Static helper to process power cap notifications.
 */
static int pcapping_protocol_process_cap_fwk_notification(
    unsigned int domain_idx,
    struct mod_scmi_power_capping_domain_context *domain_ctx)
{
    fwk_id_t cap_pending_service_id;
    struct scmi_power_capping_cap_set_p2a return_values;
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    int status;
    struct fwk_event scmi_notification_event = {
        .target_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING),
    };

    struct pcapping_protocol_event_parameters *scmi_notif_event_params =
        (struct pcapping_protocol_event_parameters *)
            scmi_notification_event.params;

    if (domain_ctx->config->cap_pai_change_notification_support) {
        scmi_notification_event.id = FWK_ID_EVENT(
            FWK_MODULE_IDX_SCMI_POWER_CAPPING,
            SCMI_POWER_CAPPING_EVENT_IDX_CAP_PAI_NOTIFY_PROCESS);

        scmi_notif_event_params->service_id =
            domain_ctx->cap_notification_service_id;
        domain_ctx->cap_notification_service_id = FWK_ID_NONE;
        scmi_notif_event_params->domain_idx = domain_idx;

        status = fwk_put_event(&scmi_notification_event);

        if (status != FWK_SUCCESS) {
            return status;
        }
    }
#endif

    cap_pending_service_id = domain_ctx->cap_pending_service_id;

    if (fwk_id_is_equal(cap_pending_service_id, FWK_ID_NONE)) {
        return FWK_SUCCESS;
    }

    domain_ctx->cap_pending_service_id = FWK_ID_NONE;

    return_values.status = (int)SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        cap_pending_service_id, &return_values, sizeof(return_values));
}

/*
 * Static helper to process PAI notifications.
 */
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int pcapping_protocol_process_pai_fwk_notification(
    unsigned int domain_idx,
    struct mod_scmi_power_capping_domain_context *domain_ctx)
{
    struct fwk_event scmi_notification_event = {
        .target_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING),
    };

    struct pcapping_protocol_event_parameters *scmi_notif_event_params =
        (struct pcapping_protocol_event_parameters *)
            scmi_notification_event.params;

    if (!domain_ctx->config->cap_pai_change_notification_support) {
        return FWK_SUCCESS;
    }

    scmi_notification_event.id = FWK_ID_EVENT(
        FWK_MODULE_IDX_SCMI_POWER_CAPPING,
        SCMI_POWER_CAPPING_EVENT_IDX_CAP_PAI_NOTIFY_PROCESS);
    scmi_notif_event_params->service_id =
        domain_ctx->pai_notification_service_id;
    domain_ctx->pai_notification_service_id = FWK_ID_NONE;
    scmi_notif_event_params->domain_idx = domain_idx;

    return fwk_put_event(&scmi_notification_event);
}

/*
 * Static helper to process power measurements notifications.
 */
static int pcapping_protocol_process_power_measurements_fwk_notification(
    unsigned int domain_idx,
    struct mod_scmi_power_capping_domain_context *domain_ctx)
{
    struct fwk_event scmi_notification_event = {
        .target_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING),
    };

    struct pcapping_protocol_event_parameters *scmi_notif_event_params =
        (struct pcapping_protocol_event_parameters *)
            scmi_notification_event.params;

    if (!domain_ctx->config->power_measurements_change_notification_support) {
        return FWK_SUCCESS;
    }

    scmi_notification_event.id = FWK_ID_EVENT(
        FWK_MODULE_IDX_SCMI_POWER_CAPPING,
        SCMI_POWER_CAPPING_EVENT_IDX_MEASUREMENT_NOTIFY_PROCESS);
    scmi_notif_event_params->service_id = FWK_ID_NONE;
    scmi_notif_event_params->domain_idx = domain_idx;

    return fwk_put_event(&scmi_notification_event);
}
#endif

/*
 * Power capping protocol implementation
 */
static int scmi_power_capping_protocol_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_POWER_CAPPING,
    };

    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_power_capping_protocol_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_attributes_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
        .attributes = pcapping_protocol_ctx.power_capping_domain_count,
    };

    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_power_capping_protocol_msg_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *parameters;
    struct scmi_protocol_message_attributes_p2a return_values;
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    int status;
#endif
    parameters = (const struct scmi_protocol_message_attributes_a2p *)payload;

    if (!scmi_power_capping_is_msg_implemented(
            service_id, parameters->message_id)) {
        return scmi_power_capping_respond_error(service_id, SCMI_NOT_FOUND);
    }

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    status = scmi_power_capping_permissions_handler(
        parameters->message_id, service_id, payload);
    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(service_id, SCMI_NOT_FOUND);
    }
#endif

#ifdef BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS
    return_values.attributes =
        pcapping_fast_channel_get_msg_supp(parameters->message_id) ?
        SCMI_POWER_CAPPING_FCH_AVAIL :
        SCMI_POWER_CAPPING_FCH_NOT_AVAIL;
#else
    return_values.attributes = SCMI_POWER_CAPPING_FCH_NOT_AVAIL;
#endif

    return_values.status = SCMI_SUCCESS;
    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_power_capping_domain_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_power_capping_domain_attributes_a2p *parameters;
    struct scmi_power_capping_domain_attributes_p2a return_values = { 0 };

    parameters =
        (const struct scmi_power_capping_domain_attributes_a2p *)payload;

    scmi_power_capping_populate_domain_attributes(
        &return_values, parameters->domain_id);

    fwk_str_strncpy(
        (char *)return_values.name,
        fwk_module_get_element_name(FWK_ID_ELEMENT(
            FWK_MODULE_IDX_SCMI_POWER_CAPPING, parameters->domain_id)),
        sizeof(return_values.name) - 1);

    return_values.status = SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_power_capping_cap_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_power_capping_cap_get_a2p *parameters;
    struct scmi_power_capping_cap_get_p2a return_values;
    struct mod_scmi_power_capping_domain_context *ctx;
    int status;

    parameters = (const struct scmi_power_capping_cap_get_a2p *)payload;

    ctx = get_domain_ctx(parameters->domain_id);

    status = pcapping_protocol_ctx.power_management_apis->power_allocator_api
                 ->get_cap(
                     ctx->config->power_allocator_domain_id,
                     &return_values.power_cap);

    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(service_id, SCMI_GENERIC_ERROR);
    }

    return_values.status = SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_power_capping_cap_set_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_power_capping_cap_set_a2p *parameters;
    struct scmi_power_capping_cap_set_p2a return_values;
    struct mod_scmi_power_capping_domain_context *ctx;
    bool async_flag;
    bool delayed_res_flag;
    fwk_id_t domain_id;
    int status;

    parameters = (const struct scmi_power_capping_cap_set_a2p *)payload;

    if ((parameters->flags & SCMI_POWER_CAPPING_INVALID_MASK) != 0) {
        return scmi_power_capping_respond_error(
            service_id, SCMI_INVALID_PARAMETERS);
    }

    ctx = get_domain_ctx(parameters->domain_id);

    if (!ctx->cap_config_support) {
        return scmi_power_capping_respond_error(service_id, SCMI_NOT_SUPPORTED);
    }

    async_flag = (parameters->flags & SCMI_POWER_CAPPING_ASYNC_FLAG_MASK) ==
        SCMI_POWER_CAPPING_ASYNC_FLAG_MASK;

    if (async_flag) {
        delayed_res_flag =
            !((parameters->flags & SCMI_POWER_CAPPING_IGN_DEL_RES_FLAG_MASK) ==
              SCMI_POWER_CAPPING_IGN_DEL_RES_FLAG_MASK);

        if (delayed_res_flag) {
            return scmi_power_capping_respond_error(
                service_id, SCMI_NOT_SUPPORTED);
        }
    }

    if (((parameters->power_cap < ctx->config->min_power_cap) ||
         (parameters->power_cap > ctx->config->max_power_cap)) &&
        (parameters->power_cap != SCMI_POWER_CAPPING_DISABLE_CAP_VALUE)) {
        return scmi_power_capping_respond_error(service_id, SCMI_OUT_OF_RANGE);
    }

    if (!fwk_id_is_equal(ctx->cap_pending_service_id, FWK_ID_NONE)) {
        return scmi_power_capping_respond_error(service_id, SCMI_BUSY);
    }

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    if (ctx->config->cap_pai_change_notification_support) {
        if (!fwk_id_is_equal(ctx->cap_notification_service_id, FWK_ID_NONE)) {
            return scmi_power_capping_respond_error(service_id, SCMI_BUSY);
        }
    }
#endif

    domain_id = ctx->config->power_allocator_domain_id;

    status = pcapping_protocol_ctx.power_management_apis->power_allocator_api
                 ->set_cap(domain_id, parameters->power_cap);

    if (status == FWK_PENDING) {
        ctx->cap_pending_service_id = service_id;
        status = FWK_SUCCESS;
        if (!async_flag) {
            return status;
        }
    }

    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(service_id, SCMI_GENERIC_ERROR);
    }

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    if (ctx->config->cap_pai_change_notification_support) {
        ctx->cap_notification_service_id = service_id;
    }
#endif

    return_values.status = SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_power_capping_pai_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_power_capping_pai_get_a2p *parameters;
    struct scmi_power_capping_pai_get_p2a return_values;
    struct mod_scmi_power_capping_domain_context *ctx;
    uint32_t pai;
    int status;

    parameters = (const struct scmi_power_capping_pai_get_a2p *)payload;
    ctx = get_domain_ctx(parameters->domain_id);

    status = pcapping_protocol_ctx.power_management_apis->power_coordinator_api
                 ->get_coordinator_period(
                     ctx->config->power_coordinator_domain_id, &pai);

    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(service_id, SCMI_GENERIC_ERROR);
    }

    return_values.pai = pai;
    return_values.status = SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_power_capping_pai_set_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_power_capping_pai_set_a2p *parameters;
    struct scmi_power_capping_pai_set_p2a return_values;
    struct mod_scmi_power_capping_domain_context *ctx;
    int status;

    parameters = (const struct scmi_power_capping_pai_set_a2p *)payload;
    ctx = get_domain_ctx(parameters->domain_id);

    if (parameters->flags != SCMI_POWER_CAPPING_PAI_RESERVED_FLAG) {
        return scmi_power_capping_respond_error(
            service_id, SCMI_INVALID_PARAMETERS);
    }

    if (!ctx->pai_config_support) {
        return scmi_power_capping_respond_error(service_id, SCMI_NOT_SUPPORTED);
    }

    if ((parameters->pai < ctx->config->min_pai) ||
        (parameters->pai > ctx->config->max_pai)) {
        return scmi_power_capping_respond_error(service_id, SCMI_OUT_OF_RANGE);
    }

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    if (ctx->config->cap_pai_change_notification_support) {
        if (!fwk_id_is_equal(ctx->pai_notification_service_id, FWK_ID_NONE)) {
            return scmi_power_capping_respond_error(service_id, SCMI_BUSY);
        }
    }
#endif

    status = pcapping_protocol_ctx.power_management_apis->power_coordinator_api
                 ->set_coordinator_period(
                     ctx->config->power_coordinator_domain_id, parameters->pai);

    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(service_id, SCMI_GENERIC_ERROR);
    }
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    if (ctx->config->cap_pai_change_notification_support) {
        ctx->pai_notification_service_id = service_id;
    }
#endif

    return_values.status = SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

static int scmi_power_capping_measurements_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_power_capping_measurements_get_a2p *parameters;
    struct scmi_power_capping_measurements_get_p2a return_values;
    struct mod_scmi_power_capping_domain_context *ctx;
    int status;
    uint32_t power;
    uint32_t period;

    parameters =
        (const struct scmi_power_capping_measurements_get_a2p *)payload;
    ctx = get_domain_ctx(parameters->domain_id);

    status =
        pcapping_protocol_ctx.power_management_apis->power_meter_api->get_power(
            ctx->config->power_meter_domain_id, &power);

    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(service_id, SCMI_GENERIC_ERROR);
    }

    status = pcapping_protocol_ctx.power_management_apis->power_coordinator_api
                 ->get_coordinator_period(
                     ctx->config->power_coordinator_domain_id, &period);

    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(service_id, SCMI_GENERIC_ERROR);
    }

    return_values.pai = period;
    return_values.power = power;
    return_values.status = SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_power_capping_cap_notify_handler(
    fwk_id_t cap_service_id,
    const uint32_t *payload)
{
    struct scmi_power_capping_cap_notify_a2p *parameters;
    struct scmi_power_capping_cap_notify_p2a return_values;
    unsigned int agent_id;
    int status;

    parameters = (struct scmi_power_capping_cap_notify_a2p *)payload;

    if (parameters->notify_enable) {
        status = pcapping_protocol_ctx.scmi_notification_api
                     ->scmi_notification_add_subscriber(
                         MOD_SCMI_PROTOCOL_ID_POWER_CAPPING,
                         parameters->domain_id,
                         MOD_SCMI_POWER_CAPPING_CAP_NOTIFY,
                         cap_service_id);
    } else {
        status = pcapping_protocol_ctx.scmi_api->get_agent_id(
            cap_service_id, &agent_id);

        if (status != FWK_SUCCESS) {
            return scmi_power_capping_respond_error(
                cap_service_id, SCMI_GENERIC_ERROR);
        }

        status = pcapping_protocol_ctx.scmi_notification_api
                     ->scmi_notification_remove_subscriber(
                         MOD_SCMI_PROTOCOL_ID_POWER_CAPPING,
                         agent_id,
                         parameters->domain_id,
                         MOD_SCMI_POWER_CAPPING_CAP_NOTIFY);
    }

    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(
            cap_service_id, SCMI_GENERIC_ERROR);
    }

    return_values.status = SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        cap_service_id, &return_values, sizeof(return_values));
}

static int scmi_power_capping_measurements_notify_handler(
    fwk_id_t cap_service_id,
    const uint32_t *payload)
{
    struct scmi_power_capping_measurements_notify_a2p *parameters;
    struct scmi_power_capping_measurements_notify_p2a return_values;
    unsigned int agent_id;
    int status;

    parameters = (struct scmi_power_capping_measurements_notify_a2p *)payload;

    if (parameters->notify_enable) {
        status = pcapping_protocol_ctx.scmi_notification_api
                     ->scmi_notification_add_subscriber(
                         MOD_SCMI_PROTOCOL_ID_POWER_CAPPING,
                         parameters->domain_id,
                         MOD_SCMI_POWER_CAPPING_MEASUREMENTS_NOTIFY,
                         cap_service_id);
    } else {
        status = pcapping_protocol_ctx.scmi_api->get_agent_id(
            cap_service_id, &agent_id);

        if (status != FWK_SUCCESS) {
            return scmi_power_capping_respond_error(
                cap_service_id, SCMI_GENERIC_ERROR);
        }

        status = pcapping_protocol_ctx.scmi_notification_api
                     ->scmi_notification_remove_subscriber(
                         MOD_SCMI_PROTOCOL_ID_POWER_CAPPING,
                         agent_id,
                         parameters->domain_id,
                         MOD_SCMI_POWER_CAPPING_MEASUREMENTS_NOTIFY);
    }

    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(
            cap_service_id, SCMI_GENERIC_ERROR);
    }

    return_values.status = SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        cap_service_id, &return_values, sizeof(return_values));
}
#endif

#ifdef BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS
static int scmi_power_capping_describe_fast_channel_handler(
    fwk_id_t cap_service_id,
    const uint32_t *payload)
{
    const struct scmi_power_capping_describe_fc_a2p *parameters;
    struct scmi_power_capping_describe_fc_p2a return_values = { 0 };
    struct pcapping_fast_channel_info info;
    unsigned int domain_idx;
    uint32_t message_id;
    int status;

    parameters = (const struct scmi_power_capping_describe_fc_a2p *)payload;
    domain_idx = parameters->domain_id;
    message_id = parameters->message_id;

    status = pcapping_fast_channel_get_info(domain_idx, message_id, &info);

    if (status != FWK_SUCCESS) {
        return_values.status =
            status == FWK_E_RANGE ? SCMI_NOT_FOUND : SCMI_NOT_SUPPORTED;
        return pcapping_protocol_ctx.scmi_api->respond(
            cap_service_id, &return_values, sizeof(return_values.status));
    }

    return_values.chan_addr_high = (uint32_t)((uint64_t)info.fch_address >> 32);
    return_values.chan_addr_low = (uint32_t)info.fch_address;

    return_values.chan_size = info.fch_channel_size;

    return_values.rate_limit = info.fch_rate_limit;

    return_values.attributes = info.fch_attributes;

    return_values.status = SCMI_SUCCESS;

    return pcapping_protocol_ctx.scmi_api->respond(
        cap_service_id, &return_values, sizeof(return_values));
}
#endif

/*
 * SCMI module -> SCMI power capping module interface
 */
static int scmi_power_capping_get_scmi_protocol_id(
    fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{
    fwk_assert(scmi_protocol_id != NULL);
    *scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_POWER_CAPPING;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
static int scmi_power_capping_permissions_handler(
    unsigned int message_id,
    fwk_id_t cap_service_id,
    const uint32_t *payload)
{
    unsigned int agent_id;
    enum mod_res_perms_permissions perms;
    int status;

    status =
        pcapping_protocol_ctx.scmi_api->get_agent_id(cap_service_id, &agent_id);

    if (status != FWK_SUCCESS) {
        return FWK_E_ACCESS;
    }

    if (message_id < MOD_SCMI_POWER_CAPPING_DOMAIN_ATTRIBUTES) {
        perms =
            pcapping_protocol_ctx.res_perms_api->agent_has_protocol_permission(
                agent_id, MOD_SCMI_PROTOCOL_ID_POWER_CAPPING);
    } else {
        perms =
            pcapping_protocol_ctx.res_perms_api->agent_has_resource_permission(
                agent_id,
                MOD_SCMI_PROTOCOL_ID_POWER_CAPPING,
                message_id,
                *payload);
    }

    if (perms == MOD_RES_PERMS_ACCESS_DENIED) {
        status = FWK_E_ACCESS;
    }

    return status;
}
#endif

static int scmi_power_capping_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    uint32_t domain_id;
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    int status;
#endif

    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return scmi_power_capping_respond_error(service_id, SCMI_NOT_FOUND);
    }

    if (message_id >= MOD_SCMI_POWER_CAPPING_DOMAIN_ATTRIBUTES) {
        domain_id = *payload;
        if (domain_id >= pcapping_protocol_ctx.power_capping_domain_count) {
            return scmi_power_capping_respond_error(service_id, SCMI_NOT_FOUND);
        }
    }

    if (handler_table[message_id] == NULL) {
        /* Message is not supported */
        return scmi_power_capping_respond_error(service_id, SCMI_NOT_SUPPORTED);
    }

    if (payload_size != payload_size_table[message_id]) {
        /* Incorrect payload size */
        return scmi_power_capping_respond_error(
            service_id, SCMI_PROTOCOL_ERROR);
    }

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    status =
        scmi_power_capping_permissions_handler(message_id, service_id, payload);
    if (status != FWK_SUCCESS) {
        return scmi_power_capping_respond_error(service_id, SCMI_DENIED);
    }
#endif

    return handler_table[message_id](service_id, payload);
}

static struct mod_scmi_to_protocol_api
    scmi_power_capping_mod_scmi_to_protocol_api = {
        .get_scmi_protocol_id = scmi_power_capping_get_scmi_protocol_id,
        .message_handler = scmi_power_capping_message_handler
    };

/*
 * Framework interface.
 */

void pcapping_protocol_init(struct mod_scmi_power_capping_context *ctx)
{
    pcapping_protocol_ctx.power_capping_domain_count = ctx->domain_count;

    pcapping_protocol_ctx.power_capping_domain_ctx_table =
        ctx->power_capping_domain_ctx_table;
}

int pcapping_protocol_domain_init(
    uint32_t domain_idx,
    const struct mod_scmi_power_capping_domain_config *config)
{
    struct mod_scmi_power_capping_domain_context *domain_ctx;
    if (domain_idx >= pcapping_protocol_ctx.power_capping_domain_count) {
        return FWK_E_PARAM;
    }

    domain_ctx = get_domain_ctx(domain_idx);
    domain_ctx->config = config;
    domain_ctx->cap_pending_service_id = FWK_ID_NONE;
    domain_ctx->cap_notification_service_id = FWK_ID_NONE;
    domain_ctx->pai_notification_service_id = FWK_ID_NONE;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static int scmi_init_notifications(int domains)
{
    int status;
    unsigned int agent_count;

    status = pcapping_protocol_ctx.scmi_api->get_agent_count(&agent_count);
    if (status != FWK_SUCCESS) {
        return status;
    }

    fwk_assert(agent_count != 0u);

    status =
        pcapping_protocol_ctx.scmi_notification_api->scmi_notification_init(
            MOD_SCMI_PROTOCOL_ID_POWER_CAPPING,
            agent_count,
            domains,
            MOD_SCMI_POWER_CAPPING_NOTIFICATION_COUNT);

    return status;
}
#endif

int pcapping_protocol_bind(void)
{
    int status;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &(pcapping_protocol_ctx.scmi_api));

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &(pcapping_protocol_ctx.res_perms_api));
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_NOTIFICATION),
        &(pcapping_protocol_ctx.scmi_notification_api));
#endif
    return status;
}

int pcapping_protocol_start(fwk_id_t id)
{
    int status;

    struct mod_scmi_power_capping_domain_context *domain_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    domain_ctx = get_domain_ctx(fwk_id_get_element_idx(id));

    status = pcapping_protocol_check_domain_configuration(domain_ctx->config);

    if (status != FWK_SUCCESS) {
        return status;
    }

    pcapping_protocol_set_cap_config_support(domain_ctx, domain_ctx->config);

    status = fwk_notification_subscribe(
        pcapping_protocol_cap_notification,
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_ALLOCATOR),
        id);

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_notification_subscribe(
        pcapping_protocol_pai_notification,
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_COORDINATOR),
        id);

    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_notification_subscribe(
        pcapping_protocol_power_measurements_notification,
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_METER),
        id);

    if (status != FWK_SUCCESS) {
        return status;
    }
    status = scmi_init_notifications(
        pcapping_protocol_ctx.power_capping_domain_count);
#endif

    return status;
}

int pcapping_protocol_process_fwk_notification(
    const struct fwk_event *fwk_notification_event)
{
    struct mod_scmi_power_capping_domain_context *domain_ctx;

    unsigned int domain_idx =
        fwk_id_get_element_idx(fwk_notification_event->target_id);

    domain_ctx = get_domain_ctx(domain_idx);

    if (fwk_id_is_equal(
            fwk_notification_event->id, pcapping_protocol_cap_notification)) {
        return pcapping_protocol_process_cap_fwk_notification(
            domain_idx, domain_ctx);
    }
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    if (fwk_id_is_equal(
            fwk_notification_event->id, pcapping_protocol_pai_notification)) {
        return pcapping_protocol_process_pai_fwk_notification(
            domain_idx, domain_ctx);
    }
    if (fwk_id_is_equal(
            fwk_notification_event->id,
            pcapping_protocol_power_measurements_notification)) {
        return pcapping_protocol_process_power_measurements_fwk_notification(
            domain_idx, domain_ctx);
    }
#endif

    return FWK_E_PARAM;
}

int pcapping_protocol_process_bind_request(fwk_id_t api_id, const void **api)
{
    if (fwk_id_is_equal(
            api_id,
            FWK_ID_API(
                FWK_MODULE_IDX_SCMI_POWER_CAPPING,
                MOD_SCMI_POWER_CAPPING_API_IDX_REQUEST))) {
        *api = &scmi_power_capping_mod_scmi_to_protocol_api;
        return FWK_SUCCESS;
    }

    return FWK_E_SUPPORT;
}

void pcapping_protocol_set_power_apis(
    struct mod_scmi_power_capping_power_apis *power_management_apis)
{
    pcapping_protocol_ctx.power_management_apis = power_management_apis;
}

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
int pcapping_protocol_process_cap_pai_notify_event(
    const struct fwk_event *event)
{
    int status;
    unsigned int agent_id;

    struct scmi_power_capping_cap_changed_p2a payload;
    struct mod_scmi_power_capping_domain_context *ctx;

    struct pcapping_protocol_event_parameters *event_params =
        (struct pcapping_protocol_event_parameters *)event->params;

    if (fwk_id_is_equal(event_params->service_id, FWK_ID_NONE)) {
        agent_id = SCMI_POWER_CAPPING_AGENT_ID_PLATFORM;
    } else {
        status = pcapping_protocol_ctx.scmi_api->get_agent_id(
            event_params->service_id, &agent_id);

        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    payload.agent_id = agent_id;
    payload.domain_id = event_params->domain_idx;
    ctx = get_domain_ctx(event_params->domain_idx);

    status =
        pcapping_protocol_ctx.power_management_apis->power_allocator_api
            ->get_cap(ctx->config->power_allocator_domain_id, &(payload.cap));

    if (status != FWK_SUCCESS) {
        return status;
    }

    status = pcapping_protocol_ctx.power_management_apis->power_coordinator_api
                 ->get_coordinator_period(
                     ctx->config->power_coordinator_domain_id, &(payload.pai));

    if (status != FWK_SUCCESS) {
        return status;
    }

    return pcapping_protocol_ctx.scmi_notification_api
        ->scmi_notification_notify(
            MOD_SCMI_PROTOCOL_ID_POWER_CAPPING,
            MOD_SCMI_POWER_CAPPING_CAP_NOTIFY,
            SCMI_POWER_CAPPING_CAP_CHANGED,
            &payload,
            sizeof(struct scmi_power_capping_cap_changed_p2a));
}

int pcapping_protocol_process_measurements_notify_event(
    const struct fwk_event *event)
{
    int status;

    struct scmi_power_capping_measurements_changed_p2a payload;
    struct mod_scmi_power_capping_domain_context *ctx;

    struct pcapping_protocol_event_parameters *event_params =
        (struct pcapping_protocol_event_parameters *)event->params;

    payload.agent_id = SCMI_POWER_CAPPING_AGENT_ID_PLATFORM;
    payload.domain_id = event_params->domain_idx;

    ctx = get_domain_ctx(event_params->domain_idx);

    status =
        pcapping_protocol_ctx.power_management_apis->power_meter_api->get_power(
            ctx->config->power_meter_domain_id, &(payload.power));

    if (status != FWK_SUCCESS) {
        return status;
    }

    return pcapping_protocol_ctx.scmi_notification_api
        ->scmi_notification_notify(
            MOD_SCMI_PROTOCOL_ID_POWER_CAPPING,
            MOD_SCMI_POWER_CAPPING_MEASUREMENTS_NOTIFY,
            SCMI_POWER_CAPPING_MEASUREMENTS_CHANGED,
            &payload,
            sizeof(struct scmi_power_capping_measurements_changed_p2a));
}
#endif
