/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "cpu_pik.h"
#include "dpu_pik.h"
#include "scp_pik.h"
#include "system_pik.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

/*
 * Rate lookup tables
 */
static const struct mod_pik_clock_rate rate_table_cpu_group_cortex_a520[1] = {
    {
        .rate = 1537 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_TC2_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static const struct mod_pik_clock_rate rate_table_cpu_group_cortex_a720[1] = {
    {
        .rate = 1893 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_TC2_PLL1,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static const struct mod_pik_clock_rate rate_table_cpu_group_cortex_x4[1] = {
    {
        .rate = 2176 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_TC2_PLL2,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static const struct mod_pik_clock_rate rate_table_gicclk[1] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (2000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_pclkscp[1] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (2000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_sysperclk[1] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (2000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_uartclk[1] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (2000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_dpu[1] = {
    {
        .rate = 600 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_ACLKDPU_SOURCE_DISPLAYPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via display PLL */
    },
};

static const struct fwk_element pik_clock_element_table[
    CLOCK_PIK_IDX_COUNT + 1] = {

    [CLOCK_PIK_IDX_CLUS0_CPU0] = {
        .name = "CLUS0_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group_cortex_a520,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_cortex_a520),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU1] = {
        .name = "CLUS0_CPU1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR->CORECLK[1].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR->CORECLK[1].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR->CORECLK[1].MOD,
            .rate_table = rate_table_cpu_group_cortex_a520,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_cortex_a520),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU2] = {
        .name = "CLUS0_CPU2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR->CORECLK[2].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR->CORECLK[2].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR->CORECLK[2].MOD,
            .rate_table = rate_table_cpu_group_cortex_a520,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_cortex_a520),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU3] = {
        .name = "CLUS0_CPU3",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR->CORECLK[3].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR->CORECLK[3].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR->CORECLK[3].MOD,
            .rate_table = rate_table_cpu_group_cortex_a520,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_cortex_a520),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU4] = {
        .name = "CLUS0_CPU4",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR->CORECLK[4].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR->CORECLK[4].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR->CORECLK[4].MOD,
            .rate_table = rate_table_cpu_group_cortex_a720,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_cortex_a720),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU5] = {
        .name = "CLUS0_CPU5",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR->CORECLK[5].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR->CORECLK[5].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR->CORECLK[5].MOD,
            .rate_table = rate_table_cpu_group_cortex_a720,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_cortex_a720),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU6] = {
        .name = "CLUS0_CPU6",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR->CORECLK[6].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR->CORECLK[6].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR->CORECLK[6].MOD,
            .rate_table = rate_table_cpu_group_cortex_a720,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_cortex_a720),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU7] = {
        .name = "CLUS0_CPU7",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR->CORECLK[7].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR->CORECLK[7].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR->CORECLK[7].MOD,
            .rate_table = rate_table_cpu_group_cortex_x4,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_cortex_x4),
        }),
    },
    [CLOCK_PIK_IDX_GIC] = {
        .name = "GIC",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->GICCLK_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->GICCLK_DIV1,
            .rate_table = rate_table_gicclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_gicclk),
            .initial_rate = 2000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_PCLKSCP] = {
        .name = "PCLKSCP",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->PCLKSCP_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->PCLKSCP_DIV1,
            .rate_table = rate_table_pclkscp,
            .rate_count = FWK_ARRAY_SIZE(rate_table_pclkscp),
            .initial_rate = 2000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_SYSPERCLK] = {
        .name = "SYSPERCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->SYSPERCLK_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->SYSPERCLK_DIV1,
            .rate_table = rate_table_sysperclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sysperclk),
            .initial_rate = 2000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_UARTCLK] = {
        .name = "UARTCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->UARTCLK_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->UARTCLK_DIV1,
            .rate_table = rate_table_uartclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_uartclk),
            .initial_rate = 2000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_DPU] = {
        .name = "DPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = true,
            .control_reg = &DPU_PIK_PTR->ACLKDP_CTRL,
            .divsys_reg = &DPU_PIK_PTR->ACLKDP_DIV1,
            .divext_reg = &DPU_PIK_PTR->ACLKDP_DIV2,
            .rate_table = rate_table_dpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_dpu),
            .initial_rate = 600 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    [CLOCK_PIK_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *pik_clock_get_element_table(fwk_id_t module_id)
{
    return pik_clock_element_table;
}

const struct fwk_module_config config_pik_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(pik_clock_get_element_table),
};
