/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_clock.h>
#include <morello_scp_pik.h>

#include <mod_css_clock.h>
#include <mod_pik_clock.h>
#include <mod_sc_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct mod_css_clock_rate rate_table_cpu_group_0[5] = {
    {
        /* Super Underdrive */
        .rate = CSS_CLK_RATE_CPU_GRP0_LEVEL1,
        .pll_rate = CSS_CLK_RATE_CPU_GRP0_LEVEL1,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Underdrive */
        .rate = CSS_CLK_RATE_CPU_GRP0_LEVEL2,
        .pll_rate = CSS_CLK_RATE_CPU_GRP0_LEVEL2,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Nominal */
        .rate = CSS_CLK_RATE_CPU_GRP0_LEVEL3,
        .pll_rate = CSS_CLK_RATE_CPU_GRP0_LEVEL3,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Overdrive */
        .rate = CSS_CLK_RATE_CPU_GRP0_LEVEL4,
        .pll_rate = CSS_CLK_RATE_CPU_GRP0_LEVEL4,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Super Overdrive */
        .rate = CSS_CLK_RATE_CPU_GRP0_LEVEL5,
        .pll_rate = CSS_CLK_RATE_CPU_GRP0_LEVEL5,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
};

static const struct mod_css_clock_rate rate_table_cpu_group_1[5] = {
    {
        /* Super Underdrive */
        .rate = CSS_CLK_RATE_CPU_GRP1_LEVEL1,
        .pll_rate = CSS_CLK_RATE_CPU_GRP1_LEVEL1,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Underdrive */
        .rate = CSS_CLK_RATE_CPU_GRP1_LEVEL2,
        .pll_rate = CSS_CLK_RATE_CPU_GRP1_LEVEL2,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Nominal */
        .rate = CSS_CLK_RATE_CPU_GRP1_LEVEL3,
        .pll_rate = CSS_CLK_RATE_CPU_GRP1_LEVEL3,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Overdrive */
        .rate = CSS_CLK_RATE_CPU_GRP1_LEVEL4,
        .pll_rate = CSS_CLK_RATE_CPU_GRP1_LEVEL4,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Super Overdrive */
        .rate = CSS_CLK_RATE_CPU_GRP1_LEVEL5,
        .pll_rate = CSS_CLK_RATE_CPU_GRP1_LEVEL5,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
};

static const struct mod_css_clock_rate rate_table_gpu[5] = {
    {
        .rate = CSS_CLK_RATE_GPU_LEVEL1,
        .pll_rate = CSS_CLK_RATE_GPU_LEVEL1,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
    {
        .rate = CSS_CLK_RATE_GPU_LEVEL2,
        .pll_rate = CSS_CLK_RATE_GPU_LEVEL2,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
    {
        .rate = CSS_CLK_RATE_GPU_LEVEL3,
        .pll_rate = CSS_CLK_RATE_GPU_LEVEL3,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
    {
        .rate = CSS_CLK_RATE_GPU_LEVEL4,
        .pll_rate = CSS_CLK_RATE_GPU_LEVEL4,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
    {
        /* Nominal */
        .rate = CSS_CLK_RATE_GPU_LEVEL5,
        .pll_rate = CSS_CLK_RATE_GPU_LEVEL5,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
};

static const fwk_id_t member_table_cpu_group_0[2] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS0_CPU0),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS0_CPU1),
};

static const fwk_id_t member_table_cpu_group_1[2] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS1_CPU0),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_CLUS1_CPU1),
};

static const fwk_id_t member_table_gpu[1] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_GPU),
};

static const fwk_id_t member_table_dpu[1] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_DPU),
};

static const struct fwk_element
                        css_clock_element_table[CLOCK_CSS_IDX_COUNT + 1] = {
    [CLOCK_CSS_IDX_CPU_GROUP0] = {
        .name = "CPU_GROUP_0",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group_0,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_0),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SC_PLL,
                CLOCK_PLL_IDX_CPU0),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SC_PLL,
                MOD_SC_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_0,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_0),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = CSS_CLK_RATE_CPU_GRP0_LEVEL5,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_CPU_GROUP1] = {
        .name = "CPU_GROUP_1",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group_1,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_1),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SC_PLL,
                CLOCK_PLL_IDX_CPU1),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SC_PLL,
                MOD_SC_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_cpu_group_1,
            .member_count = FWK_ARRAY_SIZE(member_table_cpu_group_1),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = CSS_CLK_RATE_CPU_GRP1_LEVEL5,
            .modulation_supported = true,
        }),
    },
    [CLOCK_CSS_IDX_GPU] = {
        .name = "GPU",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_gpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_gpu),
            .clock_switching_source = MOD_PIK_CLOCK_GPUCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SC_PLL,
                                          CLOCK_PLL_IDX_GPU),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SC_PLL,
                                          MOD_SC_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_gpu,
            .member_count = FWK_ARRAY_SIZE(member_table_gpu),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                             MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = CSS_CLK_RATE_GPU_LEVEL3,
            .modulation_supported = false,
        }),
    },
    [CLOCK_CSS_IDX_DPU] = {
        .name = "DPU",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_NON_INDEXED,
            .clock_default_source = MOD_PIK_CLOCK_ACLKDPU_SOURCE_DISPLAYPLLCLK,
            .clock_switching_source = MOD_PIK_CLOCK_ACLKDPU_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SC_PLL,
                                          CLOCK_PLL_IDX_DPU),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SC_PLL,
                                          MOD_SC_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_dpu,
            .member_count = FWK_ARRAY_SIZE(member_table_dpu),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                             MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = PIK_CLK_RATE_DPU,
            .modulation_supported = false,
        }),
    },
    [CLOCK_CSS_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *css_clock_get_element_table(fwk_id_t module_id)
{
    return css_clock_element_table;
}

const struct fwk_module_config config_css_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(css_clock_get_element_table),
};
