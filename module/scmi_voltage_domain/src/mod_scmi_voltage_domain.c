/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Voltage Domain Management Protocol Support.
 */

#include <internal/scmi_voltage_domain.h>

#include <mod_scmi.h>
#include <mod_scmi_voltage_domain.h>
#include <mod_voltage_domain.h>

#include <fwk_assert.h>
#include <fwk_attributes.h>
#include <fwk_core.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <string.h>

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
#    include <mod_resource_perms.h>
#endif

struct voltd_operations {
    /*
     * Service identifier currently requesting operation from this voltage
     * domain. A 'none' value indicates that there is no pending request.
     */
    fwk_id_t service_id;
};

struct scmi_voltd_ctx {
    /*! SCMI Voltage Domain Module Configuration */
    const struct mod_scmi_voltd_config *config;

    /*!
     * Pointer to the table of agent descriptors, used to provide per-agent
     * views of voltage domains in the system.
     */
    const struct mod_scmi_voltd_agent *agent_table;

    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* Voltage domain module API */
    const struct mod_voltd_api *voltd_api;

    /* Number of domain devices */
    int voltd_devices;

    /* Pointer to a table of domain operations */
    struct voltd_operations *voltd_ops;

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    /* SCMI Resource Permissions API */
    const struct mod_res_permissions_api *res_perms_api;
#endif
};

/*
 * SCMI Voltage Domain Message Handlers
 */
static int scmi_voltd_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_voltd_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_voltd_protocol_message_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_voltd_domain_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_voltd_config_get_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_voltd_level_get_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_voltd_level_set_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_voltd_config_set_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_voltd_describe_levels_handler(fwk_id_t service_id,
    const uint32_t *payload);

/*
 * Internal variables. There is only 1 SCMI voltd intance.
 */
static struct scmi_voltd_ctx scmi_voltd_ctx;

static int (*const handler_table[])(fwk_id_t, const uint32_t *) = {
    [MOD_SCMI_PROTOCOL_VERSION] = scmi_voltd_protocol_version_handler,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = scmi_voltd_protocol_attributes_handler,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_voltd_protocol_message_attributes_handler,
    [MOD_SCMI_VOLTD_DOMAIN_ATTRIBUTES] = scmi_voltd_domain_attributes_handler,
    [MOD_SCMI_VOLTD_CONFIG_GET] = scmi_voltd_config_get_handler,
    [MOD_SCMI_VOLTD_CONFIG_SET] = scmi_voltd_config_set_handler,
    [MOD_SCMI_VOLTD_LEVEL_GET] = scmi_voltd_level_get_handler,
    [MOD_SCMI_VOLTD_LEVEL_SET] = scmi_voltd_level_set_handler,
    [MOD_SCMI_VOLTD_DESCRIBE_LEVELS] = scmi_voltd_describe_levels_handler,
};

static const unsigned int payload_size_table[] = {
    [MOD_SCMI_PROTOCOL_VERSION] = 0,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        sizeof(struct scmi_protocol_message_attributes_a2p),
    [MOD_SCMI_VOLTD_DOMAIN_ATTRIBUTES] = sizeof(struct scmi_voltd_attributes_a2p),
    [MOD_SCMI_VOLTD_CONFIG_GET] = sizeof(struct scmi_voltd_config_get_a2p),
    [MOD_SCMI_VOLTD_CONFIG_SET] = sizeof(struct scmi_voltd_config_set_a2p),
    [MOD_SCMI_VOLTD_LEVEL_GET] = sizeof(struct scmi_voltd_level_get_a2p),
    [MOD_SCMI_VOLTD_LEVEL_SET] = sizeof(struct scmi_voltd_level_set_a2p),
    [MOD_SCMI_VOLTD_DESCRIBE_LEVELS] =
        sizeof(struct scmi_voltd_describe_levels_a2p),
};

/* Get SCMI VOLD agent device from incoming SCMI service ID */
static int get_agent_entry(fwk_id_t service_id,
                           const struct mod_scmi_voltd_agent **agent)
{
    int status;
    unsigned int agent_idx;

    status = scmi_voltd_ctx.scmi_api->get_agent_id(service_id, &agent_idx);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_idx >= scmi_voltd_ctx.config->agent_count)
        return FWK_E_PARAM;

    *agent = scmi_voltd_ctx.agent_table + agent_idx;

    return FWK_SUCCESS;
}

/* Get SCMI VOLD device from incoming SCMI service ID and element ID */
static int get_device(fwk_id_t service_id, unsigned int elt_idx,
                      const struct mod_scmi_voltd_device **out_device,
                      const struct mod_scmi_voltd_agent **out_agent)
{
    int status = 0;
    const struct mod_scmi_voltd_device *device = NULL;
    const struct mod_scmi_voltd_agent *agent = NULL;

    status = get_agent_entry(service_id, &agent);
    if (status != FWK_SUCCESS)
        return status;

    if (elt_idx >= agent->domain_count)
        return FWK_E_RANGE;

    device = &agent->device_table[elt_idx];
    fwk_module_is_valid_element_id(device->element_id);

    if (out_device)
        *out_device = device;

    if (out_agent)
        *out_agent = agent;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
/*
 * SCMI Resource Permissions handler
 */
static unsigned int get_domain_id(const uint32_t *payload)
{
    /*
     * Every SCMI Voltage Domain message is formatted with the domain ID
     * as the first 32bit message element. We will use the voltd_attributes
     * message as a basic format to retrieve the voltd ID to avoid
     * unnecessary code.
     */
    const struct scmi_voltd_attributes_a2p *parameters =
        (const struct scmi_voltd_attributes_a2p *)(void *)payload;

    return parameters->domain_id;
}

static int scmi_voltd_permissions_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    enum mod_res_perms_permissions perms;
    unsigned int agent_id;
    int status;

    status = scmi_voltd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return FWK_E_ACCESS;

    if (message_id < 3)
        perms = scmi_voltd_ctx.res_perms_api->agent_has_protocol_permission(
            agent_id, MOD_SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN);
    else
        perms = scmi_voltd_ctx.res_perms_api->agent_has_resource_permission(
            agent_id, MOD_SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN, message_id,
            get_domain_id(payload));

    if (perms == MOD_RES_PERMS_ACCESS_ALLOWED)
        return FWK_SUCCESS;

    return FWK_E_ACCESS;
}
#endif

/*
 * Protocol Version
 */
static int scmi_voltd_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a outmsg = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_VOLTD,
    };

    return scmi_voltd_ctx.scmi_api->respond(
        service_id, &outmsg, sizeof(outmsg));
}

/*
 * Protocol Attributes
 */
static int scmi_voltd_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct mod_scmi_voltd_agent *agent = NULL;
    struct scmi_protocol_attributes_p2a outmsg = {
        .status = SCMI_GENERIC_ERROR,
    };
    size_t outmsg_size = sizeof(outmsg.status);

    if (get_agent_entry(service_id, &agent) != FWK_SUCCESS)
        goto exit;

    fwk_assert(
        agent->domain_count < SCMI_VOLTD_PROTOCOL_ATTRIBUTES_VOLTD_COUNT_MAX);
    outmsg.attributes = SCMI_VOLTD_PROTOCOL_ATTRIBUTES(agent->domain_count);
    outmsg.status = SCMI_SUCCESS;
    outmsg_size = sizeof(outmsg);

exit:
    return scmi_voltd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);
}

/*
 * Protocol Message Attributes
 */
static int scmi_voltd_protocol_message_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *inmsg;
    unsigned int msg_id;
    struct scmi_protocol_message_attributes_p2a outmsg = {
        .status = SCMI_NOT_FOUND,
    };
    size_t outmsg_size = sizeof(outmsg.status);

    inmsg = (void *)payload;
    msg_id = inmsg->message_id;

    if ((msg_id < FWK_ARRAY_SIZE(handler_table)) && handler_table[msg_id]) {
        outmsg.status = SCMI_SUCCESS;
        outmsg_size = sizeof(outmsg);
    }

    return scmi_voltd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);
}

/*
 * Voltage Domain Attributes
 */
static int scmi_voltd_domain_attributes_handler(fwk_id_t service_id,
                                                const uint32_t *payload)
{
    const struct mod_scmi_voltd_device *device = NULL;
    const struct scmi_voltd_attributes_a2p *inmsg = NULL;
    struct scmi_voltd_attributes_p2a outmsg = {
        .status = SCMI_NOT_FOUND,
    };
    size_t outmsg_size = sizeof(outmsg.status);
    int status = 0;

    inmsg = (const struct scmi_voltd_attributes_a2p *)(void *)payload;

    status = get_device(service_id, inmsg->domain_id, &device, NULL);
    if (status != FWK_SUCCESS)
        goto exit;

    strncpy(
        outmsg.name,
        fwk_module_get_element_name(device->element_id),
        sizeof(outmsg.name) - 1);

    outmsg.status = SCMI_SUCCESS;
    outmsg_size = sizeof(outmsg);

exit:
    return scmi_voltd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);
}

/*
 * Voltage domain configuration mode id
 */
static int scmi_voltd_config_get_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status = 0;
    const struct mod_scmi_voltd_device *device = NULL;
    const struct scmi_voltd_config_get_a2p *inmsg = NULL;
    struct scmi_voltd_config_get_p2a outmsg = {
        .status = SCMI_GENERIC_ERROR,
    };
    size_t outmsg_size = sizeof(outmsg.status);

    uint8_t mode_id = (uint8_t)MOD_VOLTD_MODE_ID_OFF;
    uint8_t mode_type = (uint8_t)MOD_VOLTD_MODE_TYPE_ARCH;
    enum mod_voltd_mode_id voltd_mode_id;
    enum mod_voltd_mode_type voltd_mode_type;

    inmsg = (const struct scmi_voltd_config_get_a2p*)(void *)payload;

    status = get_device(service_id, inmsg->domain_id, &device, NULL);
    if (status == FWK_SUCCESS) {
        status = scmi_voltd_ctx.voltd_api->get_config(
            device->element_id, &mode_type, &mode_id);
    }

    if (status == FWK_SUCCESS) {
        voltd_mode_type = (enum mod_voltd_mode_type)mode_type;
        switch (voltd_mode_type) {
        case MOD_VOLTD_MODE_TYPE_ARCH: {
            voltd_mode_id = (enum mod_voltd_mode_id)mode_id;
            switch (voltd_mode_id) {
            case MOD_VOLTD_MODE_ID_OFF:
                outmsg.config = (uint32_t)SCMI_VOLTD_MODE_ID_OFF;
                break;
            case MOD_VOLTD_MODE_ID_ON:
                outmsg.config = (uint32_t)SCMI_VOLTD_MODE_ID_ON;
                break;
            default:
                status = FWK_E_STATE;
                break;
            }
            break;
        }
        case MOD_VOLTD_MODE_TYPE_IMPL:
            outmsg.config = SCMI_VOLTD_CONFIG_MODE_TYPE_BIT | (uint32_t)mode_id;
            break;
        default:
            status = FWK_E_STATE;
            break;
        }
    }

    if (status == FWK_SUCCESS) {
        outmsg.status = SCMI_SUCCESS;
        outmsg_size = sizeof(outmsg);
    } else if (status == FWK_E_STATE) {
        outmsg.status = SCMI_GENERIC_ERROR;
    } else {
        outmsg.status = SCMI_NOT_FOUND;
    }

    return scmi_voltd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);
}

static int scmi_voltd_config_set_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status = 0;
    const struct mod_scmi_voltd_device *device = NULL;
    const struct scmi_voltd_config_set_a2p *inmsg = NULL;
    struct scmi_voltd_config_set_p2a outmsg = {
        .status = SCMI_GENERIC_ERROR,
    };
    size_t outmsg_size = sizeof(outmsg.status);
    uint8_t mode_id;
    uint8_t mode_type = (uint8_t)MOD_VOLTD_MODE_TYPE_ARCH;
    enum scmi_voltd_mode_type scmi_mode_type;
    enum scmi_voltd_mode_id scmi_mode_id = SCMI_VOLTD_MODE_ID_OFF;

    inmsg = (const struct scmi_voltd_config_set_a2p*)payload;

    status = get_device(service_id, inmsg->domain_id, &device, NULL);
    if (status == FWK_SUCCESS) {
        scmi_mode_type = (enum scmi_voltd_mode_type)(
            inmsg->config & SCMI_VOLTD_CONFIG_MODE_TYPE_MASK);
        mode_id = (uint8_t)(inmsg->config & SCMI_VOLTD_CONFIG_MODE_ID_MASK);

        switch (scmi_mode_type) {
        case SCMI_VOLTD_MODE_TYPE_ARCH: {
            mode_type = (uint8_t)MOD_VOLTD_MODE_TYPE_ARCH;
            scmi_mode_id = (enum scmi_voltd_mode_id)(mode_id);

            switch (scmi_mode_id) {
            case SCMI_VOLTD_MODE_ID_OFF:
                mode_id = (uint8_t)MOD_VOLTD_MODE_ID_OFF;
                break;
            case SCMI_VOLTD_MODE_ID_ON:
                mode_id = (uint8_t)MOD_VOLTD_MODE_ID_ON;
                break;
            default:
                status = FWK_E_PARAM;
                break;
            }
            break;
        }
        case SCMI_VOLTD_MODE_TYPE_IMPL:
            mode_type = (uint8_t)MOD_VOLTD_MODE_TYPE_IMPL;
            break;
        default:
            status = FWK_E_PARAM;
            break;
        }

        if (status == FWK_SUCCESS) {
            status = scmi_voltd_ctx.voltd_api->set_config(
                device->element_id, mode_type, mode_id);
        }

        if (status == FWK_SUCCESS) {
            outmsg.status = SCMI_SUCCESS;
        } else {
            outmsg.status = SCMI_INVALID_PARAMETERS;
        }

    } else {
        outmsg.status = SCMI_NOT_FOUND;
    }

    return scmi_voltd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);
}

/*
 * Voltage domain voltage level
 */
static int scmi_voltd_level_get_handler(fwk_id_t service_id,
                                        const uint32_t *payload)
{
    int status = 0;
    const struct mod_scmi_voltd_device *device = NULL;
    const struct scmi_voltd_level_get_a2p *inmsg = NULL;
    struct scmi_voltd_level_get_p2a outmsg = {
        .status = SCMI_GENERIC_ERROR,
    };
    size_t outmsg_size = sizeof(outmsg.status);
    int32_t level = 0;

    inmsg = (const struct scmi_voltd_level_get_a2p*)payload;

    status = get_device(service_id, inmsg->domain_id, &device, NULL);
    if (status != FWK_SUCCESS) {
        outmsg.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_voltd_ctx.voltd_api->get_level(device->element_id, &level);
    if (status != FWK_SUCCESS)
        goto exit;

    outmsg.voltage_level = level;

    outmsg.status = SCMI_SUCCESS;
    outmsg_size = sizeof(outmsg);

exit:
    return scmi_voltd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);
}

static int scmi_voltd_level_set_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status = 0;
    const struct scmi_voltd_level_set_a2p *inmsg = NULL;
    const struct mod_scmi_voltd_device *device = NULL;
    struct scmi_voltd_level_set_p2a outmsg = {
        .status = SCMI_GENERIC_ERROR,
    };
    size_t outmsg_size = sizeof(outmsg.status);

    inmsg = (const struct scmi_voltd_level_set_a2p*)payload;

    status = get_device(service_id, inmsg->domain_id, &device, NULL);
    if (status != FWK_SUCCESS) {
        outmsg.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_voltd_ctx.voltd_api->set_level(device->element_id,
                                                 inmsg->voltage_level);

    switch (status) {
    case FWK_SUCCESS:
        outmsg.status = SCMI_SUCCESS;
        break;
    case FWK_E_RANGE:
        outmsg.status = SCMI_INVALID_PARAMETERS;
        break;
    default:
        break;
    }

exit:
    return scmi_voltd_ctx.scmi_api->respond(service_id, &outmsg, outmsg_size);
}

/*
 * Describe voltage domain levels
 */
static int scmi_voltd_describe_levels_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status = 0;
    int respond_status;
    const struct mod_scmi_voltd_device *device = NULL;
    const struct scmi_voltd_describe_levels_a2p *inmsg = NULL;
    struct scmi_voltd_describe_levels_p2a outmsg = {
        .status = SCMI_GENERIC_ERROR,
    };
    const struct mod_scmi_from_protocol_api *scmi_api = scmi_voltd_ctx.scmi_api;
    const struct mod_voltd_api *voltd_api = scmi_voltd_ctx.voltd_api;
    size_t payload_size = sizeof(outmsg);
    size_t max_payload_size = 0;
    size_t max_level_items = 0;
    struct mod_voltd_info info = { };
    uint32_t level_index = 0;

    inmsg = (const struct scmi_voltd_describe_levels_a2p*)(void *)payload;
    level_index = inmsg->level_index;

    status = get_device(service_id, inmsg->domain_id, &device, NULL);
    if (status != FWK_SUCCESS) {
        outmsg.status = SCMI_NOT_FOUND;
        goto exit;
    }

    /*
     * Get the maximum payload size to determine how many voltage level
     * entries can be returned in one response.
     */
    status = scmi_api->get_max_payload_size(service_id, &max_payload_size);
    if (status != FWK_SUCCESS)
        goto exit;

    if (max_payload_size < sizeof(outmsg))
        max_level_items = 0;
    else
        max_level_items = (max_payload_size - sizeof(outmsg)) / sizeof(int32_t);

    status = voltd_api->get_info(device->element_id, &info);
    if (status != FWK_SUCCESS)
        goto exit;

    if (info.level_range.level_type == MOD_VOLTD_VOLTAGE_LEVEL_DISCRETE) {
        /* The domain has a discrete list of voltage levels */
        int32_t level_uv = 0;
        unsigned int i = 0;
        unsigned int level_count = 0;
        unsigned int remaining_levels;

        if (level_index >= info.level_range.level_count) {
            outmsg.status = SCMI_OUT_OF_RANGE;
            goto exit;
        }

        /* Can at least one entry be returned? */
        if (!max_level_items) {
            status = FWK_E_SIZE;
            goto exit;
        }

        level_count = FWK_MIN(max_level_items,
                              info.level_range.level_count - level_index);

        remaining_levels = (info.level_range.level_count - level_index) -
                           level_count;

        /* Set number of returned levels in the message payload */
        outmsg.flags = SCMI_VOLTD_LEVEL_LIST_FLAGS(level_count,
                                                   remaining_levels);

        /* Set each level entry in the payload to the related voltage level */
        for (i = 0; i < level_count; i++, payload_size += sizeof(level_uv)) {
            status = voltd_api->get_level_from_index(device->element_id,
                                                     level_index + i, &level_uv);
            if (status != FWK_SUCCESS)
                goto exit;

            status = scmi_api->write_payload(service_id, payload_size,
                                             &level_uv, sizeof(level_uv));
            if (status != FWK_SUCCESS)
                goto exit;
        }
    } else {
        /* The voltage domain has a linear level stepping */
        int32_t voltd_range[3] = { info.level_range.min_uv,
                                   info.level_range.max_uv,
                                   info.level_range.step_uv };

        /* Is the payload area large enough to return the complete triplet? */
        if (max_level_items < 3) {
            status = FWK_E_SIZE;
            goto exit;
        }

        outmsg.flags = SCMI_VOLTD_LEVEL_RANGE_FLAGS;

        status = scmi_api->write_payload(service_id, payload_size,
                                         &voltd_range, sizeof(voltd_range));
        if (status != FWK_SUCCESS)
            goto exit;

        payload_size += sizeof(voltd_range);
    }

    outmsg.status = SCMI_SUCCESS;
    status = scmi_api->write_payload(service_id, 0, &outmsg, sizeof(outmsg));
    if (status)
        outmsg.status = SCMI_GENERIC_ERROR;

exit:
    if (outmsg.status == SCMI_SUCCESS) {
        respond_status = scmi_api->respond(service_id, NULL, payload_size);
    } else {
        respond_status = scmi_api->respond(
            service_id, &outmsg.status, sizeof(&outmsg.status));
    }

    if (respond_status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-VOLT] %s @%d", __func__, __LINE__);
    }
    // Return status or return FWK_SUCCESS???
    return status;
}

/*
 * Dispatch SCMI message to SCMI voltd module interface
 */
static int scmi_voltd_get_scmi_protocol_id(fwk_id_t protocol_id,
                                           uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = MOD_SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN;

    return FWK_SUCCESS;
}

static int scmi_voltd_message_handler(fwk_id_t protocol_id, fwk_id_t service_id,
    const uint32_t *payload, size_t payload_size, unsigned int message_id)
{
    int32_t outmsg;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Inconsistent voltage domain management protocol table sizes");
    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        outmsg = SCMI_NOT_FOUND;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        outmsg = SCMI_PROTOCOL_ERROR;
        goto error;
    }

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    if (scmi_voltd_permissions_handler(service_id, payload, payload_size,
                                       message_id) != FWK_SUCCESS) {
        outmsg = SCMI_DENIED;
        goto error;
    }
#endif

    return handler_table[message_id](service_id, payload);

error:
    return scmi_voltd_ctx.scmi_api->respond(
        service_id, &outmsg, sizeof(outmsg));
}

static struct mod_scmi_to_protocol_api scmi_voltd_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_voltd_get_scmi_protocol_id,
    .message_handler = scmi_voltd_message_handler
};

/*
 * Framework handlers
 */
static int scmi_voltd_init(fwk_id_t module_id, unsigned int element_count,
                           const void *data)
{
    int voltd_devices;
    const struct mod_scmi_voltd_config *config =
        (const struct mod_scmi_voltd_config *)data;

    if ((config == NULL) || (config->agent_table == NULL))
        return FWK_E_PARAM;

    scmi_voltd_ctx.config = config;
    scmi_voltd_ctx.agent_table = config->agent_table;

    voltd_devices = fwk_module_get_element_count(fwk_module_id_voltage_domain);
    if (voltd_devices == FWK_E_PARAM)
        return FWK_E_PANIC;

    scmi_voltd_ctx.voltd_devices = voltd_devices;
    scmi_voltd_ctx.voltd_ops = fwk_mm_calloc((unsigned int)voltd_devices,
                                             sizeof(struct voltd_operations));

    return FWK_SUCCESS;
}

static int scmi_voltd_bind(fwk_id_t id, unsigned int round)
{
    int status = 0;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
                             FWK_ID_API(FWK_MODULE_IDX_SCMI,
                                        MOD_SCMI_API_IDX_PROTOCOL),
                             &scmi_voltd_ctx.scmi_api);
    if (status != FWK_SUCCESS)
        return status;

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
                             FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS,
                                        MOD_RES_PERM_RESOURCE_PERMS),
                             &scmi_voltd_ctx.res_perms_api);
    if (status != FWK_SUCCESS)
        return status;
#endif

    return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_VOLTAGE_DOMAIN),
                           FWK_ID_API(FWK_MODULE_IDX_VOLTAGE_DOMAIN, 0),
                           &scmi_voltd_ctx.voltd_api);
}

static int scmi_voltd_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    *api = &scmi_voltd_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

/* SCMI Voltage Domain Management Protocol Definition */
const struct fwk_module module_scmi_voltage_domain = {
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_voltd_init,
    .bind = scmi_voltd_bind,
    .process_bind_request = scmi_voltd_process_bind_request,
};
