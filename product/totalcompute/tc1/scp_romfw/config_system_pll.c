/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "scp_mmap.h"
#include "scp_pik.h"

#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

static const struct fwk_element system_pll_element_table[2] = {
    [CLOCK_PLL_IDX_CPU_CORTEX_A510] = {
            .name = "CPU_PLL_CORTEX_A510",
            .data = &((struct mod_system_pll_dev_config){
                .control_reg = (void *)SCP_PLL_CPU0,
                .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
                .lock_flag_mask = PLL_STATUS_CPUPLL_LOCK(0),
                .initial_rate = 1537 * FWK_MHZ,
                .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
                .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
                .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
            }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *system_pll_get_element_table(
    fwk_id_t module_id)
{
    return system_pll_element_table;
}

const struct fwk_module_config config_system_pll = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(system_pll_get_element_table),
};
