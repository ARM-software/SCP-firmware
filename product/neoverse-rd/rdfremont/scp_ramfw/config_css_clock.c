/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'css_clock'.
 */

#include "scp_clock.h"

#include <mod_css_clock.h>
#include <mod_pik_clock.h>
#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/* Module 'css_clock' element count */
#define MOD_CSS_CLOCK_ELEMENT_COUNT (CFGD_MOD_CSS_CLOCK_EIDX_COUNT + 1)

/* CPU clock rate table count */
#define CPU_RATE_TABLE_COUNT 3

/*
 * Helper macro to instantiate a member table that lists the clocks included in
 * a CPU clock device.
 */
#define CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(n) \
    static const fwk_id_t css_clock_dev_member_table_cpu##n[] = { \
        FWK_ID_ELEMENT_INIT( \
            FWK_MODULE_IDX_PIK_CLOCK, CFGD_MOD_PIK_CLOCK_EIDX_CPU##n), \
    }

/*
 * Helper macro to instantiate a CPU clock device.
 */
#define CSS_CLOCK_DEV_CPU(n) \
    [CFGD_MOD_CSS_CLOCK_EIDX_CPU##n] = { \
        .name = "CPU_" #n, \
        .data = &((struct mod_css_clock_dev_config){ \
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED, \
            .rate_table = rate_table_cpu, \
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu), \
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK, \
            .pll_id = FWK_ID_ELEMENT_INIT( \
                FWK_MODULE_IDX_SYSTEM_PLL, CFGD_MOD_SYSTEM_PLL_EIDX_CPU##n), \
            .pll_api_id = FWK_ID_API_INIT( \
                FWK_MODULE_IDX_SYSTEM_PLL, MOD_SYSTEM_PLL_API_TYPE_DEFAULT), \
            .member_table = css_clock_dev_member_table_cpu##n, \
            .member_count = FWK_ARRAY_SIZE(css_clock_dev_member_table_cpu##n), \
            .member_api_id = FWK_ID_API_INIT( \
                FWK_MODULE_IDX_PIK_CLOCK, MOD_PIK_CLOCK_API_TYPE_CSS), \
            .initial_rate = 2600 * FWK_MHZ, \
            .modulation_supported = true, \
        }), \
    }

/*
 * List of application core clock speeds
 */
static const struct mod_css_clock_rate rate_table_cpu[CPU_RATE_TABLE_COUNT] = {
    {
        .rate = 2300 * FWK_MHZ,
        .pll_rate = 2300 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        .rate = 2600 * FWK_MHZ,
        .pll_rate = 2600 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        .rate = 3200 * FWK_MHZ,
        .pll_rate = 3200 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
};

/*
 * Instantiate clock device member table for all the application core clocks.
 */
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(0);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(1);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(2);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(3);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(4);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(5);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(6);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(7);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(8);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(9);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(10);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(11);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(12);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(13);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(14);
CSS_CLOCK_DEV_MEMBER_TABLE_CPUCLK(15);

/*
 * Instantiate 'css_clock' module elements.
 */
static const struct fwk_element
    css_clock_element_table[MOD_CSS_CLOCK_ELEMENT_COUNT] = {
        CSS_CLOCK_DEV_CPU(0),
        CSS_CLOCK_DEV_CPU(1),
        CSS_CLOCK_DEV_CPU(2),
        CSS_CLOCK_DEV_CPU(3),
        CSS_CLOCK_DEV_CPU(4),
        CSS_CLOCK_DEV_CPU(5),
        CSS_CLOCK_DEV_CPU(6),
        CSS_CLOCK_DEV_CPU(7),
        CSS_CLOCK_DEV_CPU(8),
        CSS_CLOCK_DEV_CPU(9),
        CSS_CLOCK_DEV_CPU(10),
        CSS_CLOCK_DEV_CPU(11),
        CSS_CLOCK_DEV_CPU(12),
        CSS_CLOCK_DEV_CPU(13),
        CSS_CLOCK_DEV_CPU(14),
        CSS_CLOCK_DEV_CPU(15),
        [CFGD_MOD_CSS_CLOCK_EIDX_COUNT] = { 0 },
    };

static const struct fwk_element *css_clock_get_element_table(fwk_id_t module_id)
{
    return css_clock_element_table;
}

const struct fwk_module_config config_css_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(css_clock_get_element_table),
};
