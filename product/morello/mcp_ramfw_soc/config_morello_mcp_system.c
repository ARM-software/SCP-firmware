/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_morello_mcp_system.h>

#include <fwk_module.h>

/*
 * Data for the morello_mcp_system module configuration
 */
static const struct mod_morello_mcp_system_module_config
    morello_mcp_system_data = {
        .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 0),
    };

/*
 * Configuration for the morello_mcp_system module
 */
struct fwk_module_config config_morello_mcp_system = {
    .data = &morello_mcp_system_data
};
