/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Control and Management Interface (SCMI) support
 *      resource management support.
 */

#include <mod_resource_perms.h>
#include <mod_scmi_std.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <string.h>

struct res_perms_ctx {
    /*! platform config data */
    struct mod_res_resource_perms_config *config;

    /*! Number of agents for the platform. */
    uint32_t agent_count;

    /*! Number of protocols for the platform. */
    uint32_t protocol_count;

    /*! Number of clock resources for the platform. */
    uint32_t clock_count;

    /*! Number of sensor resources for the platform. */
    uint32_t sensor_count;

    /*! Number of power domain resources for the platform. */
    uint32_t pd_count;

    /*! Number of performance domain resources for the platform. */
    uint32_t perf_count;

    /*! Number of devices for the platform. */
    uint32_t device_count;

#ifdef BUILD_HAS_SCMI_RESET
    /*! Number of reset domain resources for the platform. */
    uint32_t reset_domain_count;
#endif

    /*!
     * The  default is for an agent to have permission to access any
     * resource. In order to deny permissions to an agent the table
     * must be created and the relevant bit SET. If the table is not
     * created then all agents have access to all resources.
     *
     * The agent_permissions table can be created in the SCMI config
     * file.
     *
     * If the agent_permissions table was not set up in the config,
     * the agent will not be able to allocate permissions at run-time.
     *
     * Note that if the agent wants to set permissions at run-time
     * the agent_permissions[] tables must be allocated in writable
     * memory.
     */
    struct mod_res_agent_permission *agent_permissions;

    /*!
     * The list of domain devices in the system. If this is not set then setting
     * device permissions for an agent is not supported.
     */
    struct mod_res_device *domain_devices;
};

struct res_perms_backup {
    /*! \brief Power Domain:Resource permissions. */
    mod_res_perms_t *scmi_pd_perms;

    /*! Performance:Resource permissions. */
    mod_res_perms_t *scmi_perf_perms;

    /*! \brief Clock:Resource permissions. */
    mod_res_perms_t *scmi_clock_perms;

    /*! \brief Sensor:Resource permissions. */
    mod_res_perms_t *scmi_sensor_perms;

#ifdef BUILD_HAS_SCMI_RESET
    /*! \brief Reset Domain:Resource permissions. */
    mod_res_perms_t *scmi_reset_domain_perms;
#endif
};

static struct res_perms_ctx res_perms_ctx;
static struct res_perms_backup res_perms_backup;

/*
 * Map the agent-id to the corresponding index in the table.
 *
 * Returns FWK_SUCCESS if the agent_id is valid for the platform.
 * Returns FWK_E_PARAM if the agent_is id not valid for the platform.
 *
 * This is a weak definition to allow the platform override the function
 * to deal with platform-specific agent mappings, eg, for non-sequential
 * agent IDs or for sharing a single set of permissions between multiple
 * agents, etc.
 */
__attribute((weak)) int mod_res_agent_id_to_index(
    uint32_t agent_id,
    uint32_t *agent_idx)
{
    uint32_t idx = agent_id - 1;

    if (idx < res_perms_ctx.agent_count) {
        *agent_idx = idx;
        return FWK_SUCCESS;
    }

    return FWK_E_PARAM;
}

/*
 * Map the protocol-id to the corresponding index in the table.
 *
 * Returns true if the protocol_id is valid for the platform.
 * Returns false if the protocol_is id not valid for the platform.
 *
 * This is a weak definition to allow the platform override the function
 * to deal with platform-specific protocol mappings, eg, for non-contiguous
 * protocol IDs.
 */
__attribute((weak)) int mod_res_plat_protocol_id_to_index(
    uint32_t protocol_id,
    uint32_t *protocol_idx)
{
    return FWK_E_PARAM;
}

static int mod_res_protocol_id_to_index(
    uint32_t protocol_id,
    uint32_t *protocol_idx)
{
    if ((protocol_id >= MOD_SCMI_PLATFORM_PROTOCOL_ID_MIN) &&
        (protocol_id <= MOD_SCMI_PLATFORM_PROTOCOL_ID_MAX))
        return mod_res_plat_protocol_id_to_index(protocol_id, protocol_idx);

    switch (protocol_id) {
    case MOD_SCMI_PROTOCOL_ID_BASE:
    case MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN:
    case MOD_SCMI_PROTOCOL_ID_SYS_POWER:
    case MOD_SCMI_PROTOCOL_ID_PERF:
    case MOD_SCMI_PROTOCOL_ID_CLOCK:
    case MOD_SCMI_PROTOCOL_ID_SENSOR:
    case MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN:
        *protocol_idx = protocol_id - MOD_SCMI_PROTOCOL_ID_BASE;
        return FWK_SUCCESS;
    default:
        return FWK_E_PARAM;
    }
}

/*
 * Map the message-id to the corresponding index in the resources
 * table.
 *
 * Returns FWK_SUCCESS if the message_id is valid for the protocol.
 * Returns FWK_E_PARAM if the message_id not valid for the protocol.
 *
 * Contents of idx are only valid if >= 0.
 *
 * This is a weak definition to allow the platform override the function
 * to deal with platform-specific message ID mappings, eg, for non-contiguous
 * message IDs.
 */
__attribute((weak)) int mod_res_plat_message_id_to_index(
    uint32_t protocol_id,
    uint32_t message_id,
    int32_t *message_idx)
{
    return FWK_E_PARAM;
}

static int mod_res_message_id_to_index(
    uint32_t protocol_id,
    uint32_t message_id,
    int32_t *message_idx)
{
    /*
     * All protocols have
     *
     *  0: VERSION
     *  1: ATTRIBUTES
     *  2: MSG_ATTRIBUTES
     *
     * messages which are available to all agents.
     *
     * If the message is not managed in the permissions
     * we return false and idx == -1.
     */
    *message_idx = -1;

    if ((protocol_id >= MOD_SCMI_PLATFORM_PROTOCOL_ID_MIN) &&
        (protocol_id <= MOD_SCMI_PLATFORM_PROTOCOL_ID_MAX))
        return mod_res_message_id_to_index(
            protocol_id, message_id, message_idx);

    if (message_id < 3)
        return FWK_SUCCESS;

    switch (protocol_id) {
    case MOD_SCMI_PROTOCOL_ID_BASE:
        if (message_id >= MOD_SCMI_BASE_COMMAND_COUNT)
            return FWK_E_PARAM;
        if (message_id == MOD_SCMI_BASE_NOTIFY_ERRORS)
            *message_idx = 0;
        return FWK_SUCCESS;

    case MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN:
        if (message_id >= MOD_SCMI_PD_POWER_COMMAND_COUNT)
            return FWK_E_PARAM;
        if ((message_id >= MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES) &&
            (message_id <= MOD_SCMI_PD_POWER_STATE_NOTIFY))
            *message_idx = message_id - MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES;
        return FWK_SUCCESS;

    case MOD_SCMI_PROTOCOL_ID_SYS_POWER:
        if (message_id >= MOD_SCMI_SYS_POWER_COMMAND_COUNT)
            return FWK_E_PARAM;
        if ((message_id >= MOD_SCMI_SYS_POWER_STATE_SET) &&
            (message_id <= MOD_SCMI_SYS_POWER_STATE_NOTIFY))
            *message_idx = message_id - MOD_SCMI_SYS_POWER_STATE_SET;
        return FWK_SUCCESS;

    case MOD_SCMI_PROTOCOL_ID_PERF:
        if (message_id >= MOD_SCMI_PERF_COMMAND_COUNT)
            return FWK_E_PARAM;
        if ((message_id >= MOD_SCMI_PERF_DOMAIN_ATTRIBUTES) &&
            (message_id <= MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL))
            *message_idx = message_id - MOD_SCMI_PERF_DOMAIN_ATTRIBUTES;
        return FWK_SUCCESS;

    case MOD_SCMI_PROTOCOL_ID_CLOCK:
        if (message_id >= MOD_SCMI_CLOCK_COMMAND_COUNT)
            return FWK_E_PARAM;
        if ((message_id >= MOD_SCMI_CLOCK_ATTRIBUTES) &&
            (message_id <= MOD_SCMI_CLOCK_CONFIG_SET))
            *message_idx = message_id - MOD_SCMI_CLOCK_ATTRIBUTES;
        return FWK_SUCCESS;

    case MOD_SCMI_PROTOCOL_ID_SENSOR:
        if (message_id >= MOD_SCMI_SENSOR_COMMAND_COUNT)
            return FWK_E_PARAM;
        if ((message_id >= MOD_SCMI_SENSOR_DESCRIPTION_GET) &&
            (message_id <= MOD_SCMI_SENSOR_READING_GET))
            *message_idx = message_id - MOD_SCMI_SENSOR_DESCRIPTION_GET;
        return FWK_SUCCESS;

    case MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN:
        if (message_id >= MOD_SCMI_RESET_COMMAND_COUNT)
            return FWK_E_PARAM;
        if ((message_id >= MOD_SCMI_RESET_DOMAIN_ATTRIBUTES) &&
            (message_id <= MOD_SCMI_RESET_NOTIFY))
            *message_idx = message_id - MOD_SCMI_RESET_DOMAIN_ATTRIBUTES;
        return FWK_SUCCESS;

    default:
        return FWK_E_PARAM;
    }
}

/*
 * Map the resource-id to the corresponding element index in the resources
 * table.
 *
 * Returns FWK_SUCCESS if the resource_id is valid for the protocol.
 * Returns FWK_E_PARAM if the resource_id not valid for the protocol.
 *
 * Contents of resource_idx are only valid if >= 0.
 *
 * This is a weak definition to allow the platform override the function
 * to deal with platform-specific ID mappings, eg, for non-contiguous
 * IDs.
 */
__attribute((weak)) int mod_res_plat_resource_id_to_index(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id,
    uint32_t resource_id,
    int32_t *resource_idx)
{
    return FWK_E_PARAM;
}

static int mod_res_resource_id_to_index(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id,
    uint32_t resource_id,
    int32_t *resource_idx)
{
    uint32_t agent_idx;
    int message_count;
    int message_offset;
    int resource_size;
    int status;

    /*
     * All protocols have
     *
     *  0: VERSION
     *  1: ATTRIBUTES
     *  2: MSG_ATTRIBUTES
     *
     * messages which are available to all agents.
     *
     * If the message is not managed in the permissions
     * we return false and idx == -1.
     */
    *resource_idx = -1;

    if ((protocol_id >= MOD_SCMI_PLATFORM_PROTOCOL_ID_MIN) &&
        (protocol_id <= MOD_SCMI_PLATFORM_PROTOCOL_ID_MAX))
        return mod_res_plat_resource_id_to_index(
            agent_id, protocol_id, message_id, resource_id, resource_idx);

    if (message_id < 3)
        return FWK_SUCCESS;

    /*
     * Each table is organised as
     *
     *      agent[number_of_agents].commands[number_of_commands_for_protocol]
     */

    switch (protocol_id) {
    case MOD_SCMI_PROTOCOL_ID_BASE:
        /* No per-resource managemment for SCMI Base Protocol */
        return FWK_E_PARAM;

    case MOD_SCMI_PROTOCOL_ID_SYS_POWER:
        /* No per-resource managemment for SCMI System Power Protocol */
        return FWK_E_PARAM;

    case MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN:
        if ((message_id >= MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES) &&
            (message_id <= MOD_SCMI_PD_POWER_STATE_NOTIFY)) {
            message_count = MOD_SCMI_PD_POWER_STATE_NOTIFY -
                MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES + 1;
            message_offset = message_id - MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES;
            resource_size = res_perms_ctx.pd_count;
            break;
        }
        return FWK_E_PARAM;

    case MOD_SCMI_PROTOCOL_ID_PERF:
        if ((message_id >= MOD_SCMI_PERF_DOMAIN_ATTRIBUTES) &&
            (message_id <= MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL)) {
            message_count = MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL -
                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES + 1;
            message_offset = message_id - MOD_SCMI_PERF_DOMAIN_ATTRIBUTES;
            resource_size = res_perms_ctx.perf_count;
            break;
        }
        return FWK_E_PARAM;

    case MOD_SCMI_PROTOCOL_ID_CLOCK:
        if ((message_id >= MOD_SCMI_CLOCK_ATTRIBUTES) &&
            (message_id <= MOD_SCMI_CLOCK_CONFIG_SET)) {
            message_count =
                MOD_SCMI_CLOCK_CONFIG_SET - MOD_SCMI_CLOCK_ATTRIBUTES + 1;
            message_offset = message_id - MOD_SCMI_CLOCK_ATTRIBUTES;
            resource_size = res_perms_ctx.clock_count;
            break;
        }
        return FWK_E_PARAM;

    case MOD_SCMI_PROTOCOL_ID_SENSOR:
        if ((message_id >= MOD_SCMI_SENSOR_DESCRIPTION_GET) &&
            (message_id <= MOD_SCMI_SENSOR_READING_GET)) {
            message_count = MOD_SCMI_SENSOR_READING_GET -
                MOD_SCMI_SENSOR_DESCRIPTION_GET + 1;
            message_offset = message_id - MOD_SCMI_SENSOR_DESCRIPTION_GET;
            resource_size = res_perms_ctx.sensor_count;
            break;
        }
        return FWK_E_PARAM;

#ifdef BUILD_HAS_SCMI_RESET
    case MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN:
        if ((message_id >= MOD_SCMI_RESET_DOMAIN_ATTRIBUTES) &&
            (message_id <= MOD_SCMI_RESET_NOTIFY)) {
            message_count =
                MOD_SCMI_RESET_NOTIFY - MOD_SCMI_RESET_DOMAIN_ATTRIBUTES + 1;
            message_offset = message_id - MOD_SCMI_RESET_DOMAIN_ATTRIBUTES;
            resource_size = res_perms_ctx.reset_domain_count;
            break;
        }
        return FWK_E_PARAM;
#endif

    default:
        return FWK_E_PARAM;
    }

    status = mod_res_agent_id_to_index(agent_id, &agent_idx);
    if ((status != FWK_SUCCESS) || (agent_idx >= res_perms_ctx.agent_count))
        return FWK_E_PARAM;

    resource_size = MOD_RES_PERMS_RESOURCE_ELEMENT(resource_size) + 1;

    /*
     * message_count: the number of messages for the agent.
     * resource_size: the number of resource elements per message type.
     * message_offset: the offset of this message from message 0.
     *
     * [1]: &agent[agent_idx].message[0]
     * [2]: &agent[agent_idx].message[message_id]
     * [3]: &agent[agent_idx].message[message_id].resource[resource_id]
     */
    *resource_idx = (agent_idx * message_count * resource_size) + /* [1] */
        (message_offset * resource_size) + /* [2] */
        MOD_RES_PERMS_RESOURCE_ELEMENT(resource_id); /* [3] */

    return FWK_SUCCESS;
}

/*
 * Check whether an agent has access to a protocol.
 *
 * Note that we will always check the higher permissions levels
 * when called, so
 *
 *      protocol -> message -> resource
 */

/*
 * Check whether an agent has access to a protocol.
 *
 * Note that we will always check the higher permissions levels
 * when called, so
 *
 *      protocol -> message -> resource
 */
__attribute((weak)) enum mod_res_perms_permissions
    mod_res_plat_agent_protocol_permissions(
        uint32_t agent_id,
        uint32_t protocol_id)
{
    return MOD_RES_PERMS_ACCESS_DENIED;
}

static enum mod_res_perms_permissions agent_protocol_permissions(
    uint32_t agent_id,
    uint32_t protocol_id)
{
    uint32_t agent_idx;
    uint32_t protocol_idx;
    mod_res_perms_t perms;
    int status;

    /* No Agent:Protocol permissions management */
    if ((agent_id == 0) || (res_perms_ctx.agent_permissions == NULL) ||
        (res_perms_ctx.agent_permissions->agent_protocol_permissions == NULL))
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    if ((protocol_id >= MOD_SCMI_PLATFORM_PROTOCOL_ID_MIN) &&
        (protocol_id <= MOD_SCMI_PLATFORM_PROTOCOL_ID_MAX))
        return mod_res_plat_agent_protocol_permissions(agent_id, protocol_id);

    status = mod_res_agent_id_to_index(agent_id, &agent_idx);
    if ((status != FWK_SUCCESS) || (agent_idx >= res_perms_ctx.agent_count))
        return MOD_RES_PERMS_ACCESS_DENIED;

    status = mod_res_protocol_id_to_index(protocol_id, &protocol_idx);
    if ((status != FWK_SUCCESS) ||
        (protocol_idx >= res_perms_ctx.protocol_count))
        return MOD_RES_PERMS_ACCESS_DENIED;

    perms =
        res_perms_ctx.agent_permissions->agent_protocol_permissions[agent_idx]
            .protocols;

    /* Agent:Protocol access denied */
    if (perms & (1 << protocol_idx))
        return MOD_RES_PERMS_ACCESS_DENIED;

    return MOD_RES_PERMS_ACCESS_ALLOWED;
}

/*
 * Check whether an agent can access a protocol:message.
 */
__attribute((weak)) enum mod_res_perms_permissions
    mod_res_plat_agent_message_permissions(
        uint32_t agent_id,
        uint32_t protocol_id,
        uint32_t message_id)
{
    return MOD_RES_PERMS_ACCESS_DENIED;
}

static enum mod_res_perms_permissions agent_message_permissions(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id)
{
    enum mod_res_perms_permissions protocol_perms;
    uint32_t agent_idx;
    uint32_t protocol_idx;
    int32_t message_idx;
    mod_res_perms_t perms;
    int status;

    /* No permissions management */
    if ((agent_id == 0) || (res_perms_ctx.agent_permissions == NULL))
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    if ((protocol_id >= MOD_SCMI_PLATFORM_PROTOCOL_ID_MIN) &&
        (protocol_id <= MOD_SCMI_PLATFORM_PROTOCOL_ID_MAX))
        return mod_res_plat_agent_message_permissions(
            agent_id, protocol_id, message_id);

    /* Agent:Protocol access denied */
    protocol_perms = agent_protocol_permissions(agent_id, protocol_id);
    if (protocol_perms == MOD_RES_PERMS_ACCESS_DENIED)
        return MOD_RES_PERMS_ACCESS_DENIED;

    /* No Agent:Protocol:command permissions management */
    if (res_perms_ctx.agent_permissions->agent_msg_permissions == NULL)
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    status = mod_res_message_id_to_index(protocol_id, message_id, &message_idx);
    if (status != FWK_SUCCESS)
        return MOD_RES_PERMS_ACCESS_DENIED;

    if (message_idx < 0)
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    status = mod_res_agent_id_to_index(agent_id, &agent_idx);
    if (status != FWK_SUCCESS)
        return MOD_RES_PERMS_ACCESS_DENIED;

    status = mod_res_protocol_id_to_index(protocol_id, &protocol_idx);
    if (status != FWK_SUCCESS)
        return MOD_RES_PERMS_ACCESS_DENIED;

    perms = res_perms_ctx.agent_permissions->agent_msg_permissions[agent_idx]
                .messages[protocol_idx];

    /* Agent:Protocol:message access denied */
    if (perms & (1 << message_idx))
        return MOD_RES_PERMS_ACCESS_DENIED;

    return MOD_RES_PERMS_ACCESS_ALLOWED;
}

/*
 * Check the permissions for agent:protocol:message:resource.
 *
 * Note that BASE and SYSTEM_POWER protocols do not support
 * resource-level permissions checking, these protocols are
 * managed at the  agent:protocol:message level.
 */
__attribute((weak)) enum mod_res_perms_permissions
    mod_res_plat_agent_resource_permissions(
        uint32_t agent_id,
        uint32_t protocol_id,
        uint32_t message_id,
        uint32_t resource_id)
{
    return MOD_RES_PERMS_ACCESS_DENIED;
}

static enum mod_res_perms_permissions agent_resource_permissions(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id,
    uint32_t resource_id)
{
    enum mod_res_perms_permissions message_perms;
    uint32_t agent_idx;
    int32_t message_idx;
    int32_t resource_idx;
    mod_res_perms_t perms;
    int status;

    /* No permissions management */
    if ((agent_id == 0) || (res_perms_ctx.agent_permissions == NULL))
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    if ((protocol_id >= MOD_SCMI_PLATFORM_PROTOCOL_ID_MIN) &&
        (protocol_id <= MOD_SCMI_PLATFORM_PROTOCOL_ID_MAX))
        return mod_res_plat_agent_resource_permissions(
            agent_id, protocol_id, message_id, resource_id);

    /* Agent:Protocol:command access denied */
    message_perms =
        agent_message_permissions(agent_id, protocol_id, message_id);
    if (message_perms == MOD_RES_PERMS_ACCESS_DENIED)
        return MOD_RES_PERMS_ACCESS_DENIED;

    status = mod_res_message_id_to_index(protocol_id, message_id, &message_idx);
    if (status != FWK_SUCCESS)
        return MOD_RES_PERMS_ACCESS_DENIED;
    if (message_idx < 0)
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    status = mod_res_agent_id_to_index(agent_id, &agent_idx);
    if (status != FWK_SUCCESS)
        return MOD_RES_PERMS_ACCESS_DENIED;

    status = mod_res_resource_id_to_index(
        agent_id, protocol_id, message_id, resource_id, &resource_idx);
    if (status != FWK_SUCCESS)
        return MOD_RES_PERMS_ACCESS_DENIED;
    if (resource_idx < 0)
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    switch (protocol_id) {
    case MOD_SCMI_PROTOCOL_ID_BASE:
        return MOD_RES_PERMS_ACCESS_DENIED;

    case MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN:
        if (res_perms_ctx.agent_permissions->scmi_pd_perms == NULL)
            return MOD_RES_PERMS_ACCESS_ALLOWED;
        if (resource_id >= res_perms_ctx.pd_count)
            return MOD_RES_PERMS_ACCESS_DENIED;
        perms = res_perms_ctx.agent_permissions->scmi_pd_perms[resource_idx];
        break;

    case MOD_SCMI_PROTOCOL_ID_SYS_POWER:
        return MOD_RES_PERMS_ACCESS_DENIED;

    case MOD_SCMI_PROTOCOL_ID_PERF:
        if (res_perms_ctx.agent_permissions->scmi_perf_perms == NULL)
            return MOD_RES_PERMS_ACCESS_ALLOWED;
        if (resource_id >= res_perms_ctx.perf_count)
            return MOD_RES_PERMS_ACCESS_DENIED;
        perms = res_perms_ctx.agent_permissions->scmi_perf_perms[resource_idx];
        break;

    case MOD_SCMI_PROTOCOL_ID_CLOCK:
        if (res_perms_ctx.agent_permissions->scmi_clock_perms == NULL)
            return MOD_RES_PERMS_ACCESS_ALLOWED;
        if (resource_id >= res_perms_ctx.clock_count)
            return MOD_RES_PERMS_ACCESS_DENIED;
        perms = res_perms_ctx.agent_permissions->scmi_clock_perms[resource_idx];
        break;

    case MOD_SCMI_PROTOCOL_ID_SENSOR:
        if (res_perms_ctx.agent_permissions->scmi_sensor_perms == NULL)
            return MOD_RES_PERMS_ACCESS_ALLOWED;
        if (resource_id >= res_perms_ctx.sensor_count)
            return MOD_RES_PERMS_ACCESS_DENIED;
        perms =
            res_perms_ctx.agent_permissions->scmi_sensor_perms[resource_idx];
        break;

#ifdef BUILD_HAS_SCMI_RESET
    case MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN:
        if (res_perms_ctx.agent_permissions->scmi_reset_domain_perms == NULL)
            return MOD_RES_PERMS_ACCESS_ALLOWED;
        if (resource_id >= res_perms_ctx.reset_domain_count)
            return MOD_RES_PERMS_ACCESS_DENIED;
        perms = res_perms_ctx.agent_permissions
                    ->scmi_reset_domain_perms[resource_idx];
        break;
#endif

    default:
        return MOD_RES_PERMS_ACCESS_DENIED;
    }

    /* Agent:Protocol:message:resource access denied */
    if ((perms & (1 << (MOD_RES_PERMS_RESOURCE_BIT(resource_id)))))
        return MOD_RES_PERMS_ACCESS_DENIED;

    return MOD_RES_PERMS_ACCESS_ALLOWED;
}

/*
 * Set the permissions for an agent:device.
 */
static int set_agent_resource_message_perms(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_idx,
    uint32_t resource_id,
    mod_res_perms_t *perms,
    enum mod_res_perms_permissions flags)
{
    int status;
    int32_t resource_idx;
    mod_res_perms_t permissions;

    status = mod_res_resource_id_to_index(
        agent_id, protocol_id, message_idx, resource_id, &resource_idx);
    if (status != FWK_SUCCESS)
        return status;

    permissions = perms[resource_idx];

    flags =
        ((flags == 0) ? MOD_RES_PERMS_ACCESS_DENIED :
                        MOD_RES_PERMS_ACCESS_ALLOWED);

    if (flags == MOD_RES_PERMS_ACCESS_ALLOWED)
        permissions &= ~(1 << (MOD_RES_PERMS_RESOURCE_BIT(resource_id)));
    else
        permissions |= (1 << (MOD_RES_PERMS_RESOURCE_BIT(resource_id)));

    perms[resource_idx] = permissions;

    return FWK_SUCCESS;
}

static int set_agent_resource_pd_permissions(
    uint32_t agent_id,
    uint32_t resource_id,
    enum mod_res_perms_permissions flags)
{
    int status;
    int32_t message_idx;

    if (res_perms_ctx.agent_permissions->scmi_pd_perms == NULL)
        return FWK_SUCCESS;
    if (resource_id >= res_perms_ctx.pd_count)
        return FWK_E_ACCESS;

    /* Do a backup before making the changes if required */
    if ((res_perms_ctx.config->pd_cmd_count != 0) &&
        (res_perms_ctx.config->pd_resource_count != 0) &&
        (res_perms_backup.scmi_pd_perms == NULL)) {
        res_perms_backup.scmi_pd_perms = (mod_res_perms_t *)fwk_mm_alloc(
            res_perms_ctx.agent_count * res_perms_ctx.config->pd_cmd_count *
                res_perms_ctx.config->pd_resource_count,
            sizeof(mod_res_perms_t));
        memcpy(
            res_perms_backup.scmi_pd_perms,
            res_perms_ctx.agent_permissions->scmi_pd_perms,
            res_perms_ctx.agent_count * res_perms_ctx.config->pd_cmd_count *
                res_perms_ctx.config->pd_resource_count *
                sizeof(mod_res_perms_t));
    }

    for (message_idx = MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES;
         message_idx <= MOD_SCMI_PD_POWER_STATE_NOTIFY;
         message_idx++) {
        status = set_agent_resource_message_perms(
            agent_id,
            MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN,
            message_idx,
            resource_id,
            res_perms_ctx.agent_permissions->scmi_pd_perms,
            flags);

        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int set_agent_resource_perf_permissions(
    uint32_t agent_id,
    uint32_t resource_id,
    enum mod_res_perms_permissions flags)
{
    int status;
    int32_t message_idx;

    if (res_perms_ctx.agent_permissions->scmi_perf_perms == NULL)
        return FWK_SUCCESS;
    if (resource_id >= res_perms_ctx.perf_count)
        return FWK_E_ACCESS;

    /* Do a backup before making the changes if required */
    if ((res_perms_ctx.config->perf_cmd_count != 0) &&
        (res_perms_ctx.config->perf_resource_count != 0) &&
        (res_perms_backup.scmi_perf_perms == NULL)) {
        res_perms_backup.scmi_perf_perms = (mod_res_perms_t *)fwk_mm_alloc(
            res_perms_ctx.agent_count * res_perms_ctx.config->perf_cmd_count *
                res_perms_ctx.config->perf_resource_count,
            sizeof(mod_res_perms_t));
        memcpy(
            res_perms_backup.scmi_perf_perms,
            res_perms_ctx.agent_permissions->scmi_perf_perms,
            res_perms_ctx.agent_count * res_perms_ctx.config->perf_cmd_count *
                res_perms_ctx.config->perf_resource_count *
                sizeof(mod_res_perms_t));
    }

    for (message_idx = MOD_SCMI_PERF_DOMAIN_ATTRIBUTES;
         message_idx <= MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL;
         message_idx++) {
        status = set_agent_resource_message_perms(
            agent_id,
            MOD_SCMI_PROTOCOL_ID_PERF,
            message_idx,
            resource_id,
            res_perms_ctx.agent_permissions->scmi_perf_perms,
            flags);

        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int set_agent_resource_clock_permissions(
    uint32_t agent_id,
    uint32_t resource_id,
    enum mod_res_perms_permissions flags)
{
    int status;
    int32_t message_idx;

    if (res_perms_ctx.agent_permissions->scmi_clock_perms == NULL)
        return FWK_SUCCESS;
    if (resource_id >= res_perms_ctx.clock_count)
        return FWK_E_ACCESS;

    /* Do a backup before making the changes if required */
    if ((res_perms_ctx.config->clock_cmd_count != 0) &&
        (res_perms_ctx.config->clock_resource_count != 0) &&
        (res_perms_backup.scmi_clock_perms == NULL)) {
        res_perms_backup.scmi_clock_perms = (mod_res_perms_t *)fwk_mm_alloc(
            res_perms_ctx.agent_count * res_perms_ctx.config->clock_cmd_count *
                res_perms_ctx.config->clock_resource_count,
            sizeof(mod_res_perms_t));
        memcpy(
            res_perms_backup.scmi_clock_perms,
            res_perms_ctx.agent_permissions->scmi_clock_perms,
            res_perms_ctx.agent_count * res_perms_ctx.config->clock_cmd_count *
                res_perms_ctx.config->clock_resource_count *
                sizeof(mod_res_perms_t));
    }

    for (message_idx = MOD_SCMI_CLOCK_ATTRIBUTES;
         message_idx <= MOD_SCMI_CLOCK_CONFIG_SET;
         message_idx++) {
        status = set_agent_resource_message_perms(
            agent_id,
            MOD_SCMI_PROTOCOL_ID_CLOCK,
            message_idx,
            resource_id,
            res_perms_ctx.agent_permissions->scmi_clock_perms,
            flags);

        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_SCMI_RESET

static int set_agent_resource_reset_permissions(
    uint32_t agent_id,
    uint32_t resource_id,
    enum mod_res_perms_permissions flags)
{
    int status;
    int32_t message_idx;

    if (res_perms_ctx.agent_permissions->scmi_reset_domain_perms == NULL)
        return FWK_SUCCESS;
    if (resource_id >= res_perms_ctx.reset_domain_count)
        return FWK_E_ACCESS;

    /* Do a backup before making the changes if required */
    if ((res_perms_ctx.config->reset_domain_cmd_count != 0) &&
        (res_perms_ctx.config->reset_domain_resource_count != 0) &&
        (res_perms_backup.scmi_reset_domain_perms == NULL)) {
        res_perms_backup.scmi_reset_domain_perms =
            (mod_res_perms_t *)fwk_mm_alloc(
                res_perms_ctx.agent_count *
                    res_perms_ctx.config->reset_domain_cmd_count *
                    res_perms_ctx.config->reset_domain_resource_count,
                sizeof(mod_res_perms_t));
        memcpy(
            res_perms_backup.scmi_reset_domain_perms,
            res_perms_ctx.agent_permissions->scmi_reset_domain_perms,
            res_perms_ctx.agent_count *
                res_perms_ctx.config->reset_domain_cmd_count *
                res_perms_ctx.config->reset_domain_cmd_count *
                sizeof(mod_res_perms_t));
    }

    for (message_idx = MOD_SCMI_RESET_DOMAIN_ATTRIBUTES;
         message_idx <= MOD_SCMI_RESET_NOTIFY;
         message_idx++) {
        status = set_agent_resource_message_perms(
            agent_id,
            MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN,
            message_idx,
            resource_id,
            res_perms_ctx.agent_permissions->scmi_reset_domain_perms,
            flags);

        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

#endif

static int set_agent_resource_sensor_permissions(
    uint32_t agent_id,
    uint32_t resource_id,
    enum mod_res_perms_permissions flags)
{
    int status;
    int32_t message_idx;

    if (res_perms_ctx.agent_permissions->scmi_sensor_perms == NULL)
        return FWK_SUCCESS;
    if (resource_id >= res_perms_ctx.sensor_count)
        return FWK_E_ACCESS;

    /* Do a backup before making the changes if required */
    if ((res_perms_ctx.config->sensor_cmd_count != 0) &&
        (res_perms_ctx.config->sensor_resource_count != 0) &&
        (res_perms_backup.scmi_sensor_perms == NULL)) {
        res_perms_backup.scmi_sensor_perms = (mod_res_perms_t *)fwk_mm_alloc(
            res_perms_ctx.agent_count * res_perms_ctx.config->sensor_cmd_count *
                res_perms_ctx.config->sensor_resource_count,
            sizeof(mod_res_perms_t));
        memcpy(
            res_perms_backup.scmi_sensor_perms,
            res_perms_ctx.agent_permissions->scmi_sensor_perms,
            res_perms_ctx.agent_count * res_perms_ctx.config->sensor_cmd_count *
                res_perms_ctx.config->sensor_resource_count *
                sizeof(mod_res_perms_t));
    }

    for (message_idx = MOD_SCMI_SENSOR_DESCRIPTION_GET;
         message_idx <= MOD_SCMI_SENSOR_READING_GET;
         message_idx++) {
        status = set_agent_resource_message_perms(
            agent_id,
            MOD_SCMI_PROTOCOL_ID_SENSOR,
            message_idx,
            resource_id,
            res_perms_ctx.agent_permissions->scmi_sensor_perms,
            flags);

        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int mod_res_agent_set_permissions(
    enum mod_res_domain_device_types type,
    uint32_t agent_id,
    uint32_t resource_id,
    enum mod_res_perms_permissions flags)
{
    int status;

    switch (type) {
    case MOD_RES_POWER_DOMAIN_DEVICE:
        status =
            set_agent_resource_pd_permissions(agent_id, resource_id, flags);
        break;

    case MOD_RES_PERF_DOMAIN_DEVICE:
        status =
            set_agent_resource_perf_permissions(agent_id, resource_id, flags);
        break;

    case MOD_RES_CLOCK_DOMAIN_DEVICE:
        status =
            set_agent_resource_clock_permissions(agent_id, resource_id, flags);
        break;

    case MOD_RES_SENSOR_DOMAIN_DEVICE:
        status =
            set_agent_resource_sensor_permissions(agent_id, resource_id, flags);
        break;

#ifdef BUILD_HAS_SCMI_RESET
    case MOD_RES_RESET_DOMAIN_DEVICE:
        status =
            set_agent_resource_reset_permissions(agent_id, resource_id, flags);
        break;
#endif

    default:
        status = FWK_E_ACCESS;
        break;
    }

    if (status != FWK_SUCCESS)
        FWK_LOG_WARN(
            "[perms] set_permissions for agent %d type %d device %d failed\n",
            (int)agent_id,
            (int)type,
            (int)resource_id);

    return status;
}

static int mod_res_agent_set_device_permission(
    uint32_t agent_id,
    uint32_t device_id,
    uint32_t flags)
{
    struct mod_res_domain_device *dev;
    uint32_t resource_id;
    int status;
    uint32_t i;

    /* No device permissons */
    if ((res_perms_ctx.device_count == 0) ||
        (res_perms_ctx.domain_devices == NULL))
        return FWK_E_ACCESS;

    if (device_id >= res_perms_ctx.device_count)
        return FWK_E_PARAM;

    if ((flags != 0) && (flags != 1))
        return FWK_E_PARAM;

    /* Find device */
    for (i = 0; i < res_perms_ctx.device_count; i++) {
        if (res_perms_ctx.domain_devices[i].device_id == device_id)
            break;
    }

    if (i == res_perms_ctx.device_count)
        return FWK_E_ACCESS;

    dev = res_perms_ctx.domain_devices[i].domain_devices;
    while (dev->type != MOD_RES_DOMAIN_DEVICE_INVALID) {
        if (fwk_id_is_equal(dev->device_id, FWK_ID_NONE))
            return FWK_SUCCESS;

        resource_id = fwk_id_get_element_idx(dev->device_id);

        status = mod_res_agent_set_permissions(
            dev->type, agent_id, resource_id, flags);

        if (status != FWK_SUCCESS)
            FWK_LOG_WARN(
                "[perms] set_permissions for agent %d device %d failed\n",
                (int)agent_id,
                (int)i);

        /* next domain device */
        dev++;
    };

    return FWK_SUCCESS;
}

/*
 * Set the permissions for an agent:device:protocol.
 */
static int mod_res_agent_set_device_protocol_permission(
    uint32_t agent_id,
    uint32_t device_id,
    uint32_t protocol_id,
    uint32_t flags)
{
    enum mod_res_domain_device_types dev_type;
    struct mod_res_domain_device *dev;
    uint32_t resource_id;
    int status;
    uint32_t i;

    /* No device permissons */
    if ((res_perms_ctx.device_count == 0) ||
        (res_perms_ctx.domain_devices == NULL))
        return FWK_E_ACCESS;

    if (device_id >= res_perms_ctx.device_count)
        return FWK_E_PARAM;

    if ((flags != 0) && (flags != 1))
        return FWK_E_PARAM;

    /* Find device */
    for (i = 0; i < res_perms_ctx.device_count; i++) {
        if (res_perms_ctx.domain_devices[i].device_id == device_id)
            break;
    }

    if (i == res_perms_ctx.device_count)
        return FWK_E_ACCESS;

    switch (protocol_id) {
    case MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN:
        dev_type = MOD_RES_POWER_DOMAIN_DEVICE;
        break;
    case MOD_SCMI_PROTOCOL_ID_PERF:
        dev_type = MOD_RES_PERF_DOMAIN_DEVICE;
        break;
    case MOD_SCMI_PROTOCOL_ID_CLOCK:
        dev_type = MOD_RES_CLOCK_DOMAIN_DEVICE;
        break;
    case MOD_SCMI_PROTOCOL_ID_SENSOR:
        dev_type = MOD_RES_SENSOR_DOMAIN_DEVICE;
        break;
    case MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN:
        dev_type = MOD_RES_RESET_DOMAIN_DEVICE;
        break;
    default:
    case MOD_SCMI_PROTOCOL_ID_BASE:
    case MOD_SCMI_PROTOCOL_ID_SYS_POWER:
        return FWK_E_ACCESS;
    }

    dev = res_perms_ctx.domain_devices[i].domain_devices;
    while (dev->type != MOD_RES_DOMAIN_DEVICE_INVALID) {
        if (fwk_id_is_equal(dev->device_id, FWK_ID_NONE))
            return FWK_SUCCESS;

        if (dev->type != dev_type) {
            dev++;
            continue;
        }

        resource_id = fwk_id_get_element_idx(dev->device_id);

        status = mod_res_agent_set_permissions(
            dev->type, agent_id, resource_id, flags);

        if (status != FWK_SUCCESS)
            FWK_LOG_WARN(
                "[perms] set_permissions for agent %d device %d failed\n",
                (int)agent_id,
                (int)i);

        /* next domain device */
        dev++;
    };

    return FWK_SUCCESS;
}

/*
 * Reset the permissions for an agent to the default configuration.
 * Note that we only save and restore the permissions for protocols
 * which have set <protocol>_cmd_count and <protocol>_resource_count
 * in the platform config data. Also, the backup copies of the
 * permissions are not cleared after they are restored, the backup may
 * be re-used for any future reset requests.
 */
static void mod_res_agent_copy_config(
    uint32_t agent_id,
    mod_res_perms_t *perms,
    mod_res_perms_t *backup_perms,
    uint32_t protocol_id)
{
    int cmd_count;
    int resource_count;
    int32_t resource_idx;
    int i, j;
    int status;

    switch (protocol_id) {
    case MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN:
        cmd_count = MOD_SCMI_PD_POWER_COMMAND_COUNT;
        resource_count = res_perms_ctx.config->pd_resource_count;
        break;
    case MOD_SCMI_PROTOCOL_ID_PERF:
        cmd_count = MOD_SCMI_PERF_COMMAND_COUNT;
        resource_count = res_perms_ctx.config->perf_resource_count;
        break;
    case MOD_SCMI_PROTOCOL_ID_CLOCK:
        cmd_count = MOD_SCMI_CLOCK_COMMAND_COUNT;
        resource_count = res_perms_ctx.config->clock_resource_count;
        break;
    case MOD_SCMI_PROTOCOL_ID_SENSOR:
        cmd_count = MOD_SCMI_SENSOR_COMMAND_COUNT;
        resource_count = res_perms_ctx.config->sensor_resource_count;
        break;
#ifdef BUILD_HAS_SCMI_RESET
    case MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN:
        cmd_count = MOD_SCMI_RESET_COMMAND_COUNT;
        resource_count = res_perms_ctx.config->reset_domain_resource_count;
        break;
#endif
    default:
        return;
    }

    for (i = 3; i < cmd_count; i++) { /* commands < 3 are excluded */
        for (j = 0; j < resource_count; j++) {
            status = mod_res_resource_id_to_index(
                agent_id,
                protocol_id,
                i, /* message id */
                j, /* resource id */
                &resource_idx);
            if ((status != FWK_SUCCESS) || (resource_idx < 0))
                continue;
            perms[resource_idx] = backup_perms[resource_idx];
        }
    }
}

static int mod_res_agent_reset_config(uint32_t agent_id, uint32_t flags)
{
    /* No device permissons */
    if ((res_perms_ctx.device_count == 0) ||
        (res_perms_ctx.domain_devices == NULL))
        return FWK_E_ACCESS;

    if (flags == 0)
        return FWK_SUCCESS;

    if (agent_id >= res_perms_ctx.agent_count)
        return FWK_E_ACCESS;

    if (res_perms_backup.scmi_pd_perms != NULL) {
        mod_res_agent_copy_config(
            agent_id,
            res_perms_ctx.agent_permissions->scmi_pd_perms,
            res_perms_backup.scmi_pd_perms,
            MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN);
    }

    if (res_perms_backup.scmi_perf_perms != NULL) {
        mod_res_agent_copy_config(
            agent_id,
            res_perms_ctx.agent_permissions->scmi_perf_perms,
            res_perms_backup.scmi_perf_perms,
            MOD_SCMI_PROTOCOL_ID_PERF);
    }

    if (res_perms_backup.scmi_clock_perms != NULL) {
        mod_res_agent_copy_config(
            agent_id,
            res_perms_ctx.agent_permissions->scmi_clock_perms,
            res_perms_backup.scmi_clock_perms,
            MOD_SCMI_PROTOCOL_ID_CLOCK);
    }

    if (res_perms_backup.scmi_sensor_perms != NULL) {
        mod_res_agent_copy_config(
            agent_id,
            res_perms_ctx.agent_permissions->scmi_sensor_perms,
            res_perms_backup.scmi_sensor_perms,
            MOD_SCMI_PROTOCOL_ID_SENSOR);
    }

#ifdef BUILD_HAS_SCMI_RESET
    if (res_perms_backup.scmi_reset_domain_perms != NULL) {
        mod_res_agent_copy_config(
            agent_id,
            res_perms_ctx.agent_permissions->scmi_reset_domain_perms,
            res_perms_backup.scmi_reset_domain_perms,
            MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN);
    }
#endif

    return FWK_SUCCESS;
}

static const struct mod_res_permissions_api res_perms_api = {
    .agent_has_protocol_permission = agent_protocol_permissions,
    .agent_has_message_permission = agent_message_permissions,
    .agent_has_resource_permission = agent_resource_permissions,
    .agent_set_device_permission = mod_res_agent_set_device_permission,
    .agent_set_device_protocol_permission =
        mod_res_agent_set_device_protocol_permission,
    .agent_reset_config = mod_res_agent_reset_config,
};

/*
 * Module Framework support
 */
static int mod_res_perms_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only allow binding to the module */
    if (!fwk_id_is_equal(target_id, fwk_module_id_resource_perms))
        return FWK_E_PARAM;

    /* We don't do any permissions management */
    *api = &res_perms_api;

    return FWK_SUCCESS;
}

static int mod_res_perms_resources_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    struct mod_res_resource_perms_config *config;

    config = (struct mod_res_resource_perms_config *)data;
    if (config->agent_permissions != 0x0) {
        res_perms_ctx.agent_permissions =
            (struct mod_res_agent_permission *)config->agent_permissions;
        res_perms_ctx.agent_count = config->agent_count;
        res_perms_ctx.protocol_count = config->protocol_count;
        res_perms_ctx.clock_count = config->clock_count;
        res_perms_ctx.sensor_count = config->sensor_count;
        res_perms_ctx.pd_count = config->pd_count;
        res_perms_ctx.perf_count = config->perf_count;
        res_perms_ctx.device_count = config->device_count;
#ifdef BUILD_HAS_SCMI_RESET
        res_perms_ctx.reset_domain_count = config->reset_domain_count;
#endif
        res_perms_ctx.domain_devices =
            (struct mod_res_device *)config->domain_devices;
    }
    res_perms_ctx.config = config;
    return FWK_SUCCESS;
}

/* Module description */
const struct fwk_module module_resource_perms = {
    .name = "RESOURCE_PERMS",
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = mod_res_perms_resources_init,
    .process_bind_request = mod_res_perms_process_bind_request,
    .api_count = MOD_RES_PERM_API_IDX_COUNT,
};
