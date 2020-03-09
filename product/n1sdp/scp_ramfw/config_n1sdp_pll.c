/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "n1sdp_scp_mmap.h"
#include "n1sdp_system_clock.h"

#include <mod_n1sdp_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

static struct n1sdp_pll_custom_freq_param_entry freq_table[] = {
    {
        .freq_value_mhz = 1333,
        .fbdiv = 160,
        .refdiv = 3,
        .postdiv = 2,
    },
};

static const struct fwk_element n1sdp_pll_element_table[] = {
    [CLOCK_PLL_IDX_CPU0] = {
        .name = "CPU_PLL_0",
        .data = &((struct mod_n1sdp_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_CPU0_CTRL,
            .control_reg1 = (void *)SCP_PLL_CPU0_STAT,
            .initial_rate = N1SDP_PLL_RATE_CPU_PLL0,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_CPU1] = {
        .name = "CPU_PLL_1",
        .data = &((struct mod_n1sdp_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_CPU1_CTRL,
            .control_reg1 = (void *)SCP_PLL_CPU1_STAT,
            .initial_rate = N1SDP_PLL_RATE_CPU_PLL1,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_CLUS] = {
        .name = "CLUSTER_PLL",
        .data = &((struct mod_n1sdp_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_CLUS_CTRL,
            .control_reg1 = (void *)SCP_PLL_CLUS_STAT,
            .initial_rate = N1SDP_PLL_RATE_CLUSTER_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_INTERCONNECT] = {
        .name = "INT_PLL",
        .data = &((struct mod_n1sdp_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_INTERCONNECT_CTRL,
            .control_reg1 = (void *)SCP_PLL_INTERCONNECT_STAT,
            .initial_rate = N1SDP_PLL_RATE_INTERCONNECT_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_SYS] = {
        .name = "SYS_PLL",
        .data = &((struct mod_n1sdp_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_SYSPLL_CTRL,
            .control_reg1 = (void *)SCP_PLL_SYSPLL_STAT,
            .initial_rate = N1SDP_PLL_RATE_SYSTEM_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_DMC] = {
        .name = "DMC_PLL",
        .data = &((struct mod_n1sdp_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_DMC_CTRL,
            .control_reg1 = (void *)SCP_PLL_DMC_STAT,
            .initial_rate = N1SDP_PLL_RATE_DMC_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *n1sdp_pll_get_element_table
    (fwk_id_t module_id)
{
    return n1sdp_pll_element_table;
}

const struct fwk_module_config config_n1sdp_pll = {
    .get_element_table = n1sdp_pll_get_element_table,
    .data = &((struct n1sdp_pll_module_config) {
        .custom_freq_table = freq_table,
        .custom_freq_table_size = FWK_ARRAY_SIZE(freq_table),
    }),
};
