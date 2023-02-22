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

/*!
 * \addtogroup GroupMORELLOModule MORELLO Product Modules
 * \{
 */

/*!
 * \defgroup GroupMORELLOMCPSystem MORELLO MCP System Support
 * \{
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
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MORELLO_MCP_SYSTEM_H */
