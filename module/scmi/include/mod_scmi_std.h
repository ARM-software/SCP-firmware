/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Control and Management Interface (SCMI) support.
 */

#ifndef MOD_SCMI_STD_H
#define MOD_SCMI_STD_H

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI System Control & Management Interface (SCMI)
 * \{
 */

/*!
 * \brief SCMI identifier of the platform.
 */
#define MOD_SCMI_PLATFORM_ID 0

/*!
 * \brief Maximum value for an agent identifier. The limit is derived from the
 *      the base protocol's "PROTOCOL_ATTRIBUTES" command. This command returns
 *      a 32-bits "attributes" value which, in turn, contains an 8-bit field
 *      giving the number of agents in the system.
 */
#define MOD_SCMI_AGENT_ID_MAX 0xFF

/*! Maximum value of an SCMI protocol identifier */
#define MOD_SCMI_PROTOCOL_ID_MAX 0xFF

/*! Minimum Protocol ID reserved for vendor or platform-specific extensions */
#define MOD_SCMI_PLATFORM_PROTOCOL_ID_MIN 0x80
/*! Maximum Protocol ID reserved for vendor or platform-specific extensions */
#define MOD_SCMI_PLATFORM_PROTOCOL_ID_MAX MOD_SCMI_PROTOCOL_ID_MAX

/*!
 * \brief SCMI error codes.
 */
enum scmi_error {
    SCMI_SUCCESS = 0,
    SCMI_NOT_SUPPORTED = -1,
    SCMI_INVALID_PARAMETERS = -2,
    SCMI_DENIED = -3,
    SCMI_NOT_FOUND = -4,
    SCMI_OUT_OF_RANGE = -5,
    SCMI_BUSY = -6,
    SCMI_COMMS_ERROR = -7,
    SCMI_GENERIC_ERROR = -8,
    SCMI_HARDWARE_ERROR = -9,
    SCMI_PROTOCOL_ERROR = -10,
};

/*!
 * \brief Common command identifiers.
 */
enum scmi_command_id {
    MOD_SCMI_PROTOCOL_VERSION = 0x000,
    MOD_SCMI_PROTOCOL_ATTRIBUTES = 0x001,
    MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES = 0x002
};

/*!
 * \brief Definitions of the SCMI Protocol Identifiers and the command
 *      identifiers for each protocol.
 */

/*!
 * \brief SCMI Base Protocol
 */
#define MOD_SCMI_PROTOCOL_ID_BASE UINT32_C(0x10)

/*!
 * \brief SCMI Base Protocol Message IDs
 */
enum scmi_base_command_id {
    MOD_SCMI_BASE_DISCOVER_VENDOR = 0x003,
    MOD_SCMI_BASE_DISCOVER_SUB_VENDOR = 0x004,
    MOD_SCMI_BASE_DISCOVER_IMPLEMENTATION_VERSION = 0x005,
    MOD_SCMI_BASE_DISCOVER_LIST_PROTOCOLS = 0x006,
    MOD_SCMI_BASE_DISCOVER_AGENT = 0x007,
    MOD_SCMI_BASE_NOTIFY_ERRORS = 0x008,
    MOD_SCMI_BASE_SET_DEVICE_PERMISSIONS = 0x009,
    MOD_SCMI_BASE_SET_PROTOCOL_PERMISSIONS = 0x00A,
    MOD_SCMI_BASE_RESET_AGENT_CONFIG = 0x00B,
    MOD_SCMI_BASE_COMMAND_COUNT,
};

/*!
 * \brief SCMI Power Domain Protocol
 */
#define MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN UINT32_C(0x11)

/*!
 * \brief SCMI Power Domain Protocol Message IDs
 */
enum scmi_pd_command_id {
    MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES = 0x03,
    MOD_SCMI_PD_POWER_STATE_SET = 0x04,
    MOD_SCMI_PD_POWER_STATE_GET = 0x05,
    MOD_SCMI_PD_POWER_STATE_NOTIFY = 0x06,
    MOD_SCMI_PD_POWER_STATE_CHANGE_REQUESTED_NOTIFY = 0x07,
    MOD_SCMI_PD_POWER_COMMAND_COUNT,
};

/*!
 * \brief SCMI System Power Protocol
 */
#define MOD_SCMI_PROTOCOL_ID_SYS_POWER UINT32_C(0x12)

/*!
 * \brief SCMI System Power Protocol Message IDs
 */
enum scmi_sys_power_command_id {
    MOD_SCMI_SYS_POWER_STATE_SET = 0x003,
    MOD_SCMI_SYS_POWER_STATE_GET = 0x004,
    MOD_SCMI_SYS_POWER_STATE_NOTIFY = 0x005,
    MOD_SCMI_SYS_POWER_COMMAND_COUNT,
};

/*!
 * \brief SCMI Performance Protocol
 */
#define MOD_SCMI_PROTOCOL_ID_PERF UINT32_C(0x13)

/*!
 * \brief SCMI Performance Protocol Message IDs
 */
enum scmi_perf_command_id {
    MOD_SCMI_PERF_DOMAIN_ATTRIBUTES = 0x003,
    MOD_SCMI_PERF_DESCRIBE_LEVELS = 0x004,
    MOD_SCMI_PERF_LIMITS_SET = 0x005,
    MOD_SCMI_PERF_LIMITS_GET = 0x006,
    MOD_SCMI_PERF_LEVEL_SET = 0x007,
    MOD_SCMI_PERF_LEVEL_GET = 0x008,
    MOD_SCMI_PERF_NOTIFY_LIMITS = 0x009,
    MOD_SCMI_PERF_NOTIFY_LEVEL = 0x00A,
    MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL = 0x00B,
    MOD_SCMI_PERF_COMMAND_COUNT,
};

/*!
 * \brief SCMI Clock Protocol
 */
#define MOD_SCMI_PROTOCOL_ID_CLOCK UINT32_C(0x14)

/*!
 * \brief SCMI Clock Protocol Message IDs
 */
enum scmi_clock_command_id {
    MOD_SCMI_CLOCK_ATTRIBUTES = 0x003,
    MOD_SCMI_CLOCK_DESCRIBE_RATES = 0x004,
    MOD_SCMI_CLOCK_RATE_SET = 0x005,
    MOD_SCMI_CLOCK_RATE_GET = 0x006,
    MOD_SCMI_CLOCK_CONFIG_SET = 0x007,
    MOD_SCMI_CLOCK_COMMAND_COUNT,
};

/*!
 * \brief SCMI Sensor Protocol
 */
#define MOD_SCMI_PROTOCOL_ID_SENSOR UINT32_C(0x15)

/*!
 * \brief SCMI Sensor Protocol Message IDs
 */
enum scmi_sensor_command_id {
    MOD_SCMI_SENSOR_DESCRIPTION_GET = 0x003,
    MOD_SCMI_SENSOR_TRIP_POINT_NOTIFY = 0x004,
    MOD_SCMI_SENSOR_TRIP_POINT_CONFIG = 0x005,
    MOD_SCMI_SENSOR_READING_GET = 0x006,
#ifdef BUILD_HAS_SCMI_SENSOR_V2
    MOD_SCMI_SENSOR_AXIS_DESCRIPTION_GET = 0x007,
#endif
    MOD_SCMI_SENSOR_COMMAND_COUNT,
};

/*!
 * \brief SCMI Reset Domain Protocol
 */
#define MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN UINT32_C(0x16)

/*!
 * \brief SCMI Reset Domain Management Protocol Message IDs
 */
enum scmi_reset_domain_command_id {
    MOD_SCMI_RESET_DOMAIN_ATTRIBUTES = 0x03,
    MOD_SCMI_RESET_REQUEST = 0x04,
    MOD_SCMI_RESET_NOTIFY = 0x05,
    MOD_SCMI_RESET_COMMAND_COUNT,
};

/*!
 * \brief SCMI Reset Domain Management Protocol Response IDs.
 */
enum scmi_reset_domain_response_id {
    MOD_SCMI_RESET_ISSUED = 0x00,
    MOD_SCMI_RESET_COMPLETE = 0x04,
};

/*!
 * \brief SCMI Voltage Domain Protocol
 */
#define MOD_SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN UINT32_C(0x17)

/*!
 * \brief SCMI Voltage Domain Protocol Message IDs
 */
enum scmi_voltd_command_id {
    MOD_SCMI_VOLTD_DOMAIN_ATTRIBUTES = 0x003,
    MOD_SCMI_VOLTD_DESCRIBE_LEVELS = 0x004,
    MOD_SCMI_VOLTD_CONFIG_SET = 0x005,
    MOD_SCMI_VOLTD_CONFIG_GET = 0x006,
    MOD_SCMI_VOLTD_LEVEL_SET = 0x007,
    MOD_SCMI_VOLTD_LEVEL_GET = 0x008,
    MOD_SCMI_VOLTD_COMMAND_COUNT,
};

/*!
 * \brief SCMI power capping and monitoring protocol
 */
#define MOD_SCMI_PROTOCOL_ID_POWER_CAPPING UINT32_C(0x18)

/*!
 * \brief SCMI power capping and monitoring protocol message IDs
 */
enum scmi_power_capping_command_id {
    MOD_SCMI_POWER_CAPPING_DOMAIN_ATTRIBUTES = 0x003,
    MOD_SCMI_POWER_CAPPING_CAP_GET = 0x004,
    MOD_SCMI_POWER_CAPPING_CAP_SET = 0x005,
    MOD_SCMI_POWER_CAPPING_PAI_GET = 0x006,
    MOD_SCMI_POWER_CAPPING_PAI_SET = 0x007,
    MOD_SCMI_POWER_CAPPING_DOMAIN_NAME_GET = 0x008,
    MOD_SCMI_POWER_CAPPING_MEASUREMENTS_GET = 0x009,
    MOD_SCMI_POWER_CAPPING_CAP_NOTIFY = 0x00A,
    MOD_SCMI_POWER_CAPPING_MEASUREMENTS_NOTIFY = 0x00B,
    MOD_SCMI_POWER_CAPPING_DESCRIBE_FAST_CHANNEL = 0x00C,
    MOD_SCMI_POWER_CAPPING_COMMAND_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_STD_H */
