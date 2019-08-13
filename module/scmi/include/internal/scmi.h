/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) protocol independent
 *      definitions.
 */

#ifndef INTERNAL_SCMI_H
#define INTERNAL_SCMI_H

#include <stdint.h>

/*!
 * \defgroup GroupSCMI System Control & Management Interface (SCMI)
 * @{
 */

/*!
 * \brief Maximum value for an agent identifier. The limit is derived from the
 *      the base protocol's "PROTOCOL_ATTRIBUTES" command. This command returns
 *      a 32-bits "attributes" value which, in turn, contains an 8-bit field
 *      giving the number of agents in the system.
 */
#define SCMI_AGENT_ID_MAX 0xFF

/*!
 * \brief SCMI identifier of the platform.
 */
#define SCMI_PLATFORM_ID 0

/*! Maximum value of an SCMI protocol identifier */
#define SCMI_PROTOCOL_ID_MAX 0xFF

/*!
 * \brief Return a packed 32-bit message header comprised of a 10-bit message
 *      identifier, an 8-bit protocol identifier, and a 10-bit token.
 *
 * \param MESSAGE_ID Message identifier.
 * \param PROTOCOL_ID Protocol identifier.
 * \param TOKEN Token.
 */
#define SCMI_MESSAGE_HEADER(MESSAGE_ID, PROTOCOL_ID, TOKEN) \
    ((((MESSAGE_ID) << SCMI_MESSAGE_HEADER_MESSAGE_ID_POS) & \
        SCMI_MESSAGE_HEADER_MESSAGE_ID_MASK) | \
    (((PROTOCOL_ID) << SCMI_MESSAGE_HEADER_PROTOCOL_ID_POS) & \
        SCMI_MESSAGE_HEADER_PROTOCOL_ID_MASK) | \
    (((TOKEN) << SCMI_MESSAGE_HEADER_TOKEN_POS) & \
        SCMI_MESSAGE_HEADER_TOKEN_POS))

/*!
 * \brief Entity role.
 */
enum scmi_role {
    /*! Agent entity */
    SCMI_ROLE_AGENT,

    /*! Platform entity */
    SCMI_ROLE_PLATFORM
};

/*!
 * \brief Agent type
 *
 * \details The SCMI specification defines three specific agent types:
 *          - a PSCI implementation on AP processors.
 *          - a management agent.
 *          - an OSPM.
 * The POWER_STATE_SET command targeting a power domain, including AP cores, is
 * processed only if issued by a PSCI agent. The processing of the
 * SYSTEM_POWER_STATE_SET command depends on the type of the agent that issued
 * it. The OTHER type is added here to cover the other type of agents.
 */
enum scmi_agent_type {
    /*! PSCI agent */
    SCMI_AGENT_TYPE_PSCI,

    /*! Management agent */
    SCMI_AGENT_TYPE_MANAGEMENT,

    /*! OSPM agent */
    SCMI_AGENT_TYPE_OSPM,

    /*! Other agent */
    SCMI_AGENT_TYPE_OTHER,

    /*! Number of agent types */
    SCMI_AGENT_TYPE_COUNT,
};

/*!
 * \brief Channel type.
 *
 * \details Defines the channel direction in terms of the master to the slave.
 *
 * \note The integer values of this enumeration are based on the master of
 * communications in that configuration.
 */
enum scmi_channel_type {
    /*!< Agent-to-platform */
    SCMI_CHANNEL_TYPE_A2P = SCMI_ROLE_AGENT,

    /*!< Platform-to-agent */
    SCMI_CHANNEL_TYPE_P2A = SCMI_ROLE_PLATFORM
};

/*!
 * \brief SCMI error codes.
 */
enum scmi_error {
    SCMI_SUCCESS            =  0,
    SCMI_NOT_SUPPORTED      = -1,
    SCMI_INVALID_PARAMETERS = -2,
    SCMI_DENIED             = -3,
    SCMI_NOT_FOUND          = -4,
    SCMI_OUT_OF_RANGE       = -5,
    SCMI_BUSY               = -6,
    SCMI_COMMS_ERROR        = -7,
    SCMI_GENERIC_ERROR      = -8,
    SCMI_HARDWARE_ERROR     = -9,
    SCMI_PROTOCOL_ERROR     = -10,
};

/*!
 * \brief Common command identifiers.
 */
enum scmi_command_id {
    SCMI_PROTOCOL_VERSION            = 0x000,
    SCMI_PROTOCOL_ATTRIBUTES         = 0x001,
    SCMI_PROTOCOL_MESSAGE_ATTRIBUTES = 0x002
};

/*!
 * \brief Generic platform-to-agent PROTOCOL_VERSION structure.
 */
struct __attribute((packed)) scmi_protocol_version_p2a {
    int32_t status;
    uint32_t version;
};

/*!
 * \brief Generic platform-to-agent PROTOCOL_ATTRIBUTES structure.
 */
struct __attribute((packed)) scmi_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
};

/*!
 * \brief Generic agent-to-platform PROTOCOL_MESSAGE_ATTRIBUTES structure.
 */
struct __attribute((packed)) scmi_protocol_message_attributes_a2p {
    uint32_t message_id;
};

/*!
 * \brief Generic platform-to-agent PROTOCOL_MESSAGE_ATTRIBUTES structure.
 */
struct __attribute((packed)) scmi_protocol_message_attributes_p2a {
    int32_t status;
    uint32_t attributes;
};

/*!
 * @}
 */

#endif /* INTERNAL_SCMI_H */
