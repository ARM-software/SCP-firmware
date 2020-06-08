/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_css_clock.h>
#include <mod_pik_clock.h>
#include <mod_sid.h>
#include <mod_system_pll.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

static const struct mod_css_clock_rate rate_table_cpu_group_big[] = {
    {
        /* Super Underdrive */
        .rate = 1313 * FWK_MHZ,
        .pll_rate = 1313 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Underdrive */
        .rate = 1531 * FWK_MHZ,
        .pll_rate = 1531 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Nominal */
        .rate = 1750 * FWK_MHZ,
        .pll_rate = 1750 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Overdrive */
        .rate = 2100 * FWK_MHZ,
        .pll_rate = 2100 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Super Overdrive */
        .rate = 2700 * FWK_MHZ,
        .pll_rate = 2700 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
};

static const struct mod_css_clock_rate rate_table_cpu_group_little[] = {
    {
        /* Super Underdrive */
        .rate = 665 * FWK_MHZ,
        .pll_rate = 665 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Underdrive */
        .rate = 998 * FWK_MHZ,
        .pll_rate = 998 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Nominal */
        .rate = 1330 * FWK_MHZ,
        .pll_rate = 1330 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Overdrive */
        .rate = 1463 * FWK_MHZ,
        .pll_rate = 1463 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
    {
        /* Super Overdrive */
        .rate = 2200 * FWK_MHZ,
        .pll_rate = 2200 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
        .clock_mod_numerator = 1,
        .clock_mod_denominator = 1,
    },
};

static const struct mod_css_clock_rate rate_table_gpu[] = {
    {
        .rate = 450 * FWK_MHZ,
        .pll_rate = 450 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
    {
        .rate = 487500 * FWK_KHZ,
        .pll_rate = 487500 * FWK_KHZ,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
    {
        .rate = 525 * FWK_MHZ,
        .pll_rate = 525 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
    {
        .rate = 562500 * FWK_KHZ,
        .pll_rate = 562500 * FWK_KHZ,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
    {
        /* Nominal */
        .rate = 800 * FWK_MHZ,
        .pll_rate = 800 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
};

static const struct mod_css_clock_rate rate_table_vpu[] = {
    {
        /* Nominal */
        .rate = 650 * FWK_MHZ,
        .pll_rate = 650 * FWK_MHZ,
        .clock_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .clock_div_type = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .clock_div = 1,
    },
};

static const fwk_id_t member_table_cpu_big_cfg_a[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 6),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 7),
};

static const fwk_id_t member_table_cpu_little_cfg_a[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 0),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 1),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 2),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 3),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 4),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 5),
};

static const fwk_id_t member_table_cpu_big_cfg_b[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 4),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 5),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 6),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 7),
};

static const fwk_id_t member_table_cpu_little_cfg_b[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 0),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 1),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 2),
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 3),
};

static const fwk_id_t member_table_gpu[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 8),
};

static const fwk_id_t member_table_vpu[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 9),
};

static const fwk_id_t member_table_dpu[] = {
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 10),
};

static const struct fwk_element css_clock_element_table[] = {
    {
        .name = "CPU_GROUP_BIG",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group_big,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_big),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 1),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                          MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                             MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2700 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    {
        .name = "CPU_GROUP_LITTLE",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_cpu_group_little,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_little),
            .clock_switching_source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 0),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                          MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                             MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 2200 * FWK_MHZ,
            .modulation_supported = true,
        }),
    },
    {
        .name = "GPU",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_gpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_gpu),
            .clock_switching_source = MOD_PIK_CLOCK_GPUCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 2),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                          MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_gpu,
            .member_count = FWK_ARRAY_SIZE(member_table_gpu),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                             MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 800 * FWK_MHZ,
            .modulation_supported = false,
        }),
    },
    {
        .name = "VPU",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_INDEXED,
            .rate_table = rate_table_vpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_vpu),
            .clock_switching_source = MOD_PIK_CLOCK_VPUCLK_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 4),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                          MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_vpu,
            .member_count = FWK_ARRAY_SIZE(member_table_vpu),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                             MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 650 * FWK_MHZ,
            .modulation_supported = false,
        }),
    },
    {
        .name = "DPU",
        .data = &((struct mod_css_clock_dev_config) {
            .clock_type = MOD_CSS_CLOCK_TYPE_NON_INDEXED,
            .clock_default_source = MOD_PIK_CLOCK_ACLKDPU_SOURCE_DISPLAYPLLCLK,
            .clock_switching_source = MOD_PIK_CLOCK_ACLKDPU_SOURCE_SYSREFCLK,
            .pll_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 3),
            .pll_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                          MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .member_table = member_table_dpu,
            .member_count = FWK_ARRAY_SIZE(member_table_dpu),
            .member_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                             MOD_PIK_CLOCK_API_TYPE_CSS),
            .initial_rate = 600 * FWK_MHZ,
            .modulation_supported = false,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *css_clock_get_element_table
    (fwk_id_t module_id)
{
    int status;
    struct mod_css_clock_dev_config *config_big;
    struct mod_css_clock_dev_config *config_little;
    const struct mod_sid_info *system_info;

    status = mod_sid_get_system_info(&system_info);
    fwk_assert(status == FWK_SUCCESS);

    config_big =
        (struct mod_css_clock_dev_config *) css_clock_element_table[0].data;
    config_little =
        (struct mod_css_clock_dev_config *) css_clock_element_table[1].data;

    switch (system_info->config_number) {
    case 1:
    case 5:
    case 7:
    case 8:
        config_big->member_table = member_table_cpu_big_cfg_a;
        config_big->member_count = FWK_ARRAY_SIZE(member_table_cpu_big_cfg_a);

        config_little->member_table = member_table_cpu_little_cfg_a;
        config_little->member_count =
            FWK_ARRAY_SIZE(member_table_cpu_little_cfg_a);
        break;
    case 2:
    case 3:
    case 4:
    case 6:
        config_big->member_table = member_table_cpu_big_cfg_b;
        config_big->member_count =
            FWK_ARRAY_SIZE(member_table_cpu_big_cfg_b);

        config_little->member_table = member_table_cpu_little_cfg_b;
        config_little->member_count =
            FWK_ARRAY_SIZE(member_table_cpu_little_cfg_b);
        break;
    default:
        return NULL;
    }

    return css_clock_element_table;
}

const struct fwk_module_config config_css_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(css_clock_get_element_table),
};
