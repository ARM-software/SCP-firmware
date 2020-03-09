/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "scp_sgi575_pik.h"
#include "sgi575_pik_cpu.h"
#include "sgi575_pik_scp.h"
#include "sgi575_pik_system.h"
#include "system_clock.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>

/*
 * Rate lookup tables
 */

static struct mod_pik_clock_rate rate_table_cpu_group_0[] = {
    {
        .rate = 2600 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static struct mod_pik_clock_rate rate_table_cpu_group_1[] = {
    {
        .rate = 2600 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
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
    /*
     * Cluster 0 CPUS
     */
    [CLOCK_PIK_IDX_CLUS0_CPU0] = {
        .name = "CLUS0_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(0)->CORECLK[0].CTRL,
            .divext_reg = &PIK_CLUSTER(0)->CORECLK[0].DIV,
            .modulator_reg = &PIK_CLUSTER(0)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group_0,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_0),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU1] = {
        .name = "CLUS0_CPU1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(0)->CORECLK[1].CTRL,
            .divext_reg = &PIK_CLUSTER(0)->CORECLK[1].DIV,
            .modulator_reg = &PIK_CLUSTER(0)->CORECLK[1].MOD,
            .rate_table = rate_table_cpu_group_0,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_0),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU2] = {
        .name = "CLUS0_CPU2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(0)->CORECLK[2].CTRL,
            .divext_reg = &PIK_CLUSTER(0)->CORECLK[2].DIV,
            .modulator_reg = &PIK_CLUSTER(0)->CORECLK[2].MOD,
            .rate_table = rate_table_cpu_group_0,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_0),
        }),
    },
    [CLOCK_PIK_IDX_CLUS0_CPU3] = {
        .name = "CLUS0_CPU3",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(0)->CORECLK[3].CTRL,
            .divext_reg = &PIK_CLUSTER(0)->CORECLK[3].DIV,
            .modulator_reg = &PIK_CLUSTER(0)->CORECLK[3].MOD,
            .rate_table = rate_table_cpu_group_0,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_0),
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_CPU0] = {
        .name = "CLUS1_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(1)->CORECLK[0].CTRL,
            .divext_reg = &PIK_CLUSTER(1)->CORECLK[0].DIV,
            .modulator_reg = &PIK_CLUSTER(1)->CORECLK[0].MOD,
            .rate_table = rate_table_cpu_group_1,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_1),
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_CPU1] = {
        .name = "CLUS1_CPU1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(1)->CORECLK[1].CTRL,
            .divext_reg = &PIK_CLUSTER(1)->CORECLK[1].DIV,
            .modulator_reg = &PIK_CLUSTER(1)->CORECLK[1].MOD,
            .rate_table = rate_table_cpu_group_1,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_1),
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_CPU2] = {
        .name = "CLUS1_CPU2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(1)->CORECLK[2].CTRL,
            .divext_reg = &PIK_CLUSTER(1)->CORECLK[2].DIV,
            .modulator_reg = &PIK_CLUSTER(1)->CORECLK[2].MOD,
            .rate_table = rate_table_cpu_group_1,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_1),
        }),
    },
    [CLOCK_PIK_IDX_CLUS1_CPU3] = {
        .name = "CLUS1_CPU3",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUSTER(1)->CORECLK[3].CTRL,
            .divext_reg = &PIK_CLUSTER(1)->CORECLK[3].DIV,
            .modulator_reg = &PIK_CLUSTER(1)->CORECLK[3].MOD,
            .rate_table = rate_table_cpu_group_1,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group_1),
        }),
    },
    [CLOCK_PIK_IDX_DMC] = {
        .name = "DMC",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->DMCCLK_CTRL,
            .divext_reg = &PIK_SYSTEM->DMCCLK_DIV1,
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
            .control_reg = &PIK_SYSTEM->INTCLK_CTRL,
            .divext_reg = &PIK_SYSTEM->INTCLK_DIV1,
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
            .control_reg = &PIK_SCP->CORECLK_CTRL,
            .divsys_reg = &PIK_SCP->CORECLK_DIV1,
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
            .control_reg = &PIK_SYSTEM->GICCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->GICCLK_DIV1,
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
            .control_reg = &PIK_SYSTEM->PCLKSCP_CTRL,
            .divsys_reg = &PIK_SYSTEM->PCLKSCP_DIV1,
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
            .control_reg = &PIK_SYSTEM->SYSPERCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->SYSPERCLK_DIV1,
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
            .control_reg = &PIK_SYSTEM->UARTCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->UARTCLK_DIV1,
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
