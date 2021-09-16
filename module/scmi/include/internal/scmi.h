/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
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
 * \{
 */

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
 *      The POWER_STATE_SET command targeting a power domain, including AP
 *      cores, is processed only if issued by a PSCI agent. The processing of
 *      the SYSTEM_POWER_STATE_SET command depends on the type of the agent that
 *      issued it. The OTHER type is added here to cover the other type of
 *      agents.
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
 * \details Defines the channel direction in terms of the requester to the
 *      completer.
 *
 * \note The integer values of this enumeration are based on the requester of
 *      communications in that configuration.
 */
enum scmi_channel_type {
    /*!< Agent-to-platform */
    SCMI_CHANNEL_TYPE_A2P = SCMI_ROLE_AGENT,

    /*!< Platform-to-agent */
    SCMI_CHANNEL_TYPE_P2A = SCMI_ROLE_PLATFORM
};


/*!
 * \brief Generic platform-to-agent PROTOCOL_VERSION structure.
 */
struct scmi_protocol_version_p2a {
    int32_t status;
    uint32_t version;
};

/*!
 * \brief Generic platform-to-agent PROTOCOL_ATTRIBUTES structure.
 */
struct scmi_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
};

/*!
 * \brief Generic agent-to-platform PROTOCOL_MESSAGE_ATTRIBUTES structure.
 */
struct scmi_protocol_message_attributes_a2p {
    uint32_t message_id;
};

/*!
 * \brief Generic platform-to-agent PROTOCOL_MESSAGE_ATTRIBUTES structure.
 */
struct scmi_protocol_message_attributes_p2a {
    int32_t status;
    uint32_t attributes;
};

/*!
 * \}
 */

#endif /* INTERNAL_SCMI_H */
