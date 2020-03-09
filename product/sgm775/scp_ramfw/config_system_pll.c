/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm775_pik.h"
#include "sgm775_pik_scp.h"
#include "system_mmap.h"

#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stddef.h>

static const struct fwk_element system_pll_element_table[] = {
    {
        .name = "CPU_PLL_0",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)PLL_CLUS0_0,
            .status_reg = (void *)&PIK_SCP->PLL_STATUS1,
            .lock_flag_mask = PLL_STATUS1_CPUPLLLOCK(0, 0),
            .initial_rate = 1330 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "CPU_PLL_1",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)PLL_CLUS0_1,
            .status_reg = (void *)&PIK_SCP->PLL_STATUS1,
            .lock_flag_mask = PLL_STATUS1_CPUPLLLOCK(0, 1),
            .initial_rate = 1750 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "GPU_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)PLL_GPU,
            .status_reg = (void *)&PIK_SCP->PLL_STATUS0,
            .lock_flag_mask = PLL_STATUS0_GPUPLLLOCK,
            .initial_rate = 600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "DPU_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)PLL_DISPLAY,
            .status_reg = (void *)&PIK_SCP->PLL_STATUS0,
            .lock_flag_mask = PLL_STATUS0_DISPLAYPLLLOCK,
            .initial_rate = 260 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "VPU_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)PLL_VIDEO,
            .status_reg = (void *)&PIK_SCP->PLL_STATUS0,
            .lock_flag_mask = PLL_STATUS0_VIDEOPLLLOCK,
            .initial_rate = 600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            .defer_initialization = false,
        }),
    },
    {
        .name = "PIX0_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)PIX0_CONTROL,
            .status_reg = NULL,
            .initial_rate = 594 * FWK_MHZ,
            .min_rate = 12500 * FWK_KHZ,
            .max_rate = 594 * FWK_MHZ,
            .min_step = 250 * FWK_KHZ,
            .defer_initialization = false,
        }),
    },
    {
        .name = "PIX1_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)PIX1_CONTROL,
            .status_reg = NULL,
            .initial_rate = 594 * FWK_MHZ,
            .min_rate = 12500 * FWK_KHZ,
            .max_rate = 594 * FWK_MHZ,
            .min_step = 250 * FWK_KHZ,
            .defer_initialization = false,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *system_pll_get_element_table
    (fwk_id_t module_id)
{
    return system_pll_element_table;
}

struct fwk_module_config config_system_pll = {
    .get_element_table = system_pll_get_element_table,
    .data = NULL,
};
