/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'system_pll'.
 */

#include "scp_clock.h"
#include "scp_exp_mmap.h"
#include "scp_pwrctrl.h"

#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

/* Module 'system_pll' element count */
#define MOD_SYSTEM_PLL_ELEMENT_COUNT (CFGD_MOD_SYSTEM_PLL_EIDX_COUNT + 1)

static const struct fwk_element sys_pll_table[MOD_SYSTEM_PLL_ELEMENT_COUNT] = {
    [CFGD_MOD_SYSTEM_PLL_EIDX_SYS] = {
        .name = "SYS_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_SYSPLL,
            .status_reg = (void *)SCP_PLL_STATUS0,
            .lock_flag_mask = PLL_STATUS_0_SYSPLL_LOCK,
            .initial_rate = 2000 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CFGD_MOD_SYSTEM_PLL_EIDX_INTERCONNECT] = {
        .name = "INT_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_INTERCONNECT,
            .status_reg = (void *)SCP_PLL_STATUS0,
            .lock_flag_mask = PLL_STATUS_0_INTPLL_LOCK,
            .initial_rate = 2000 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CFGD_MOD_SYSTEM_PLL_EIDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *system_pll_get_element_table(
    fwk_id_t module_id)
{
    return sys_pll_table;
}

const struct fwk_module_config config_system_pll = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(system_pll_get_element_table),
};
