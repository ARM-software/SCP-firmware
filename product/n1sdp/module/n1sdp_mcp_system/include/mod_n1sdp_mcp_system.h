/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP MCP System Support
 */

#ifndef MOD_N1SDP_MCP_SYSTEM_H
#define MOD_N1SDP_MCP_SYSTEM_H

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupN1SDPMCPSystem N1SDP MCP System Support
 * @{
 */

/*!
 * \brief Events used by MCP system module.
 */
enum mcp_system_event {
    /*! MCP system run event */
    MOD_MCP_SYSTEM_EVENT_RUN,
    /*! MCP system event count */
    MOD_MCP_SYSTEM_EVENT_COUNT,
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_N1SDP_MCP_SYSTEM_H */
