/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP MCP System Support
 */

#ifndef MOD_N1SDP_MCP_SYSTEM_H
#define MOD_N1SDP_MCP_SYSTEM_H

#include <fwk_module_idx.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * \{
 */

/*!
 * \defgroup GroupN1SDPMCPSystem N1SDP MCP System Support
 * \{
 */

#define N1SDP_SCP_AGENT_SCMI_RESPONSE_TIMEOUT_MILLISEC 1000

/*!
 * \brief Events used by MCP system module.
 */
enum mcp_system_event {
    /*! MCP system SCP handshake event */
    MOD_MCP_SYSTEM_EVENT_SCP_HANDSHAKE,
    /*! MCP system get protocol version event */
    MOD_MCP_SYSTEM_EVENT_PROTOCOL_VERSION_GET,
    /*! MCP system get clock status event */
    MOD_MCP_SYSTEM_EVENT_CLOCK_STATUS_GET,
    /*! MCP system get CHIPID info event */
    MOD_MCP_SYSTEM_EVENT_CHIPID_INFO_GET,
    /*! MCP system initialization complete event */
    MOD_MCP_SYSTEM_EVENT_INITIALIZATION_COMPLETE,
    /*! MCP system event count */
    MOD_MCP_SYSTEM_EVENT_COUNT,
};

/*!
 * \brief SCP clock status initialized.
 */
#define SCP_CLOCK_STATUS_INITIALIZED UINT32_C(0x1)

/*!
 * \brief N1SDP MCP System module configuration.
 */
struct mod_n1sdp_mcp_system_module_config {
    /*!
     * Element identifier of the alarm used for timeouts
     */
    fwk_id_t alarm_id;
};
/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_N1SDP_MCP_SYSTEM_H */
