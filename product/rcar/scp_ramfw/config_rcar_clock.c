/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <system_clock.h>

#include <mod_rcar_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

/*
 * Rate lookup tables
 */

static struct mod_rcar_clock_rate rate_table_cpu_a53[] = {
    {
        .rate = 800 * FWK_MHZ,
        .source = MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL2,
        .divider_reg = MOD_RCAR_CLOCK_A53_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
    {
        .rate = 1000 * FWK_MHZ,
        .source = MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL2,
        .divider_reg = MOD_RCAR_CLOCK_A53_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
    {
        .rate = 1200 * FWK_MHZ,
        .source = MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL2,
        .divider_reg = MOD_RCAR_CLOCK_A53_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static struct mod_rcar_clock_rate rate_table_cpu_a57[] = {
    {
        .rate = 500 * FWK_MHZ,
        .source = MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_RCAR_CLOCK_A57_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
    {
        .rate = 1000 * FWK_MHZ,
        .source = MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_RCAR_CLOCK_A57_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
    {
        .rate = 1500 * FWK_MHZ,
        .source = MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_RCAR_CLOCK_A57_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
    {
        .rate = 1600 * FWK_MHZ,
        .source = MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_RCAR_CLOCK_A57_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
    {
        .rate = 1700 * FWK_MHZ,
        .source = MOD_RCAR_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_RCAR_CLOCK_A57_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static const struct fwk_element rcar_clock_element_table[] = {
    /*
     * A53 CPUS
     */
    {
        .name = "CLUS0_CPU0",
        .data = &((struct mod_rcar_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_CLUSTER,
            .is_group_member = false,
            .rate_table = rate_table_cpu_a53,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a53),
            .initial_rate = 1200 * FWK_MHZ,
            .defer_initialization = false,
        }),
    },
    /*
     * A57 CPUS
     */
    {
        .name = "CLUS0_CPU4",
        .data = &((struct mod_rcar_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_CLUSTER,
            .is_group_member = false,
            .rate_table = rate_table_cpu_a57,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a57),
            .initial_rate = 1500 * FWK_MHZ,
            .defer_initialization = false,
        }),
    },
    /*
     * VPU
     */
    {
        .name = "VPU",
        .data = &((struct mod_rcar_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = true,
            .initial_rate = 600 * FWK_MHZ,
            .defer_initialization = false,
        }),
    },
    /*
     * DPU
     */
    {
        .name = "ACLKDP",
        .data = &((struct mod_rcar_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = true,
            .initial_rate = (CLOCK_RATE_SYSPLLCLK / 3),
            .defer_initialization = false,
        }),
    },
    {
        .name = "DPU_M0",
        .data = &((struct mod_rcar_clock_dev_config){
            .type = MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = true,
            .initial_rate = 260 * FWK_MHZ,
            .defer_initialization = false,
        }),
    },
    {}, /* Termination description. */
};

static const struct fwk_element *rcar_clock_get_element_table(
    fwk_id_t module_id)
{
    return rcar_clock_element_table;
}

struct fwk_module_config config_rcar_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(rcar_clock_get_element_table),
    .data = &((struct mod_ext_clock_rate){
        .ext_clk_rate = PLL_BASE_CLOCK,
    }),
};
