/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"

#include <mod_css_clock.h>
#include <mod_pik_clock.h>
#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>

#define MEMBER_TABLE_CPU_GROUP(n) \
    static const fwk_id_t member_table_cpu_group_##n[] = { \
        FWK_ID_ELEMENT_INIT( \
            FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS##n##_CPU0), \
    }

#define CLOCK_CSS_CPU_GROUP(n) \
    [CLOCK_CSS_IDX_CPU_GROUP##n] = { \
        .name = "CPU_GROUP_" #n, \
        .data = &((struct mod_css_clock_dev_config){ \
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED, \
            .rate_table = rate_table_cpu_group, \
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group), \
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK, \
            .pll_id = FWK_ID_ELEMENT_INIT( \
                FWK_MODULE_IDX_SYSTEM_PLL, CLOCK_PLL_IDX_CPU##n), \
            .pll_api_id = FWK_ID_API_INIT( \
                FWK_MODULE_IDX_SYSTEM_PLL, MOD_SYSTEM_PLL_API_TYPE_DEFAULT), \
            .member_table = member_table_cpu_group_##n, \
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_##n), \
            .member_api_id = FWK_ID_API_INIT( \
                FWK_MODULE_IDX_PIK_CLOCK, MOD_PIK_CLOCK_API_TYPE_CSS), \
            .initial_rate = 2600 * FWK_MHZ, \
            .modulation_supported = true, \
        }), \
    }

static const struct mod_css_clock_rate rate_table_cpu_group[] = {
    {
        /* Super Underdrive */
        .rate = 1313 * FWK_MHZ,
        .pll_rate = 1313 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Underdrive */
        .rate = 1531 * FWK_MHZ,
        .pll_rate = 1531 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Nominal */
        .rate = 1750 * FWK_MHZ,
        .pll_rate = 1750 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Overdrive */
        .rate = 2100 * FWK_MHZ,
        .pll_rate = 2100 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Super Overdrive */
        .rate = 2600 * FWK_MHZ,
        .pll_rate = 2600 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
};

MEMBER_TABLE_CPU_GROUP(0);
MEMBER_TABLE_CPU_GROUP(1);
MEMBER_TABLE_CPU_GROUP(2);
MEMBER_TABLE_CPU_GROUP(3);
MEMBER_TABLE_CPU_GROUP(4);
MEMBER_TABLE_CPU_GROUP(5);
MEMBER_TABLE_CPU_GROUP(6);
MEMBER_TABLE_CPU_GROUP(7);
MEMBER_TABLE_CPU_GROUP(8);
MEMBER_TABLE_CPU_GROUP(9);
MEMBER_TABLE_CPU_GROUP(10);
MEMBER_TABLE_CPU_GROUP(11);
MEMBER_TABLE_CPU_GROUP(12);
MEMBER_TABLE_CPU_GROUP(13);
MEMBER_TABLE_CPU_GROUP(14);
MEMBER_TABLE_CPU_GROUP(15);

static const struct fwk_element css_clock_element_table[] = {
    CLOCK_CSS_CPU_GROUP(0),        CLOCK_CSS_CPU_GROUP(1),
    CLOCK_CSS_CPU_GROUP(2),        CLOCK_CSS_CPU_GROUP(3),
    CLOCK_CSS_CPU_GROUP(4),        CLOCK_CSS_CPU_GROUP(5),
    CLOCK_CSS_CPU_GROUP(6),        CLOCK_CSS_CPU_GROUP(7),
    CLOCK_CSS_CPU_GROUP(8),        CLOCK_CSS_CPU_GROUP(9),
    CLOCK_CSS_CPU_GROUP(10),       CLOCK_CSS_CPU_GROUP(11),
    CLOCK_CSS_CPU_GROUP(12),       CLOCK_CSS_CPU_GROUP(13),
    CLOCK_CSS_CPU_GROUP(14),       CLOCK_CSS_CPU_GROUP(15),
    [CLOCK_CSS_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *css_clock_get_element_table(fwk_id_t module_id)
{
    return css_clock_element_table;
}

const struct fwk_module_config config_css_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(css_clock_get_element_table),
};
