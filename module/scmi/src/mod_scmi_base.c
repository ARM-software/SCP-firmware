/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Base Protocol support.
 */

#include <internal/mod_scmi_base.h>
#include <internal/scmi.h>

#include <mod_scmi_std.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_string.h>

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
#    include <mod_resource_perms.h>
#endif

static struct mod_scmi_ctx *shared_scmi_ctx;
static const struct mod_scmi_from_protocol_api *protocol_api;

static const char *const default_agent_names[SCMI_AGENT_TYPE_COUNT] = {
    [SCMI_AGENT_TYPE_PSCI] = "PSCI",
    [SCMI_AGENT_TYPE_MANAGEMENT] = "MANAGEMENT",
    [SCMI_AGENT_TYPE_OSPM] = "OSPM",
    [SCMI_AGENT_TYPE_OTHER] = "OTHER",
};

static const unsigned int
    base_payload_size_table[MOD_SCMI_BASE_COMMAND_COUNT] = {
        [MOD_SCMI_PROTOCOL_VERSION] = 0,
        [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
        [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
            (unsigned int)sizeof(struct scmi_protocol_message_attributes_a2p),
        [MOD_SCMI_BASE_DISCOVER_VENDOR] = 0,
        [MOD_SCMI_BASE_DISCOVER_SUB_VENDOR] = 0,
        [MOD_SCMI_BASE_DISCOVER_IMPLEMENTATION_VERSION] = 0,
        [MOD_SCMI_BASE_DISCOVER_LIST_PROTOCOLS] =
            (unsigned int)sizeof(struct scmi_base_discover_list_protocols_a2p),
        [MOD_SCMI_BASE_DISCOVER_AGENT] =
            (unsigned int)sizeof(struct scmi_base_discover_agent_a2p),
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
        [MOD_SCMI_BASE_SET_DEVICE_PERMISSIONS] =
            sizeof(struct scmi_base_set_device_permissions_a2p),
        [MOD_SCMI_BASE_SET_PROTOCOL_PERMISSIONS] =
            sizeof(struct scmi_base_set_protocol_permissions_a2p),
        [MOD_SCMI_BASE_RESET_AGENT_CONFIG] =
            sizeof(struct scmi_base_reset_agent_config_a2p),
#endif
    };

static int scmi_base_protocol_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_base_protocol_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_base_protocol_message_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_base_discover_vendor_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_base_discover_sub_vendor_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_base_discover_implementation_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_base_discover_list_protocols_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_base_discover_agent_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
static int scmi_base_set_device_permissions(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_base_set_protocol_permissions(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_base_reset_agent_config(
    fwk_id_t service_id,
    const uint32_t *payload);
#endif

static int (*const base_handler_table[MOD_SCMI_BASE_COMMAND_COUNT])(
    fwk_id_t,
    const uint32_t *) = {
    [MOD_SCMI_PROTOCOL_VERSION] = scmi_base_protocol_version_handler,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = scmi_base_protocol_attributes_handler,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_base_protocol_message_attributes_handler,
    [MOD_SCMI_BASE_DISCOVER_VENDOR] = scmi_base_discover_vendor_handler,
    [MOD_SCMI_BASE_DISCOVER_SUB_VENDOR] = scmi_base_discover_sub_vendor_handler,
    [MOD_SCMI_BASE_DISCOVER_IMPLEMENTATION_VERSION] =
        scmi_base_discover_implementation_version_handler,
    [MOD_SCMI_BASE_DISCOVER_LIST_PROTOCOLS] =
        scmi_base_discover_list_protocols_handler,
    [MOD_SCMI_BASE_DISCOVER_AGENT] = scmi_base_discover_agent_handler,
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    [MOD_SCMI_BASE_SET_DEVICE_PERMISSIONS] = scmi_base_set_device_permissions,
    [MOD_SCMI_BASE_SET_PROTOCOL_PERMISSIONS] =
        scmi_base_set_protocol_permissions,
    [MOD_SCMI_BASE_RESET_AGENT_CONFIG] = scmi_base_reset_agent_config,
#endif
};
/*
 * Base protocol implementation
 */
/*
 * Base Protocol - PROTOCOL_VERSION
 */
static int scmi_base_protocol_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_BASE,
    };

    return protocol_api->respond(
        service_id, &return_values, sizeof(return_values));
}

/*
 * Base Protocol - PROTOCOL_ATTRIBUTES
 */
static int scmi_base_protocol_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    size_t protocol_count;
    int status;
    unsigned int agent_id;

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    size_t global_protocol_count;
    enum mod_res_perms_permissions perms;
    uint8_t protocol_id;
    unsigned int index;
#else
    enum scmi_agent_type agent_type;
#endif

    status = protocol_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        return status;
    }

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    for (index = 0, protocol_count = 0, global_protocol_count = 0;
         (index < FWK_ARRAY_SIZE(shared_scmi_ctx->scmi_protocol_id_to_idx)) &&
         (global_protocol_count < shared_scmi_ctx->protocol_count);
         index++) {
        if ((shared_scmi_ctx->scmi_protocol_id_to_idx[index] == 0) ||
            (index == MOD_SCMI_PROTOCOL_ID_BASE)) {
            continue;
        }

        protocol_id = (uint8_t)index;

        /*
         * Check that the agent has the permission to access the protocol
         */
        perms = shared_scmi_ctx->res_perms_api->agent_has_protocol_permission(
            agent_id, protocol_id);

        if (perms == MOD_RES_PERMS_ACCESS_ALLOWED) {
            protocol_count++;
        }

        global_protocol_count++;
    }
#else
    status = protocol_api->get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /*
     * PSCI agents are only allowed access to certain protocols defined
     * for the platform.
     */
    if (agent_type == SCMI_AGENT_TYPE_PSCI) {
        fwk_assert(
            shared_scmi_ctx->protocol_count >
            shared_scmi_ctx->config->dis_protocol_count_psci);
        protocol_count = shared_scmi_ctx->protocol_count -
            shared_scmi_ctx->config->dis_protocol_count_psci;
    } else {
        protocol_count = shared_scmi_ctx->protocol_count;
    }
#endif

    struct scmi_protocol_attributes_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
    };

    return_values.attributes = (uint32_t)SCMI_BASE_PROTOCOL_ATTRIBUTES(
        protocol_count, shared_scmi_ctx->config->agent_count);

    return protocol_api->respond(
        service_id, &return_values, sizeof(return_values));
}

/*
 * Base Protocol - PROTOCOL_MESSAGE_ATTRIBUTES
 */
static int scmi_base_protocol_message_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_message_attributes_a2p *parameters;
    unsigned int message_id;
    struct scmi_protocol_message_attributes_p2a return_values = {
        .status = (int32_t)SCMI_NOT_FOUND,
    };

    parameters = (struct scmi_protocol_message_attributes_a2p *)payload;
    message_id = parameters->message_id;

    if ((message_id < FWK_ARRAY_SIZE(base_handler_table)) &&
        (base_handler_table[message_id] != NULL)) {
        return_values.status = (int32_t)SCMI_SUCCESS;
    }

    /* For this protocol, all commands have an attributes value of 0, which
     * has already been set by the initialization of "return_values".
     */

    return protocol_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
}

/*
 * Base Protocol - BASE_DISCOVER_VENDOR
 */
static int scmi_base_discover_vendor_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_base_discover_vendor_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
    };

    if (shared_scmi_ctx->config->vendor_identifier != NULL) {
        fwk_str_strncpy(
            return_values.vendor_identifier,
            shared_scmi_ctx->config->vendor_identifier,
            sizeof(return_values.vendor_identifier) - 1);
    }

    return protocol_api->respond(
        service_id, &return_values, sizeof(return_values));
}

/*
 * BASE_DISCOVER_SUB_VENDOR
 */
static int scmi_base_discover_sub_vendor_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_base_discover_sub_vendor_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
    };

    if (shared_scmi_ctx->config->sub_vendor_identifier != NULL) {
        fwk_str_strncpy(
            return_values.sub_vendor_identifier,
            shared_scmi_ctx->config->sub_vendor_identifier,
            sizeof(return_values.sub_vendor_identifier) - 1);
    }

    return protocol_api->respond(
        service_id, &return_values, sizeof(return_values));
}

/*
 * BASE_DISCOVER_IMPLEMENTATION_VERSION
 */
static int scmi_base_discover_implementation_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS, .version = FWK_BUILD_VERSION
    };

    return protocol_api->respond(
        service_id, &return_values, sizeof(return_values));
}

/*
 * BASE_DISCOVER_LIST_PROTOCOLS
 */
static int scmi_base_discover_list_protocols_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    int status, respond_status;
    const struct scmi_base_discover_list_protocols_a2p *parameters;
    struct scmi_base_discover_list_protocols_p2a return_values = {
        .status = (int32_t)SCMI_GENERIC_ERROR,
        .num_protocols = 0,
    };
    unsigned int skip;
    size_t max_payload_size;
    size_t payload_size;
    size_t entry_count;
    size_t protocol_count, protocol_count_max;
    size_t avail_protocol_count = 0;
    unsigned int index;
    uint8_t protocol_id;
#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    enum mod_res_perms_permissions perms;
#else
    unsigned int dis_protocol_list_psci_index;
    unsigned int protocol_count_psci;
    enum scmi_agent_type agent_type;
#endif
    unsigned int agent_id;

    status = protocol_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        goto error;
    }

#ifndef BUILD_HAS_MOD_RESOURCE_PERMS
    status = protocol_api->get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS) {
        goto error;
    }
#endif

    status = protocol_api->get_max_payload_size(service_id, &max_payload_size);
    if (status != FWK_SUCCESS) {
        goto error;
    }

    if (max_payload_size <
        (sizeof(struct scmi_base_discover_list_protocols_p2a) +
         sizeof(return_values.protocols[0]))) {
        status = FWK_E_SIZE;
        goto error;
    }

    entry_count =
        max_payload_size - sizeof(struct scmi_base_discover_list_protocols_p2a);

    parameters = (const struct scmi_base_discover_list_protocols_a2p *)payload;
    skip = parameters->skip;

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    protocol_count_max =
        (shared_scmi_ctx->protocol_count < (skip + entry_count)) ?
        shared_scmi_ctx->protocol_count :
        (skip + entry_count);
#else

    if (agent_type == SCMI_AGENT_TYPE_PSCI) {
        fwk_assert(
            shared_scmi_ctx->protocol_count >
            shared_scmi_ctx->config->dis_protocol_count_psci);

        protocol_count_psci = shared_scmi_ctx->protocol_count -
            shared_scmi_ctx->config->dis_protocol_count_psci;

        protocol_count_max = (protocol_count_psci < (skip + entry_count)) ?
            protocol_count_psci :
            (skip + entry_count);
    } else {
        protocol_count_max =
            (shared_scmi_ctx->protocol_count < (skip + entry_count)) ?
            shared_scmi_ctx->protocol_count :
            (skip + entry_count);
    }
#endif

    for (index = 0,
        protocol_count = 0,
        payload_size = sizeof(struct scmi_base_discover_list_protocols_p2a);
         (index < FWK_ARRAY_SIZE(shared_scmi_ctx->scmi_protocol_id_to_idx)) &&
         (protocol_count < protocol_count_max);
         index++) {
        if ((shared_scmi_ctx->scmi_protocol_id_to_idx[index] == 0) ||
            (index == MOD_SCMI_PROTOCOL_ID_BASE)) {
            continue;
        }

        protocol_id = (uint8_t)index;

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
        /*
         * Check that the agent has the permission to access the protocol
         */
        perms = shared_scmi_ctx->res_perms_api->agent_has_protocol_permission(
            agent_id, protocol_id);

        if (perms == MOD_RES_PERMS_ACCESS_DENIED) {
            continue;
        }
#else
        /*
         * PSCI agents are only allowed access certain protocols defined
         * for the platform.
         */
        if (agent_type == SCMI_AGENT_TYPE_PSCI) {
            /*
             * assert if a valid list of disabled protocols is supplied in case
             * the number of the disabled protocols is not zero. In case the
             * number of the disabled protocols is zero , then no list needs to
             * be supplied
             */
            fwk_assert(
                (shared_scmi_ctx->config->dis_protocol_list_psci != NULL) ||
                (shared_scmi_ctx->config->dis_protocol_count_psci == 0));

            /*
             * check if the current protocol is within the disabled protocols
             * list.
             */
            for (dis_protocol_list_psci_index = 0;
                 dis_protocol_list_psci_index <
                 shared_scmi_ctx->config->dis_protocol_count_psci;
                 dis_protocol_list_psci_index++) {
                if (protocol_id ==
                    shared_scmi_ctx->config->dis_protocol_list_psci
                        [dis_protocol_list_psci_index]) {
                    break;
                }
            }

            /*
             * don't include the protocol in case it is in the disabled list
             */
            if (dis_protocol_list_psci_index !=
                shared_scmi_ctx->config->dis_protocol_count_psci) {
                continue;
            }
        }
#endif

        protocol_count++;
        if (protocol_count <= skip) {
            continue;
        }

        status = protocol_api->write_payload(
            service_id, payload_size, &protocol_id, sizeof(protocol_id));
        if (status != FWK_SUCCESS) {
            goto error;
        }
        payload_size += sizeof(protocol_id);
        avail_protocol_count++;
    }

    if (skip > protocol_count) {
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        goto error;
    }

    return_values.status = (int32_t)SCMI_SUCCESS;
    return_values.num_protocols = (uint32_t)avail_protocol_count;

    status = protocol_api->write_payload(
        service_id, 0, &return_values, sizeof(return_values));
    if (status != FWK_SUCCESS) {
        goto error;
    }

    payload_size = FWK_ALIGN_NEXT(payload_size, sizeof(uint32_t));

    return protocol_api->respond(service_id, NULL, payload_size);

error:
    respond_status = protocol_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI_BASE] %s @%d", __func__, __LINE__);
    }

    return status;
}

/*
 * BASE_DISCOVER_AGENT
 */
static int scmi_base_discover_agent_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_base_discover_agent_a2p *parameters;
    struct scmi_base_discover_agent_p2a return_values = {
        .status = (int32_t)SCMI_NOT_FOUND,
    };
    const struct mod_scmi_agent *agent;

#if (SCMI_PROTOCOL_VERSION_BASE >= UINT32_C(0x20000))
    unsigned int agent_id;
    int status;
#endif

    parameters = (const struct scmi_base_discover_agent_a2p *)payload;

#if (SCMI_PROTOCOL_VERSION_BASE >= UINT32_C(0x20000))
    if ((parameters->agent_id > shared_scmi_ctx->config->agent_count) &&
        (parameters->agent_id != 0xFFFFFFFF)) {
        goto exit;
    }
#else
    if (parameters->agent_id > shared_scmi_ctx->config->agent_count) {
        goto exit;
    }
#endif

    return_values.status = (int32_t)SCMI_SUCCESS;

    if (parameters->agent_id == MOD_SCMI_PLATFORM_ID) {
        static const char name[] = "platform";

        static_assert(
            sizeof(return_values.name) >= sizeof(name),
            "return_values.name is not large enough to contain name");

        fwk_str_memcpy(return_values.name, name, sizeof(name));

#if (SCMI_PROTOCOL_VERSION_BASE >= UINT32_C(0x20000))
        return_values.agent_id = MOD_SCMI_PLATFORM_ID;
#endif

        goto exit;
    }

#if (SCMI_PROTOCOL_VERSION_BASE >= UINT32_C(0x20000))
    if (parameters->agent_id == 0xFFFFFFFF) {
        /*
         * An agent can discover its own agent_id and name by passing agent_id
         * of 0xFFFFFFFF. In this case, the command returns the agent_id and
         * name of the calling agent.
         */

        status = protocol_api->get_agent_id(service_id, &agent_id);
        if (status != FWK_SUCCESS) {
            return FWK_E_ACCESS;
        }

        return_values.agent_id = (uint32_t)agent_id;

        fwk_str_strncpy(
            &return_values.name[0],
            fwk_module_get_element_name(service_id),
            sizeof(return_values.name) - 1);

        goto exit;
    }
#endif

#if (SCMI_PROTOCOL_VERSION_BASE >= UINT32_C(0x20000))
    /*
     * agent_id is identical to the agent_id field passed via the calling
     * parameters.
     */
    return_values.agent_id = parameters->agent_id;
#endif

    agent = &shared_scmi_ctx->config->agent_table[parameters->agent_id];

    fwk_str_strncpy(
        return_values.name,
        (agent->name != NULL) ? agent->name : default_agent_names[agent->type],
        sizeof(return_values.name) - 1);

exit:
    return protocol_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
}

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS

/*
 * BASE_SET_DEVICE_PERMISSIONS
 */
static int scmi_base_set_device_permissions(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_base_set_device_permissions_a2p *parameters;
    struct scmi_base_set_device_permissions_p2a return_values = {
        .status = (int32_t)SCMI_NOT_FOUND,
    };
    int status = FWK_SUCCESS;
    int respond_status;

    parameters = (const struct scmi_base_set_device_permissions_a2p *)payload;

    if (parameters->agent_id > shared_scmi_ctx->config->agent_count) {
        status = FWK_E_ACCESS;
        goto exit;
    }

    if (parameters->agent_id == MOD_SCMI_PLATFORM_ID) {
        return_values.status = (int32_t)SCMI_SUCCESS;
        goto exit;
    }

    if (parameters->flags & ~(uint32_t)MOD_RES_PERMS_PERMISSIONS_MASK) {
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        status = FWK_E_PARAM;
        goto exit;
    }

    status = shared_scmi_ctx->res_perms_api->agent_set_device_permission(
        parameters->agent_id, parameters->device_id, parameters->flags);

    switch (status) {
    case FWK_SUCCESS:
        return_values.status = (int32_t)SCMI_SUCCESS;
        break;
    case FWK_E_PARAM:
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        break;
    case FWK_E_ACCESS:
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        break;
    default:
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;
        break;
    }

exit:
    respond_status = protocol_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));

    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI_BASE] %s @%d", __func__, __LINE__);
    }

    return status;
}

/*
 * BASE_SET_PROTOCOL_PERMISSIONS
 */
static int scmi_base_set_protocol_permissions(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_base_set_protocol_permissions_a2p *parameters;
    struct scmi_base_set_protocol_permissions_p2a return_values = {
        .status = (int32_t)SCMI_NOT_FOUND,
    };
    int status = FWK_SUCCESS;
    int respond_status;

    parameters = (const struct scmi_base_set_protocol_permissions_a2p *)payload;

    if (parameters->agent_id > shared_scmi_ctx->config->agent_count) {
        status = FWK_E_ACCESS;
        goto exit;
    }

    if (parameters->agent_id == MOD_SCMI_PLATFORM_ID) {
        return_values.status = (int32_t)SCMI_SUCCESS;
        goto exit;
    }

    if (parameters->flags & ~(uint32_t)MOD_RES_PERMS_PERMISSIONS_MASK) {
        status = FWK_E_PARAM;
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    if (parameters->command_id == MOD_SCMI_PROTOCOL_ID_BASE) {
        return_values.status = (int32_t)SCMI_DENIED;
        goto exit;
    }

    status =
        shared_scmi_ctx->res_perms_api->agent_set_device_protocol_permission(
            parameters->agent_id,
            parameters->device_id,
            parameters->command_id,
            parameters->flags);

    switch (status) {
    case FWK_SUCCESS:
        return_values.status = (int32_t)SCMI_SUCCESS;
        break;
    case FWK_E_PARAM:
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        break;
    case FWK_E_ACCESS:
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        break;
    default:
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;
        break;
    }

exit:
    respond_status = protocol_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));

    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI_BASE] %s @%d", __func__, __LINE__);
    }

    return status;
}

/*
 * BASE_RESET_AGENT_CONFIG
 */
static int scmi_base_reset_agent_config(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_base_reset_agent_config_a2p *parameters;
    struct scmi_base_reset_agent_config_p2a return_values = {
        .status = (int32_t)SCMI_NOT_FOUND,
    };
    int status;

    parameters = (const struct scmi_base_reset_agent_config_a2p *)payload;

    if (parameters->agent_id > shared_scmi_ctx->config->agent_count) {
        goto exit;
    }

    if (parameters->agent_id == MOD_SCMI_PLATFORM_ID) {
        return_values.status = (int32_t)SCMI_SUCCESS;
        goto exit;
    }

    if (parameters->flags & ~(uint32_t)MOD_RES_PERMS_PERMISSIONS_MASK) {
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    status = shared_scmi_ctx->res_perms_api->agent_reset_config(
        parameters->agent_id, parameters->flags);

    switch (status) {
    case FWK_SUCCESS:
        return_values.status = (int32_t)SCMI_SUCCESS;
        break;
    case FWK_E_PARAM:
        return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
        break;
    case FWK_E_ACCESS:
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        break;
    default:
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;
        break;
    }

exit:
    return protocol_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));
}

/*
 * SCMI Resource Permissions handler
 */
static int scmi_base_permissions_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    enum mod_res_perms_permissions perms;
    unsigned int agent_id;
    int status;

    status = protocol_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS) {
        return FWK_E_ACCESS;
    }

    if (message_id < 3) {
        return FWK_SUCCESS;
    }

    /*
     * Check that the agent has permissions to access the message.
     */
    perms = shared_scmi_ctx->res_perms_api->agent_has_message_permission(
        agent_id, MOD_SCMI_PROTOCOL_ID_BASE, message_id);

    if (perms == MOD_RES_PERMS_ACCESS_ALLOWED) {
        return FWK_SUCCESS;
    } else {
        return FWK_E_ACCESS;
    }
}

#endif

int scmi_base_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    int32_t return_value;

    static_assert(
        FWK_ARRAY_SIZE(base_handler_table) ==
            FWK_ARRAY_SIZE(base_payload_size_table),
        "[SCMI] Base protocol table sizes not consistent");
    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(base_handler_table)) {
        return_value = (int32_t)SCMI_NOT_FOUND;
        goto error;
    }

    if (payload_size != base_payload_size_table[message_id]) {
        return_value = (int32_t)SCMI_PROTOCOL_ERROR;
        goto error;
    }

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    if (scmi_base_permissions_handler(
            service_id, payload, payload_size, message_id) != FWK_SUCCESS) {
        return_value = (int32_t)SCMI_DENIED;
        goto error;
    }
#endif

    return base_handler_table[message_id](service_id, payload);

error:
    return protocol_api->respond(
        service_id, &return_value, sizeof(return_value));
}

void scmi_base_set_api(const struct mod_scmi_from_protocol_api *api)
{
    protocol_api = api;
}

void scmi_base_set_shared_ctx(struct mod_scmi_ctx *scmi_ctx_param)
{
    shared_scmi_ctx = scmi_ctx_param;
}
