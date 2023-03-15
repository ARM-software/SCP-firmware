/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Agent Support
 */

#ifndef MOD_SCMI_AGENT_H
#define MOD_SCMI_AGENT_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * \{
 */

/*!
 * \defgroup GroupN1SDPScmiAgent SCMI Agent Support
 * \{
 */

/*!
 * \brief SCMI agent - Management protocol ID definition.
 */
#define SCMI_PROTOCOL_ID_MANAGEMENT      UINT32_C(0x89)
/*!
 * \brief SCMI agent - Management protocol version definition.
 */
#define SCMI_PROTOCOL_VERSION_MANAGEMENT UINT32_C(0x10000)

/*!
 * \brief SCMI agent - Management protocol message IDs.
 */
enum scmi_management_message_id {
    /*! Message ID for getting protocol version */
    SCMI_MANAGEMENT_PROTOCOL_VERSION_GET = 0x0,
    /*! Message ID for getting protocol attributes */
    SCMI_MANAGEMENT_PROTOCOL_ATTRIBUTES_GET = 0x1,
    /*! Message ID for getting message attributes */
    SCMI_MANAGEMENT_MESSAGE_ATTRIBUTES_GET = 0x2,
    /*! Message ID for getting clock status */
    SCMI_MANAGEMENT_CLOCK_STATUS_GET = 0x3,
    /*! Message ID for getting chip ID information */
    SCMI_MANAGEMENT_CHIPID_INFO_GET = 0x4,
    /*! Count of Message IDs */
    SCMI_MANAGEMENT_MESSAGE_ID_COUNT,
};

/*!
 * \brief SCMI agent - Management protocol response payload sizes.
 */
enum scmi_management_message_payload_size {
    /*! Payload size for protocol version get response */
    PROTOCOL_VERSION_GET_RESPONSE_PAYLOAD_SIZE = 8,
    /*! Payload size for clock status get response*/
    CLOCK_STATUS_GET_RESPONSE_PAYLOAD_SIZE = 8,
    /*! Payload size for chip ID get response */
    CHIPID_INFO_GET_RESPONSE_PAYLOAD_SIZE = 12,
};

/*!
 * \brief SCMI agent configuration data.
 */
struct mod_scmi_agent_config {
    /*!
     * \brief The service ID which corresponds to the required
     * channel in the transport layer.
     */
    fwk_id_t service_id;
};

/*!
 * \brief Parameters of the protocol version response event.
 */
struct mod_scmi_agent_protocol_version_event_param {
    /*! Protocol version */
    uint32_t protocol_version;
};

/*!
 * \brief Parameters of the clock status response event.
 */
struct mod_scmi_agent_clock_status_event_param {
    /*! Clock Status */
    uint32_t clock_status;
};

/*!
 * \brief Parameters of the chipid response event.
 */
struct mod_scmi_agent_chipid_info_event_param {
    /*! Multichip mode */
    uint8_t multichip_mode;
    /*! Chipid info */
    uint8_t chipid;
};

/*!
 * \brief SCMI Management Agent API Interface
 *
 * \details Interface used for MCP System -> SCMI Agent.
 */
struct mod_scmi_agent_api {
    /*!
     * \brief Trigger SCMI request to get the management protocol version from
     * SCP
     *
     * \param agent_id Agent identifier
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*get_protocol_version)(fwk_id_t agent_id);

    /*!
     * \brief Trigger SCMI request to get the PLL clock status from SCP
     *
     * \param agent_id Agent identifier
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*get_clock_status)(fwk_id_t agent_id);

    /*!
     * \brief Trigger SCMI request to get the chip ID information from SCP
     *
     * \param agent_id Agent identifier
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*get_chipid_info)(fwk_id_t agent_id);
};

/*!
 * \brief API types exposed by SCMI agent module.
 */
enum mod_scmi_agent_api_idx {
    /*! API ID to be bound by SCMI module */
    MOD_SCMI_AGENT_API_IDX_SCMI,
    /*! API ID to be bound by system module */
    MOD_SCMI_AGENT_API_IDX_SYSTEM,
    /*! API ID count */
    MOD_SCMI_AGENT_API_IDX_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_AGENT_H */
