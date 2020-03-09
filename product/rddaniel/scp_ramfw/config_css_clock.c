/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
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

static const fwk_id_t member_table_cpu_group_0[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS0_CPU0),
};

static const fwk_id_t member_table_cpu_group_1[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS1_CPU0),
};

static const fwk_id_t member_table_cpu_group_2[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS2_CPU0),
};

static const fwk_id_t member_table_cpu_group_3[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS3_CPU0),
};

static const fwk_id_t member_table_cpu_group_4[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS4_CPU0),
};

static const fwk_id_t member_table_cpu_group_5[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS5_CPU0),
};

static const fwk_id_t member_table_cpu_group_6[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS6_CPU0),
};

static const fwk_id_t member_table_cpu_group_7[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS7_CPU0),
};

static const fwk_id_t member_table_cpu_group_8[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS8_CPU0),
};

static const fwk_id_t member_table_cpu_group_9[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS9_CPU0),
};

static const fwk_id_t member_table_cpu_group_10[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS10_CPU0),
};

static const fwk_id_t member_table_cpu_group_11[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS11_CPU0),
};

static const fwk_id_t member_table_cpu_group_12[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS12_CPU0),
};

static const fwk_id_t member_table_cpu_group_13[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS13_CPU0),
};

static const fwk_id_t member_table_cpu_group_14[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS14_CPU0),
};

static const fwk_id_t member_table_cpu_group_15[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS15_CPU0),
};

static const struct fwk_element css_clock_element_table[] = {
    [CLOCK_CSS_IDX_CPU_GROUP0] = {
        .name = "CPU_GROUP_0",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU0),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_0,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_0),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP1] = {
        .name = "CPU_GROUP_1",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU1),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_1,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_1),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
     [CLOCK_CSS_IDX_CPU_GROUP2] = {
        .name = "CPU_GROUP_2",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU2),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_2,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_2),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP3] = {
        .name = "CPU_GROUP_3",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU3),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_3,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_3),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP4] = {
        .name = "CPU_GROUP_4",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU4),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_4,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_4),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP5] = {
        .name = "CPU_GROUP_5",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU5),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_5,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_5),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP6] = {
        .name = "CPU_GROUP_6",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU6),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_6,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_6),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP7] = {
        .name = "CPU_GROUP_7",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU7),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_7,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_7),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP8] = {
        .name = "CPU_GROUP_8",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU8),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_8,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_8),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP9] = {
        .name = "CPU_GROUP_9",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU9),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_9,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_9),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP10] = {
        .name = "CPU_GROUP_10",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU10),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_10,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_10),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP11] = {
        .name = "CPU_GROUP_11",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU11),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_11,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_11),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP12] = {
        .name = "CPU_GROUP_12",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU12),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_12,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_12),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP13] = {
        .name = "CPU_GROUP_13",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU13),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_13,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_13),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP14] = {
        .name = "CPU_GROUP_14",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU14),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_14,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_14),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP15] = {
        .name = "CPU_GROUP_15",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                CLOCK_PLL_IDX_CPU15),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_15,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_15),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2600 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *css_clock_get_element_table
    (fwk_id_t module_id)
{
    return css_clock_element_table;
}

const struct fwk_module_config config_css_clock = {
    .get_element_table = css_clock_get_element_table,
};
