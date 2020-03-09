/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
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
 * @{
 */

/*!
 * \defgroup GroupN1SDPScmiAgent SCMI Agent Support
 * @{
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
    SCMI_MANAGEMENT_PROTOCOL_VERSION_GET    = 0x0,
    /*! Message ID for getting protocol attributes */
    SCMI_MANAGEMENT_PROTOCOL_ATTRIBUTES_GET = 0x1,
    /*! Message ID for getting message attributes */
    SCMI_MANAGEMENT_MESSAGE_ATTRIBUTES_GET  = 0x2,
    /*! Message ID for getting clock status */
    SCMI_MANAGEMENT_CLOCK_STATUS_GET        = 0x3,
    /*! Message ID for getting chip ID information */
    SCMI_MANAGEMENT_CHIPID_INFO_GET         = 0x4,
};

/*!
 * \brief SCMI agent configuration data.
 */
struct mod_scmi_agent_config {
    /*!
     * \brief Identifier of the transport entity.
     */
    fwk_id_t transport_id;

    /*!
     * \brief Identifier of the API of the transport entity.
     */
    fwk_id_t transport_api_id;
};

/*!
 * \brief SCMI Management Agent API Interface
 *
 * \details Interface used for MCP System -> SCMI Agent.
 */
struct mod_scmi_agent_api {
    /*!
     * \brief Get the management protocol version from SCP
     *
     * \param agent_id Agent identifier
     * \param[out] version Protocol version.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*get_protocol_version)(fwk_id_t agent_id, uint32_t *version);

    /*!
     * \brief Get the PLL clock status from SCP
     *
     * \param agent_id Agent identifier
     * \param[out] clock_status SCP clock status.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*get_clock_status)(fwk_id_t agent_id, uint32_t *clock_status);

    /*!
     * \brief Get the chip ID information from SCP
     *
     * \param agent_id Agent identifier
     * \param[out] multichip_mode Multi-chip mode value.
     * \param[out] chipid Chip ID value.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*get_chipid_info)(fwk_id_t agent_id, uint8_t *multichip_mode,
                           uint8_t *chipid);
};

/*!
 * \brief API types exposed by SCMI agent module.
 */
enum mod_scmi_agent_api_idx {
    /*! API ID to be binded by system module */
    MOD_SCMI_AGENT_API_IDX_SYSTEM,
    /*! API ID count */
    MOD_SCMI_AGENT_API_IDX_COUNT,
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SCMI_AGENT_H */
