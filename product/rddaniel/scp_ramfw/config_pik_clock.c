/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "cpu_pik.h"
#include "scp_pik.h"
#include "system_pik.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>

/*
 * Rate lookup tables
 */
static struct mod_pik_clock_rate rate_table_cpu_group[] = {
    {
        .rate = 2600 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static const struct mod_pik_clock_rate rate_table_sys_intclk[] = {
    {
        .rate = 2000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_INTCLK_SOURCE_INTPLL,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

static const struct mod_pik_clock_rate rate_table_sys_dmcclk[] = {
    {
        .rate = 1600 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_DMCCLK_SOURCE_DDRPLL,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
    },
};

static const struct mod_pik_clock_rate rate_table_scp[] = {
    {
        .rate = 250 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (250 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_gicclk[] = {
    {
        .rate = 1000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (1000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_pclkscp[] = {
    {
        .rate = 400 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (400 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_sysperclk[] = {
    {
        .rate = 500 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (500 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_uartclk[] = {
    {
        .rate = 250 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (250 * FWK_MHZ),
    },
};

static const struct fwk_element pik_clock_element_table[] = {

    [CLOCK_PIK_IDX_CLUS0_CPU0] = {
        .name = "CLUS0_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(0)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(0)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(0)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_CPU0] = {
        .name = "CLUS1_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(1)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(1)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(0)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS2_CPU0] = {
        .name = "CLUS2_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(2)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(2)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(2)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS3_CPU0] = {
        .name = "CLUS3_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(3)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(3)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(3)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS4_CPU0] = {
        .name = "CLUS4_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(4)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(4)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(4)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS5_CPU0] = {
        .name = "CLUS5_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(5)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(5)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(5)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS6_CPU0] = {
        .name = "CLUS6_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(6)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(6)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(6)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS7_CPU0] = {
        .name = "CLUS7_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(7)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(7)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(7)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS8_CPU0] = {
        .name = "CLUS8_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(8)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(8)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(8)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS9_CPU0] = {
        .name = "CLUS9_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(9)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(9)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(9)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS10_CPU0] = {
        .name = "CLUS10_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(10)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(10)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(10)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS11_CPU0] = {
        .name = "CLUS11_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(11)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(11)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(11)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS12_CPU0] = {
        .name = "CLUS12_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(12)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(12)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(12)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS13_CPU0] = {
        .name = "CLUS13_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(13)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(13)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(13)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS14_CPU0] = {
        .name = "CLUS14_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(14)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(14)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(14)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_CLUS15_CPU0] = {
        .name = "CLUS15_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &CLUSTER_PIK_PTR(15)->CORECLK[0].CTRL,
            .divext_reg = &CLUSTER_PIK_PTR(15)->CORECLK[0].DIV,
            .modulator_reg = &CLUSTER_PIK_PTR(15)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group),
        }),
    },
    [CLOCK_PIK_IDX_DMC] = {
        .name = "DMC",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->DMCCLK_CTRL,
            .divext_reg = &SYSTEM_PIK_PTR->DMCCLK_DIV1,
            .rate_table = rate_table_sys_dmcclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_dmcclk),
            .initial_rate = 1600 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_INTERCONNECT] = {
        .name = "INTERCONNECT",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SYSTEM_PIK_PTR->INTCLK_CTRL,
            .divext_reg = &SYSTEM_PIK_PTR->INTCLK_DIV1,
            .rate_table = rate_table_sys_intclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_intclk),
            .initial_rate = 2000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_SCP] = {
        .name = "SCP",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &SCP_PIK_PTR->CORECLK_CTRL,
            .divsys_reg = &SCP_PIK_PTR->CORECLK_DIV1,
            .rate_table = rate_table_scp,
            .rate_count = FWK_ARRAY_SIZE(rate_table_scp),
            .initial_rate = 250 * FWK_MHZ,
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
            .initial_rate = 1000 * FWK_MHZ,
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
            .initial_rate = 400 * FWK_MHZ,
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
            .initial_rate = 500 * FWK_MHZ,
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
            .initial_rate = 250 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *pik_clock_get_element_table
    (fwk_id_t module_id)
{
    return pik_clock_element_table;
}

const struct fwk_module_config config_pik_clock = {
    .get_element_table = pik_clock_get_element_table,
};
