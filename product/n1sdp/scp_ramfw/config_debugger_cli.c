/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_debugger_cli.h>

#include <fwk_module_idx.h>

/*
 * Data for the debugger CLI module configuration
 */
static const struct mod_debugger_cli_module_config debugger_cli_data = {
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 1),
    .poll_period = 100
};

/*
 * Configuration for the debugger CLI module
 */
struct fwk_module_config config_debugger_cli = {
    .get_element_table = NULL,
    .data = &debugger_cli_data
};
