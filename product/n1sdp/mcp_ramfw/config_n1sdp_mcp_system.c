/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_n1sdp_mcp_system.h>

#include <fwk_module.h>

/*
 * Data for the n1sdp_mcp_system module configuration
 */
static const struct mod_n1sdp_mcp_system_module_config n1sdp_mcp_system_data = {
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 0),
};

/*
 * Configuration for the n1sdp_mcp_system module
 */
struct fwk_module_config config_n1sdp_mcp_system = {
    .data = &n1sdp_mcp_system_data
};
