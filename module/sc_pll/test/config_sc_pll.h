/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/sc_pll.h>

#include <mod_sc_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#define SCP_PLL_CPU0_CTRL    0x00000120
#define SCP_PLL_CPU0_STAT    0x00000120
#define SC_PLL_RATE_CPU_PLL0 (4000 / 3) * FWK_MHZ
#define CLOCK_RATE_REFCLK    50 * FWK_MHZ

volatile uint32_t control_reg0 = 0x40000000;
volatile uint32_t control_reg1 = 0x80000000;
volatile uint32_t control_reg1_timeout = 0x70000000;

static struct mod_sc_pll_dev_param dev_param = {
    .postdiv1_min = 1,
    .postdiv1_max = 7,
    .postdiv2_min = 1,
    .postdiv2_max = 1,
    .pll_rate_min = 50 * FWK_MHZ,
    .pll_rate_max = 3200 * FWK_MHZ,
};

static const struct fwk_element
                        sc_pll_element_table[2] = {
    [0] = {
        .name = "CPU_PLL_0",
        .data = &((struct mod_sc_pll_dev_config) {
            .control_reg0 = &control_reg0,
            .control_reg1 = &control_reg1,
            .initial_rate = SC_PLL_RATE_CPU_PLL0,
            .ref_rate = CLOCK_RATE_REFCLK,
            .dev_param = &dev_param,
        }),
    },
    [1] = {0},
};

static const struct fwk_element *sc_pll_get_element_table(fwk_id_t module_id)
{
    return sc_pll_element_table;
}

const struct fwk_module_config config_sc_pll = {
    .data = &((struct mod_sc_pll_dev_param){
        .postdiv1_min = 1,
        .postdiv1_max = 7,
        .postdiv2_min = 1,
        .postdiv2_max = 1,
        .pll_rate_min = 50 * FWK_MHZ,
        .pll_rate_max = 3200 * FWK_MHZ,
    }),
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(sc_pll_get_element_table),
};
