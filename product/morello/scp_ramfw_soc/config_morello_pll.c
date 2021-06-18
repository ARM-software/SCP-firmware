/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_clock.h>
#include <morello_scp_mmap.h>
#include <morello_system_clock.h>

#include <mod_morello_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

static const struct fwk_element
                        morello_pll_element_table[CLOCK_PLL_IDX_COUNT + 1] = {
    [CLOCK_PLL_IDX_CPU0] = {
        .name = "CPU_PLL_0",
        .data = &((struct mod_morello_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_CPU0_CTRL,
            .control_reg1 = (void *)SCP_PLL_CPU0_STAT,
            .initial_rate = MORELLO_PLL_RATE_CPU_PLL0,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_CPU1] = {
        .name = "CPU_PLL_1",
        .data = &((struct mod_morello_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_CPU1_CTRL,
            .control_reg1 = (void *)SCP_PLL_CPU1_STAT,
            .initial_rate = MORELLO_PLL_RATE_CPU_PLL1,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_CLUS] = {
        .name = "CLUSTER_PLL",
        .data = &((struct mod_morello_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_CLUS_CTRL,
            .control_reg1 = (void *)SCP_PLL_CLUS_STAT,
            .initial_rate = MORELLO_PLL_RATE_CLUSTER_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_INTERCONNECT] = {
        .name = "INT_PLL",
        .data = &((struct mod_morello_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_INTERCONNECT_CTRL,
            .control_reg1 = (void *)SCP_PLL_INTERCONNECT_STAT,
            .initial_rate = MORELLO_PLL_RATE_INTERCONNECT_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_SYS] = {
        .name = "SYS_PLL",
        .data = &((struct mod_morello_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_SYSPLL_CTRL,
            .control_reg1 = (void *)SCP_PLL_SYSPLL_STAT,
            .initial_rate = MORELLO_PLL_RATE_SYSTEM_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_DMC] = {
        .name = "DMC_PLL",
        .data = &((struct mod_morello_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_DMC_CTRL,
            .control_reg1 = (void *)SCP_PLL_DMC_STAT,
            .initial_rate = MORELLO_PLL_RATE_DMC_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_GPU] = {
        .name = "GPU_PLL",
        .data = &((struct mod_morello_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_GPU_CTRL,
            .control_reg1 = (void *)SCP_PLL_GPU_STAT,
            .initial_rate = MORELLO_PLL_RATE_GPU_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_DPU] = {
        .name = "DPU_PLL",
        .data = &((struct mod_morello_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_DPU_CTRL,
            .control_reg1 = (void *)SCP_PLL_DPU_STAT,
            .initial_rate = MORELLO_PLL_RATE_DPU_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_PXL] = {
        .name = "PXL_PLL",
        .data = &((struct mod_morello_pll_dev_config) {
            .control_reg0 = (void *)SCP_PLL_PIXEL_CTRL,
            .control_reg1 = (void *)SCP_PLL_PIXEL_STAT,
            .initial_rate = MORELLO_PLL_RATE_PIXEL_PLL,
            .ref_rate = CLOCK_RATE_REFCLK,
        }),
    },
    [CLOCK_PLL_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *morello_pll_get_element_table(
    fwk_id_t module_id)
{
    return morello_pll_element_table;
}

const struct fwk_module_config config_morello_pll = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(morello_pll_get_element_table),
};
