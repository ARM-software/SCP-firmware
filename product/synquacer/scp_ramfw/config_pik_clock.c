/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "pik_cpu.h"
#include "synquacer_mmap.h"
#include "system_clock.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stdint.h>

#define PIK_DEBUG_TRACECLK_CTRL UINT32_C(0x0810)
#define PIK_DEBUG_TRACECLK_DIV1 UINT32_C(0x0814)
#define PIK_DEBUG_PCLKDBG_CTRL UINT32_C(0x0820)
#define PIK_DEBUG_ATCLKDBG_CTRL UINT32_C(0x0830)
#define PIK_DEBUG_ATCLKDBG_DIV1 UINT32_C(0x0834)

/*
 * Rate lookup tables
 */
static const struct mod_pik_clock_rate rate_table_dbgtraceclk[] = {
    {
        .rate = 125 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 125 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_dbgatclk[] = {
    {
        .rate = 500 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 500 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_dbgpclk[] = {
    {
        .rate = 500 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 500 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_clus_cpuclk[] = {
    {
        .rate = 1000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 1000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_clus_atclkdbg[] = {
    {
        .rate = 500 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 500 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_clus_pclkdbg[] = {
    {
        .rate = 500 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 500 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_clus_aclkcpu[] = {
    {
        .rate = 1000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 1000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_clus_ppuclk[] = {
    {
        .rate = 125 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSREFCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 125 * FWK_MHZ),
    },
};

static const struct fwk_element pik_clock_element_table[] = {
    [CLOCK_PIK_IDX_CLUS0_CPUCLK] = {
        .name = "CLUS0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(0)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPUCLK_DIV2] = {
        .name = "CLUS0_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(0)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_ATCLKDBG] = {
        .name = "CLUS0_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_PCLKDBG] = {
        .name = "CLUS0_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_ACLKCPU] = {
        .name = "CLUS0_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_PPUCLK] = {
        .name = "CLUS0_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(0)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_PPUCLK_DIV2] = {
        .name = "CLUS0_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(0)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(0)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS1_CPUCLK] = {
        .name = "CLUS1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(1)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_CPUCLK_DIV2] = {
        .name = "CLUS1_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(1)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_ATCLKDBG] = {
        .name = "CLUS1_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_PCLKDBG] = {
        .name = "CLUS1_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_ACLKCPU] = {
        .name = "CLUS1_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_PPUCLK] = {
        .name = "CLUS1_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(1)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_PPUCLK_DIV2] = {
        .name = "CLUS1_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(1)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(1)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS2_CPUCLK] = {
        .name = "CLUS2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(2)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(2)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS2_CPUCLK_DIV2] = {
        .name = "CLUS2_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(2)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(2)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS2_ATCLKDBG] = {
        .name = "CLUS2_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(2)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS2_PCLKDBG] = {
        .name = "CLUS2_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(2)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS2_ACLKCPU] = {
        .name = "CLUS2_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(2)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS2_PPUCLK] = {
        .name = "CLUS2_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(2)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(2)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS2_PPUCLK_DIV2] = {
        .name = "CLUS2_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(2)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(2)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS3_CPUCLK] = {
        .name = "CLUS3",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(3)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(3)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS3_CPUCLK_DIV2] = {
        .name = "CLUS3_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(3)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(3)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS3_ATCLKDBG] = {
        .name = "CLUS3_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(3)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS3_PCLKDBG] = {
        .name = "CLUS3_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(3)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS3_ACLKCPU] = {
        .name = "CLUS3_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(3)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS3_PPUCLK] = {
        .name = "CLUS3_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(3)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(3)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS3_PPUCLK_DIV2] = {
        .name = "CLUS3_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(3)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(3)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS4_CPUCLK] = {
        .name = "CLUS4",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(4)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(4)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS4_CPUCLK_DIV2] = {
        .name = "CLUS4_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(4)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(4)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS4_ATCLKDBG] = {
        .name = "CLUS4_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(4)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS4_PCLKDBG] = {
        .name = "CLUS4_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(4)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS4_ACLKCPU] = {
        .name = "CLUS4_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(4)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS4_PPUCLK] = {
        .name = "CLUS4_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(4)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(4)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS4_PPUCLK_DIV2] = {
        .name = "CLUS4_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(4)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(4)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS5_CPUCLK] = {
        .name = "CLUS5",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(5)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(5)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS5_CPUCLK_DIV2] = {
        .name = "CLUS5_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(5)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(5)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS5_ATCLKDBG] = {
        .name = "CLUS5_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(5)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS5_PCLKDBG] = {
        .name = "CLUS5_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(5)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS5_ACLKCPU] = {
        .name = "CLUS5_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(5)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS5_PPUCLK] = {
        .name = "CLUS5_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(5)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(5)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS5_PPUCLK_DIV2] = {
        .name = "CLUS5_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(5)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(5)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS6_CPUCLK] = {
        .name = "CLUS6",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(6)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(6)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS6_CPUCLK_DIV2] = {
        .name = "CLUS6_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(6)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(6)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS6_ATCLKDBG] = {
        .name = "CLUS6_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(6)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS6_PCLKDBG] = {
        .name = "CLUS6_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(6)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS6_ACLKCPU] = {
        .name = "CLUS6_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(6)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS6_PPUCLK] = {
        .name = "CLUS6_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(6)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(6)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS6_PPUCLK_DIV2] = {
        .name = "CLUS6_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(6)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(6)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS7_CPUCLK] = {
        .name = "CLUS7",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(7)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(7)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS7_CPUCLK_DIV2] = {
        .name = "CLUS7_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(7)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(7)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS7_ATCLKDBG] = {
        .name = "CLUS7_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(7)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS7_PCLKDBG] = {
        .name = "CLUS7_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(7)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS7_ACLKCPU] = {
        .name = "CLUS7_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(7)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS7_PPUCLK] = {
        .name = "CLUS7_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(7)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(7)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS7_PPUCLK_DIV2] = {
        .name = "CLUS7_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(7)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(7)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS8_CPUCLK] = {
        .name = "CLUS8",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(8)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(8)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS8_CPUCLK_DIV2] = {
        .name = "CLUS8_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(8)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(8)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS8_ATCLKDBG] = {
        .name = "CLUS8_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(8)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS8_PCLKDBG] = {
        .name = "CLUS8_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(8)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS8_ACLKCPU] = {
        .name = "CLUS8_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(8)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS8_PPUCLK] = {
        .name = "CLUS8_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(8)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(8)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS8_PPUCLK_DIV2] = {
        .name = "CLUS8_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(8)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(8)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS9_CPUCLK] = {
        .name = "CLUS9",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(9)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(9)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS9_CPUCLK_DIV2] = {
        .name = "CLUS9_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(9)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(9)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS9_ATCLKDBG] = {
        .name = "CLUS9_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(9)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS9_PCLKDBG] = {
        .name = "CLUS9_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(9)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS9_ACLKCPU] = {
        .name = "CLUS9_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(9)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS9_PPUCLK] = {
        .name = "CLUS9_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(9)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(9)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS9_PPUCLK_DIV2] = {
        .name = "CLUS9_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(9)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(9)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS10_CPUCLK] = {
        .name = "CLUS10",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(10)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(10)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS10_CPUCLK_DIV2] = {
        .name = "CLUS10_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(10)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(10)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS10_ATCLKDBG] = {
        .name = "CLUS10_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(10)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS10_PCLKDBG] = {
        .name = "CLUS10_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(10)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS10_ACLKCPU] = {
        .name = "CLUS10_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(10)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS10_PPUCLK] = {
        .name = "CLUS10_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(10)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(10)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS10_PPUCLK_DIV2] = {
        .name = "CLUS10_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(10)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(10)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_CLUS11_CPUCLK] = {
        .name = "CLUS11",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(11)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(11)->CPUCLK_DIV1,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS11_CPUCLK_DIV2] = {
        .name = "CLUS11_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(11)->CPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(11)->CPUCLK_DIV2,
            .rate_table = rate_table_clus_cpuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_cpuclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS11_ATCLKDBG] = {
        .name = "CLUS11_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(11)->ATCLKDBG_CTRL,
            .rate_table = rate_table_clus_atclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_atclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS11_PCLKDBG] = {
        .name = "CLUS11_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(11)->PCLKDBG_CTRL,
            .rate_table = rate_table_clus_pclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_pclkdbg),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS11_ACLKCPU] = {
        .name = "CLUS11_ACLKCPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(11)->ACLKCPU_CTRL,
            .rate_table = rate_table_clus_aclkcpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_aclkcpu),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS11_PPUCLK] = {
        .name = "CLUS11_PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(11)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(11)->PPUCLK_DIV1,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CLUS11_PPUCLK_DIV2] = {
        .name = "CLUS11_PPUCLK_DIV2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_CLUSTER(11)->PPUCLK_CTRL,
            .divsys_reg = &PIK_CLUSTER(11)->PPUCLK_DIV2,
            .rate_table = rate_table_clus_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_clus_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },


    [CLOCK_PIK_IDX_DEBUG_TRACECLK] = {
        .name = "DEBUG_TRACECLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg =
              (volatile uint32_t *)(PIK_DEBUG_BASE + PIK_DEBUG_TRACECLK_CTRL),
            .divsys_reg =
              (volatile uint32_t *)(PIK_DEBUG_BASE + PIK_DEBUG_TRACECLK_DIV1),
            .rate_table = rate_table_dbgtraceclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_dbgtraceclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_DEBUG_ATCLKDBG] = {
        .name = "DEBUG_ATCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg =
              (volatile uint32_t *)(PIK_DEBUG_BASE + PIK_DEBUG_ATCLKDBG_CTRL),
            .divsys_reg =
              (volatile uint32_t *)(PIK_DEBUG_BASE + PIK_DEBUG_ATCLKDBG_DIV1),
            .rate_table = rate_table_dbgatclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_dbgatclk),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_DEBUG_PCLKDBG] = {
        .name = "DEBUG_PCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE,
            .is_group_member = false,
            .control_reg =
              (volatile uint32_t *)(PIK_DEBUG_BASE + PIK_DEBUG_PCLKDBG_CTRL),
            .rate_table = rate_table_dbgpclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_dbgpclk),
            .initial_rate = 500 * FWK_MHZ,
        }),
    },

    [CLOCK_PIK_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *pik_clock_get_element_table(
    fwk_id_t module_id)
{
    return pik_clock_element_table;
}

const struct fwk_module_config config_pik_clock = {
    .get_element_table = pik_clock_get_element_table,
};
