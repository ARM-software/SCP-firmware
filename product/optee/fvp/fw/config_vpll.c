/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_system_pll.h>

#include <fwk_module.h>

uint32_t ctrl_reg[7];

static const struct fwk_element system_pll_element_table[] = {
    {
        .name = "CPU_PLL_0",
        .data = &((struct mod_system_pll_dev_config){
            .control_reg = &ctrl_reg[0],
            .initial_rate = 1330 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "CPU_PLL_1",
        .data = &((struct mod_system_pll_dev_config){
            .control_reg = &ctrl_reg[1],
            .initial_rate = 1750 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "GPU_PLL",
        .data = &((struct mod_system_pll_dev_config){
            .control_reg = &ctrl_reg[2],
            .initial_rate = 600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "DPU_PLL",
        .data = &((struct mod_system_pll_dev_config){
            .control_reg = &ctrl_reg[3],
            .initial_rate = 260 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "VPU_PLL",
        .data = &((struct mod_system_pll_dev_config){
            .control_reg = &ctrl_reg[4],
            .initial_rate = 600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "PIX0_PLL",
        .data = &((struct mod_system_pll_dev_config){
            .control_reg = &ctrl_reg[5],
            .initial_rate = 594 * FWK_MHZ,
            .min_rate = 12500 * FWK_KHZ,
            .max_rate = 594 * FWK_MHZ,
            .min_step = 250 * FWK_KHZ,
            .defer_initialization = false,
        }),
    },
    {
        .name = "PIX1_PLL",
        .data = &((struct mod_system_pll_dev_config){
            .control_reg = &ctrl_reg[6],
            .initial_rate = 594 * FWK_MHZ,
            .min_rate = 12500 * FWK_KHZ,
            .max_rate = 594 * FWK_MHZ,
            .min_step = 250 * FWK_KHZ,
            .defer_initialization = false,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *system_pll_get_element_table(
    fwk_id_t module_id)
{
    return system_pll_element_table;
}

struct fwk_module_config config_system_pll = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(system_pll_get_element_table),
};
