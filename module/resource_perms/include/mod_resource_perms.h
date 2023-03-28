/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Control and Management Interface (SCMI)
 *      Resource Management Support.
 */

#ifndef MOD_SCMI_RESOURCE_PERMISSIONS_H
#define MOD_SCMI_RESOURCE_PERMISSIONS_H

#include <mod_scmi_std.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_status.h>

#include <stdint.h>

/*
 * We will use type uint16_t to track the resources.
 */
typedef uint16_t mod_res_perms_t;

#define MOD_RES_PERMS_TYPE_BITS (sizeof(mod_res_perms_t) * CHAR_BIT)
#define MOD_RES_PERMS_TYPE_SHIFT (4U)
#define MOD_RES_PERMS_TYPE_MASK  ((1U << MOD_RES_PERMS_TYPE_SHIFT) - 1U)

/*! Find the array element for the resource */
#define MOD_RES_PERMS_RESOURCE_ELEMENT(resource_id) \
    (uint32_t)(resource_id >> MOD_RES_PERMS_TYPE_SHIFT)

/*! Find the bit in the array element for the resource */
#define MOD_RES_PERMS_RESOURCE_BIT(resource_id) \
    (uint32_t)(resource_id & MOD_RES_PERMS_TYPE_MASK)

/*!
 * \brief Bits to set to deny Protocol permissions. Note that the default
 *        throughout is to allow access to a Protocol:Message:Resource.
 */
enum mod_res_perms_permissions {
    MOD_RES_PERMS_ACCESS_ALLOWED = 0,
    MOD_RES_PERMS_ACCESS_DENIED = 1,
};

#define MOD_RES_PERMS_PERMISSIONS_MASK 0x1

#define MOD_RES_PERMS_PROTOCOL_OFFSET MOD_SCMI_PROTOCOL_ID_BASE

enum mod_res_perms_protocol_deny {
    MOD_RES_PERMS_SCMI_ALL_PROTOCOLS_ALLOWED = 0,
    MOD_RES_PERMS_SCMI_BASE_PROTOCOL_DENIED = MOD_RES_PERMS_ACCESS_DENIED
        << (MOD_SCMI_PROTOCOL_ID_BASE - MOD_RES_PERMS_PROTOCOL_OFFSET),
    MOD_RES_PERMS_SCMI_CLOCK_PROTOCOL_DENIED = MOD_RES_PERMS_ACCESS_DENIED
        << (MOD_SCMI_PROTOCOL_ID_CLOCK - MOD_RES_PERMS_PROTOCOL_OFFSET),
    MOD_RES_PERMS_SCMI_SYS_POWER_PROTOCOL_DENIED = MOD_RES_PERMS_ACCESS_DENIED
        << (MOD_SCMI_PROTOCOL_ID_SYS_POWER - MOD_RES_PERMS_PROTOCOL_OFFSET),
    MOD_RES_PERMS_SCMI_POWER_DOMAIN_DENIED = MOD_RES_PERMS_ACCESS_DENIED
        << (MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN - MOD_RES_PERMS_PROTOCOL_OFFSET),
    MOD_RES_PERMS_SCMI_PERF_PROTOCOL_DENIED = MOD_RES_PERMS_ACCESS_DENIED
        << (MOD_SCMI_PROTOCOL_ID_PERF - MOD_RES_PERMS_PROTOCOL_OFFSET),
    MOD_RES_PERMS_SCMI_SENSOR_PROTOCOL_DENIED = MOD_RES_PERMS_ACCESS_DENIED
        << (MOD_SCMI_PROTOCOL_ID_SENSOR - MOD_RES_PERMS_PROTOCOL_OFFSET),
    MOD_RES_PERMS_SCMI_RESET_DOMAIN_PROTOCOL_DENIED =
        MOD_RES_PERMS_ACCESS_DENIED
        << (MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN - MOD_RES_PERMS_PROTOCOL_OFFSET),
    MOD_RES_PERMS_SCMI_VOLTAGE_DOMAIN_PROTOCOL_DENIED =
        MOD_RES_PERMS_ACCESS_DENIED
        << (MOD_SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN -
            MOD_RES_PERMS_PROTOCOL_OFFSET),
};

/*!
 * \brief Bitmask of the protocols disabled for the agent.
 */
struct mod_res_agent_protocol_permissions {
    /*!
     * \details This bitmask specifies which protocols are DISABLED for this
     *      agent. If a bit is set that agent does NOT have permission to
     *      use the protocol.
     */
    mod_res_perms_t protocols;
};

/*!
 * \brief Bits to set to deny Message permissions. Note that the default
 *        throughout is to allow access to a Message:Resource.
 */
#define MOD_RES_PERMS_MESSAGE_OFFSET MOD_SCMI_PROTOCOL_ID_BASE

enum mod_res_perms_message_idx {
    MOD_RES_PERMS_SCMI_BASE_MESSAGE_IDX =
        MOD_SCMI_PROTOCOL_ID_BASE - MOD_RES_PERMS_MESSAGE_OFFSET,
    MOD_RES_PERMS_SCMI_CLOCK_MESSAGE_IDX =
        MOD_SCMI_PROTOCOL_ID_CLOCK - MOD_RES_PERMS_MESSAGE_OFFSET,
    MOD_RES_PERMS_SCMI_SYS_POWER_MESSAGE_IDX =
        MOD_SCMI_PROTOCOL_ID_SYS_POWER - MOD_RES_PERMS_MESSAGE_OFFSET,
    MOD_RES_PERMS_SCMI_POWER_DOMAIN_MESSAGE_IDX =
        MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN - MOD_RES_PERMS_MESSAGE_OFFSET,
    MOD_RES_PERMS_SCMI_PERF_MESSAGE_IDX =
        MOD_SCMI_PROTOCOL_ID_PERF - MOD_RES_PERMS_MESSAGE_OFFSET,
    MOD_RES_PERMS_SCMI_SENSOR_MESSAGE_IDX =
        MOD_SCMI_PROTOCOL_ID_SENSOR - MOD_RES_PERMS_MESSAGE_OFFSET,
    MOD_RES_PERMS_SCMI_RESET_DOMAIN_MESSAGE_IDX =
        MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN - MOD_RES_PERMS_MESSAGE_OFFSET,
    MOD_RES_PERMS_SCMI_VOLTAGE_DOMAIN_MESSAGE_IDX =
        MOD_SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN - MOD_RES_PERMS_MESSAGE_OFFSET,
    MOD_RES_PERMS_SCMI_POWER_CAPPING_MESSAGE_IDX =
        MOD_SCMI_PROTOCOL_ID_POWER_CAPPING - MOD_RES_PERMS_MESSAGE_OFFSET,
};

/*!
 * \brief Bits to set when denying message permissions. The
 *      VERSION/ATTRIBUTES/MSG_ATTRIBUTES messages are available to all
 *      agents. The message bits for each protocol are defined as
 *      offsets from the corresponding base index.
 */
enum mod_res_perms_message_bitmask_base {
    MOD_RES_PERMS_SCMI_BASE_BITMASK_IDX = MOD_SCMI_BASE_NOTIFY_ERRORS,
    MOD_RES_PERMS_SCMI_CLOCK_BITMASK_IDX = MOD_SCMI_CLOCK_ATTRIBUTES,
    MOD_RES_PERMS_SCMI_SYS_POWER_BITMASK_IDX = MOD_SCMI_SYS_POWER_STATE_SET,
    MOD_RES_PERMS_SCMI_POWER_DOMAIN_BITMASK_IDX =
        MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES,
    MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX = MOD_SCMI_PERF_DOMAIN_ATTRIBUTES,
    MOD_RES_PERMS_SCMI_SENSOR_BITMASK_IDX = MOD_SCMI_SENSOR_DESCRIPTION_GET,
    MOD_RES_PERMS_SCMI_RESET_DOMAIN_BITMASK_IDX =
        MOD_SCMI_RESET_DOMAIN_ATTRIBUTES,
    MOD_RES_PERMS_SCMI_VOLTAGE_DOMAIN_BITMASK_IDX =
        MOD_SCMI_VOLTD_DOMAIN_ATTRIBUTES,
};

/*!
 * \brief Bitmask of the messages for each protocol disabled for
 *      the agent. Currently we have 9 SCMI protocols.
 */
struct mod_res_agent_msg_permissions {
    /*! \details Bitmask of the disabled messages for each protocol. */
    mod_res_perms_t messages[9];
};

/*!
 * \brief Bitmask of the resources for each protocol:command disabled for
 *      the agent.
 *
 * \details Each protocol will manage a unique number of commands, so
 *      we specify different size tables.
 *
 * If the permissions table is not set then the agent is deemed
 *      to be allowed access that protocol:command:resource. They will
 *      be checked in order protocol->command->resource.
 *
 * In order for a protocol to be disabled for an agent, the
 *      bit must be SET in the agent_protocol_permissions table.
 *
 * In order for a command to be disabled for an agent, the
 *      bit must be SET in the agent_cmd_permissions table.
 *
 * In order for a resource to be disabled for an agent, the
 *      bit must be SET in the agent_permission table.
 *
 * \note The VERSION/ATTRIBUTES/MSG_ATTRIBUTES commands
 *      are available to all agents.
 *
 * \note The BASE and SYSTEM_POWER protocols are managed by
 *      agent:protocol:command, there are no resource
 *      permissions associated with these protocols.
 *
 */

/*!
 * \brief SCMI Clock Protocol Message index offset
 */
#define MOD_RES_PERMS_CLOCK_PERMS_OFFSET MOD_SCMI_CLOCK_ATTRIBUTES

/*!
 * \brief SCMI Clock Protocol Message Indices
 */
enum mod_res_clock_permissions_idx {
    MOD_RES_PERMS_SCMI_CLOCK_ATTRIBUTES_IDX =
        MOD_SCMI_CLOCK_ATTRIBUTES - MOD_RES_PERMS_CLOCK_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_CLOCK_DESCRIBE_RATE_IDX =
        MOD_SCMI_CLOCK_DESCRIBE_RATES - MOD_RES_PERMS_CLOCK_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_CLOCK_RATE_SET_IDX =
        MOD_SCMI_CLOCK_RATE_SET - MOD_RES_PERMS_CLOCK_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_CLOCK_RATE_GET_IDX =
        MOD_SCMI_CLOCK_RATE_GET - MOD_RES_PERMS_CLOCK_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_CLOCK_CONFIG_SET_IDX =
        MOD_SCMI_CLOCK_CONFIG_SET - MOD_RES_PERMS_CLOCK_PERMS_OFFSET,
};

/*!
 * \brief SCMI Power Domain Protocol Message index offset
 */
#define MOD_RES_PERMS_POWER_DOMAIN_PERMS_OFFSET \
    MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES

/*!
 * \brief SCMI Power Domain Protocol Message Indices
 */
enum mod_res_power_domain_permissions_idx {
    MOD_RES_PERMS_SCMI_POWER_DOMAIN_ATTRIBUTES_IDX =
        MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES -
        MOD_RES_PERMS_POWER_DOMAIN_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_SET_IDX =
        MOD_SCMI_PD_POWER_STATE_SET - MOD_RES_PERMS_POWER_DOMAIN_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_GET_IDX =
        MOD_SCMI_PD_POWER_STATE_GET - MOD_RES_PERMS_POWER_DOMAIN_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_NOTIFY_IDX =
        MOD_SCMI_PD_POWER_STATE_NOTIFY -
        MOD_RES_PERMS_POWER_DOMAIN_PERMS_OFFSET,
};

/*!
 * \brief SCMI Performance Protocol Message index offset
 */
#define MOD_RES_PERMS_PERF_PERMS_OFFSET MOD_SCMI_PERF_DOMAIN_ATTRIBUTES

/*!
 * \brief SCMI Performance Protocol Message Indices
 */
enum mod_res_perf_permissions_idx {
    MOD_RES_PERMS_SCMI_PERF_ATTRIBUTES_IDX =
        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES - MOD_RES_PERMS_PERF_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_PERF_DESCRIBE_LEVELS_IDX =
        MOD_SCMI_PERF_DESCRIBE_LEVELS - MOD_RES_PERMS_PERF_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_PERF_LIMITS_SET_IDX =
        MOD_SCMI_PERF_LIMITS_SET - MOD_RES_PERMS_PERF_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_PERF_LIMITS_GET_IDX =
        MOD_SCMI_PERF_LIMITS_GET - MOD_RES_PERMS_PERF_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_PERF_LEVEL_SET_IDX =
        MOD_SCMI_PERF_LEVEL_SET - MOD_RES_PERMS_PERF_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_PERF_LEVEL_GET_IDX =
        MOD_SCMI_PERF_LEVEL_GET - MOD_RES_PERMS_PERF_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_PERF_NOTIFY_LIMITS_IDX =
        MOD_SCMI_PERF_NOTIFY_LIMITS - MOD_RES_PERMS_PERF_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_PERF_NOTIFY_LEVEL_IDX =
        MOD_SCMI_PERF_NOTIFY_LEVEL - MOD_RES_PERMS_PERF_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_PERF_DESCRIBE_FAST_CHANNEL_IDX =
        MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL - MOD_RES_PERMS_PERF_PERMS_OFFSET,
};

/*!
 * \brief SCMI Sensor Protocol Message index offset
 */
#define MOD_RES_PERMS_SENSOR_PERMS_OFFSET MOD_SCMI_SENSOR_DESCRIPTION_GET

/*!
 * \brief SCMI Sensor Protocol Message Indices
 */
enum mod_res_sensor_permissions_idx {
    MOD_RES_PERMS_SCMI_SENSOR_DESCRIPTION_GET_IDX =
        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES - MOD_RES_PERMS_SENSOR_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_NOTIFY_IDX =
        MOD_SCMI_SENSOR_TRIP_POINT_NOTIFY - MOD_RES_PERMS_SENSOR_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_CONFIG_IDX =
        MOD_SCMI_SENSOR_TRIP_POINT_CONFIG - MOD_RES_PERMS_SENSOR_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_SENSOR_READING_GET_IDX =
        MOD_SCMI_SENSOR_READING_GET - MOD_RES_PERMS_SENSOR_PERMS_OFFSET,
};

/*!
 * \brief SCMI Reset Domain Management Protocol Message index offset
 */
#define MOD_RES_PERMS_RESET_DOMAIN_PERMS_OFFSET MOD_SCMI_RESET_DOMAIN_ATTRIBUTES

/*!
 * \brief SCMI Reset Domain Management Protocol Message Indices
 */
enum mod_res_reset_domain_permissions_idx {
    MOD_RES_PERMS_SCMI_RESET_DOMAIN_ATTRIBUTES_IDX =
        MOD_SCMI_RESET_DOMAIN_ATTRIBUTES -
        MOD_RES_PERMS_RESET_DOMAIN_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_REQUEST_IDX =
        MOD_SCMI_RESET_REQUEST - MOD_RES_PERMS_RESET_DOMAIN_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_NOTIFY_IDX =
        MOD_SCMI_RESET_NOTIFY - MOD_RES_PERMS_RESET_DOMAIN_PERMS_OFFSET,
};

/*!
 * \brief SCMI Voltage Domain Management Protocol Message index offset
 */
#define MOD_RES_PERMS_VOLTD_PERMS_OFFSET MOD_SCMI_VOLTD_DOMAIN_ATTRIBUTES

/*!
 * \brief SCMI Reset Domain Management Protocol Message Indices
 */
enum mod_res_voltage_domain_permissions_idx {
    MOD_RES_PERMS_SCMI_VOLTD_DOMAIN_ATTRIBUTES_IDX =
        MOD_SCMI_VOLTD_DOMAIN_ATTRIBUTES - MOD_RES_PERMS_VOLTD_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_VOLTD_DESCRIBE_LEVELS_IDX =
        MOD_SCMI_VOLTD_DESCRIBE_LEVELS - MOD_RES_PERMS_VOLTD_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_VOLTD_CONFIG_SET_IDX =
        MOD_SCMI_VOLTD_CONFIG_SET - MOD_RES_PERMS_VOLTD_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_VOLTD_CONFIG_GET_IDX =
        MOD_SCMI_VOLTD_CONFIG_GET - MOD_RES_PERMS_VOLTD_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_VOLTD_LEVEL_SET_IDX =
        MOD_SCMI_VOLTD_LEVEL_SET - MOD_RES_PERMS_VOLTD_PERMS_OFFSET,
    MOD_RES_PERMS_SCMI_VOLTD_LEVEL_GET_IDX =
        MOD_SCMI_VOLTD_LEVEL_GET - MOD_RES_PERMS_VOLTD_PERMS_OFFSET,
};

/*!
 * \brief SCMI Domain Types
 */
enum mod_res_domain_device_types {
    MOD_RES_POWER_DOMAIN_DEVICE = 0,
    MOD_RES_PERF_DOMAIN_DEVICE,
    MOD_RES_CLOCK_DOMAIN_DEVICE,
    MOD_RES_SENSOR_DOMAIN_DEVICE,
    MOD_RES_RESET_DOMAIN_DEVICE,
    MOD_RES_VOLTAGE_DOMAIN_DEVICE,
    MOD_RES_POWER_CAPPING_DEVICE,
    MOD_RES_PLATFORM_DOMAIN_DEVICE,
    MOD_RES_DOMAIN_DEVICE_INVALID
};

/*!
 * \brief Each device is made up multiple domain devices.
 *      The protocol is determined by the device type.
 *      The resource ID for a protocol is the element ID of the
 *      device_id.
 */
struct mod_res_domain_device {
    /*! \brief Identifier of the domain device instance */
    fwk_id_t device_id;

    /*! \brief Type of the domain device instance */
    enum mod_res_domain_device_types type;
};

/*!
 * \brief Device definition.
 */
struct mod_res_device {
    /*! \brief Device Identifier */
    uint16_t device_id;

    /*! \brief List of domain devices in the device */
    struct mod_res_domain_device *domain_devices;
};

/*!
 * \brief SCMI Agent Permissions
 *
 * \details An agent may have any combination of the permissions
 *      tables set.
 */
struct mod_res_agent_permission {
    /*! \brief  Protocol permissions. */
    struct mod_res_agent_protocol_permissions *agent_protocol_permissions;

    /*! \brief Protocol:Message permissions. */
    struct mod_res_agent_msg_permissions *agent_msg_permissions;

    /*! \brief Power Domain:Resource permissions. */
    mod_res_perms_t *scmi_pd_perms;

    /*! Performance:Resource permissions. */
    mod_res_perms_t *scmi_perf_perms;

    /*! \brief Clock:Resource permissions. */
    mod_res_perms_t *scmi_clock_perms;

    /*! \brief Sensor:Resource permissions. */
    mod_res_perms_t *scmi_sensor_perms;

#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
    /*! \brief Reset Domain:Resource permissions. */
    mod_res_perms_t *scmi_reset_domain_perms;
#endif
    /*! \brief Voltage Domain:Resource permissions. */
    mod_res_perms_t *scmi_voltd_perms;

    /*! \brief Power capping:Resource permissions. */
    mod_res_perms_t *scmi_power_capping_perms;
};

/*!
 * \brief Type of the interfaces exposed by the resource permissions module.
 */
enum mod_res_perms_api_idx {
    MOD_RES_PERM_RESOURCE_PERMS,
    MOD_RES_PERM_API_IDX_COUNT,
};

/*!
 * \brief Interfaces exposed by the resource permissions module.
 */
struct mod_res_permissions_api {
    /*!
     * \brief Check whether the agent has permission to access a protocol.
     *
     * \param agent_id      identifier of the agent.
     * \param protocol_id   identifier of the protocol.
     *
     * \retval MOD_RES_PERMS_ACCESS_ALLOWED The agent has permissions to
     *      use the protocol.
     * \retval MOD_RES_PERMS_ACCESS_DENIED The agent does not have
     *      permissions to use the protocol.
     */
    enum mod_res_perms_permissions (*agent_has_protocol_permission)(
        uint32_t agent_id,
        uint32_t protocol_id);

    /*!
     * \brief Check whether the agent has permission to access a message.
     *
     * \param agent_id      identifier of the agent.
     * \param protocol_id   identifier of the protocol.
     * \param message_id    identifier of the message.
     *
     * \retval MOD_RES_PERMS_ACCESS_ALLOWED The agent has permissions to
     *      use the protocol.
     * \retval MOD_RES_PERMS_ACCESS_DENIED The agent does not have
     *      permissions to use the message.
     */
    enum mod_res_perms_permissions (*agent_has_message_permission)(
        uint32_t agent_id,
        uint32_t protocol_id,
        uint32_t message_id);

    /*!
     * \brief Check whether the agent has permission to access a resource.
     *
     * \param agent_id      identifier of the agent.
     * \param protocol_id   identifier of the protocol.
     * \param message_id    identifier of the message.
     * \param resource_id   identifier of the resource.
     *
     * \retval MOD_RES_PERMS_ACCESS_ALLOWED The agent has permissions to
     *      use the protocol.
     * \retval MOD_RES_PERMS_ACCESS_DENIED The agent does not have
     *      permissions to use the resource.
     */
    enum mod_res_perms_permissions (*agent_has_resource_permission)(
        uint32_t agent_id,
        uint32_t protocol_id,
        uint32_t message_id,
        uint32_t resource_id);

    /*!
     * \brief Set device permissions for an agent
     *
     * \param agent_id      identifier of the agent.
     * \param device_id     identifier of the device.
     * \param flags         permissions to set.
     *
     * \retval ::FWK_SUCCESS  The operation has completed successfully.
     * \retval ::FWK_E_ACCESS Unknown agent_id or device_id.
     * \retval ::FWK_E_PARAM  Invalid flags or protocol_ID.
     */
    int (*agent_set_device_permission)(
        uint32_t agent_id,
        uint32_t device_id,
        uint32_t flags);

    /*!
     * \brief Set device protocol permissions for an agent
     *
     * \param agent_id      identifier of the agent.
     * \param device_id     identifier of the device.
     * \param device_id     identifier of the protocol.
     * \param flags         permissions to set.
     *
     * \retval ::FWK_SUCCESS  The operation has completed successfully.
     * \retval ::FWK_E_ACCESS Unknown agent_id or device_id.
     * \retval ::FWK_E_PARAM  Invalid flags or protocol_ID.
     */
    int (*agent_set_device_protocol_permission)(
        uint32_t agent_id,
        uint32_t device_id,
        uint32_t protocol_id,
        uint32_t flags);

    /*!
     * \brief Reset permissions for an agent
     *
     * \param agent_id      identifier of the agent.
     * \param flags         permissions to set.
     *
     * \retval ::FWK_SUCCESS  The operation has completed successfully.
     * \retval ::FWK_E_ACCESS Unknown agent_id.
     * \retval ::FWK_E_PARAM  Invalid flags.
     */
    int (*agent_reset_config)(uint32_t agent_id, uint32_t flags);
};

/*!
 * \brief Protocol permission counters.
 */
struct protocol_permissions_counters {
    /*! \brief Number of protocol elements supported by the platform. */
    uint32_t count;

    /*! \brief Number of commands supported by the platform. */
    uint32_t cmd_count;

    /*! \brief Number of resources supported by the platform. */
    uint32_t resource_count;
};

/*!
 * \brief Resource Permissions module configuration data.
 *
 * \note If the agent_permissions table is not set in the config then no
 *      resource permissions are implemented.
 */
struct mod_res_resource_perms_config {
    /*! \brief Number of agents on the platform. */
    uint32_t agent_count;

    /*! \brief Number of SCMI protocols supported by the platform. */
    uint32_t protocol_count;

    /*! \brief Number of SCMI clock protocol counters. */
    struct protocol_permissions_counters clock_counters;

    /*! \brief Number of SCMI sensor protocol counters. */
    struct protocol_permissions_counters sensor_counters;

    /*! \brief Number of SCMI power domain protocol counters. */
    struct protocol_permissions_counters pd_counters;

    /*! \brief Number of SCMI perf protocol counters. */
    struct protocol_permissions_counters perf_counters;

#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
    /*! \brief Number of SCMI reset_domain protocol counters. */
    struct protocol_permissions_counters reset_domain_counters;
#endif

    /*! \brief Number of SCMI voltd protocol counters. */
    struct protocol_permissions_counters voltd_counters;

    /*! \brief Number of SCMI power capping protocol counters. */
    struct protocol_permissions_counters power_capping_counters;

    /*! \brief Number of devices supported by the platform. */
    uint32_t device_count;

    /*! \brief Address of the permissions table */
    uintptr_t agent_permissions;

    /*! \brief Address of the domain devices */
    uintptr_t domain_devices;
};

/*!
 * \defgroup GroupResPerms Mapping
 *
 * \brief Resource Permissions Identifier Mapping.
 *
 * \details The Resource Permissions Identifier Checkin and Mapping functions
 *      are weak definitions to allow a platform to implement a function
 *      for mapping agent IDs, SCMI protocol IDs and SCMI Message IDs
 *      appropriate to that platform. The permissons checking functions
 *      may also be implemented. The Resource Permissions Identifier
 *      Mapping/Checking functions may be overridden in the
 *      `product/<platform>/src` directory.
 *      This may be useful for platforms with non-contiguous or
 *      platform-specific agent:protocol:command identifiers.
 *
 * \{
 */

/*!
 * \brief Resource Permissions Map Agent ID to index.
 *
 * \details This function maps an Agent ID to an index in the
 *      resource permissions table.
 *
 *      The Resource Permissions Mapping handlers have default
 *      weak implementations that allow a platform to implement
 *      a policy appropriate for that platform.
 *
 * \param[out] agent_idx Index for agent.
 * \param[in] agent_id Identifier of the agent.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int mod_res_agent_id_to_index(uint32_t agent_id, uint32_t *agent_idx);

/*!
 * \brief Resource Permissions Map Protocol ID to index.
 *
 * \details This function maps an SCMI Protocol ID to an index in the
 *      resource permissions table.
 *
 *      The Resource Permissions Mapping handlers have default
 *      weak implementations that allow a platform to implement
 *      a policy appropriate for that platform.
 *
 * \param[out] protocol_idx Index for protocol.
 * \param[in] protocol_id Identifier of the Protocol.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int mod_res_plat_protocol_id_to_index(
    uint32_t protocol_id,
    uint32_t *protocol_idx);

/*!
 * \brief Resource Permissions Map Message ID to index.
 *
 * \details This function maps an SCMI Message ID to an index in the
 *      resource permissions table.
 *
 *      The Resource Permissions Mapping handlers have default
 *      weak implementations that allow a platform to implement
 *      a policy appropriate for that platform.
 *
 * \param[out] message_idx Index for message.
 * \param[in] protocol_id Identifier of the Protocol.
 * \param[in] message_id Identifier of the Message.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int mod_res_plat_message_id_to_index(
    uint32_t protocol_id,
    uint32_t message_id,
    int32_t *message_idx);

/*!
 * \brief Resource Permissions Map Resource ID to index.
 *
 * \details This function maps an SCMI Resource ID to an index in the
 *      resource permissions table.
 *
 *      The Resource Permissions Mapping handlers have default
 *      weak implementations that allow a platform to implement
 *      a policy appropriate for that platform.
 *
 * \param[out] resource_idx Index for resource.
 * \param[in] agent_id Identifier of the Agent.
 * \param[in] protocol_id Identifier of the Protocol.
 * \param[in] message_id Identifier of the Message.
 * \param[in] resource_id Identifier of the Resource.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int mod_res_plat_resource_id_to_index(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id,
    uint32_t resource_id,
    int32_t *resource_idx);

/*!
 * \brief Check whether the agent has permission to access a protocol.
 *
 * \param agent_id      identifier of the agent.
 * \param protocol_id   identifier of the protocol.
 *
 * \retval MOD_RES_PERMS_ACCESS_ALLOWED The agent has permissions to
 *      use the protocol.
 * \retval MOD_RES_PERMS_ACCESS_DENIED The agent does not have
 *      permissions to use the protocol.
 */
enum mod_res_perms_permissions mod_res_plat_agent_protocol_permission(
    uint32_t agent_id,
    uint32_t protocol_id);

/*!
 * \brief Check whether the agent has permission to access a message.
 *
 * \param agent_id      identifier of the agent.
 * \param protocol_id   identifier of the protocol.
 * \param message_id    identifier of the message.
 *
 * \retval MOD_RES_PERMS_ACCESS_ALLOWED The agent has permissions to
 *      use the protocol.
 * \retval MOD_RES_PERMS_ACCESS_DENIED The agent does not have
 *      permissions to use the message.
 */
enum mod_res_perms_permissions mod_res_plat_agent_message_permission(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id);

/*!
 * \brief Check whether the agent has permission to access a resource.
 *
 * \param agent_id      identifier of the agent.
 * \param protocol_id   identifier of the protocol.
 * \param message_id    identifier of the message.
 * \param resource_id   identifier of the resource.
 *
 * \retval MOD_RES_PERMS_ACCESS_ALLOWED The agent has permissions to
 *      use the protocol.
 * \retval MOD_RES_PERMS_ACCESS_DENIED The agent does not have
 *      permissions to use the resource.
 */
enum mod_res_perms_permissions mod_res_plat_agent_resource_permissions(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id,
    uint32_t resource_id);

/*!
 * \}
 */

#endif /* MOD_SCMI_RESOURCE_PERMISSIONS_H */
