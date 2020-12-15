/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "scp_pik.h"
#include "scp_soc_mmap.h"

#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#define CLOCK_PLL_IDX_CPU(n) \
    [CLOCK_PLL_IDX_CPU##n] = { \
        .name = "CPU_PLL_" #n, \
        .data = &((struct mod_system_pll_dev_config){ \
            .control_reg = (void *)SCP_PLL_CPU##n, \
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1], \
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(n), \
            .initial_rate = 2600 * FWK_MHZ, \
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE, \
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE, \
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL, \
        }), \
    }

static const struct fwk_element system_pll_element_table[] = {
    CLOCK_PLL_IDX_CPU(0),
    CLOCK_PLL_IDX_CPU(1),
    CLOCK_PLL_IDX_CPU(2),
    CLOCK_PLL_IDX_CPU(3),
    [CLOCK_PLL_IDX_SYS] = {
        .name = "SYS_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_SYSPLL,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[0],
            .lock_flag_mask = PLL_STATUS_0_SYSPLLLOCK,
            .initial_rate = 2000 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_DMC] = {
        .name = "DMC_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_DMC,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[0],
            .lock_flag_mask = PLL_STATUS_0_DDRPLLLOCK,
            .initial_rate = 1600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_INTERCONNECT] = {
        .name = "INT_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_INTERCONNECT,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[0],
            .lock_flag_mask = PLL_STATUS_0_INTPLLLOCK,
            .initial_rate = 2000 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *system_pll_get_element_table(
    fwk_id_t module_id)
{
    return system_pll_element_table;
}

const struct fwk_module_config config_system_pll = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(system_pll_get_element_table),
};
