/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "n1sdp_mcp_pik.h"
#include "n1sdp_pik_mcp.h"
#include "n1sdp_system_clock.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>

/*
 * Rate lookup tables
 */

static const struct mod_pik_clock_rate rate_table_mcp_coreclk[] = {
    {
        .rate = PIK_CLK_RATE_MCP_CORECLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_MCP_CORECLK,
    },
};

static const struct mod_pik_clock_rate rate_table_mcp_axiclk[] = {
    {
        .rate = PIK_CLK_RATE_MCP_AXICLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / PIK_CLK_RATE_MCP_AXICLK,
    },
};

static const struct fwk_element pik_clock_element_table[] = {
    [CLOCK_PIK_IDX_MCP_CORECLK] = {
        .name = "MCP CORECLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_MCP->CORECLK_CTRL,
            .divsys_reg = &PIK_MCP->CORECLK_DIV1,
            .rate_table = rate_table_mcp_coreclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_mcp_coreclk),
            .initial_rate = PIK_CLK_RATE_MCP_CORECLK,
            .defer_initialization = true,
        }),
    },
    [CLOCK_PIK_IDX_MCP_AXICLK] = {
        .name = "MCP AXICLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_MCP->CORECLK_CTRL,
            .divsys_reg = &PIK_MCP->CORECLK_DIV1,
            .rate_table = rate_table_mcp_axiclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_mcp_axiclk),
            .initial_rate = PIK_CLK_RATE_MCP_AXICLK,
            .defer_initialization = true,
        }),
    },
    [CLOCK_PIK_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *pik_clock_get_element_table
    (fwk_id_t module_id)
{
    return pik_clock_element_table;
}

const struct fwk_module_config config_pik_clock = {
    .get_element_table = pik_clock_get_element_table,
};
