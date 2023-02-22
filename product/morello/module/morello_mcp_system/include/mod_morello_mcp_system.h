/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO MCP System Support
 */

#ifndef MOD_MORELLO_MCP_SYSTEM_H
#define MOD_MORELLO_MCP_SYSTEM_H

#include <fwk_module_idx.h>

/*!
 * \addtogroup GroupMORELLOModule MORELLO Product Modules
 * \{
 */

/*!
 * \defgroup GroupMORELLOMCPSystem MORELLO MCP System Support
 * \{
 */

#define MORELLO_SCP_AGENT_SCMI_RESPONSE_TIMEOUT_MILLISEC 1000

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
 * \brief MORELLO MCP System module configuration.
 */
struct mod_morello_mcp_system_module_config {
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

#endif /* MOD_MORELLO_MCP_SYSTEM_H */
