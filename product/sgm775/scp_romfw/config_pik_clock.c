/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm775_pik.h"
#include "sgm775_pik_cpu.h"
#include "sgm775_pik_gpu.h"
#include "sgm775_pik_system.h"
#include "system_clock.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>

/*
 * Rate lookup tables.
 */

static const struct mod_pik_clock_rate rate_table_sys_fcmclk[] = {
    {
        .rate = CLOCK_RATE_SYSPLLCLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = 1,
    },
};

static const struct mod_pik_clock_rate rate_table_sys_aclknci[] = {
    {
        .rate = 666 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSPLLCLK, 666 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_sys_gicclk[] = {
    {
        .rate = 666 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSPLLCLK, 666 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_sys_pclkscp[] = {
    {
        .rate = 125 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSPLLCLK, 125 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_sys_sysperclk[] = {
    {
        .rate = 125 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSPLLCLK, 125 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_cpu_a55[] = {
    {
        .rate = 1330 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static const struct mod_pik_clock_rate rate_table_cpu_a75[] = {
    {
        .rate = 1750 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL1,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via CPU PLL */
    },
};

static const struct mod_pik_clock_rate rate_table_gpu[] = {
    {
        .rate = 600 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_PRIVPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1, /* Rate adjusted via GPU PLL */
    },
};

static const struct fwk_element pik_clock_element_table[] = {
    /*
     * System Clocks
     */
    {
        .name = "SYS_ACLKNCI",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->ACLKNCI_CTRL,
            .divsys_reg = &PIK_SYSTEM->ACLKNCI_DIV1,
            .rate_table = rate_table_sys_aclknci,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_aclknci),
            .initial_rate = 666 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "SYS_FCMCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->FCMCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->FCMCLK_DIV1,
            .rate_table = rate_table_sys_fcmclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_fcmclk),
            .initial_rate = CLOCK_RATE_SYSPLLCLK,
            .defer_initialization = true,
        }),
    },
    {
        .name = "SYS_GICCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->GICCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->GICCLK_DIV1,
            .rate_table = rate_table_sys_gicclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_gicclk),
            .initial_rate = 666 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "SYS_PCLKSCP",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->PCLKSCP_CTRL,
            .divsys_reg = &PIK_SYSTEM->PCLKSCP_DIV1,
            .rate_table = rate_table_sys_pclkscp,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_pclkscp),
            .initial_rate = 125 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "SYS_SYSPERCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->SYSPERCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->SYSPERCLK_DIV1,
            .rate_table = rate_table_sys_sysperclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_sys_sysperclk),
            .initial_rate = 125 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    /*
     * A55 CPUS
     */
    {
        .name = "CLUS0_CPU0",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->AP_CLK_CTRL[0].CORECLK_CTRL,
            .divext_reg = &PIK_CLUS0->AP_CLK_CTRL[0].CORECLK_DIV,
            .modulator_reg = &PIK_CLUS0->AP_CLK_CTRL[0].CORECLK_MOD,
            .rate_table = rate_table_cpu_a55,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a55),
            .initial_rate = 1330 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU1",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->AP_CLK_CTRL[1].CORECLK_CTRL,
            .divext_reg = &PIK_CLUS0->AP_CLK_CTRL[1].CORECLK_DIV,
            .modulator_reg = &PIK_CLUS0->AP_CLK_CTRL[1].CORECLK_MOD,
            .rate_table = rate_table_cpu_a55,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a55),
            .initial_rate = 1330 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU2",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->AP_CLK_CTRL[2].CORECLK_CTRL,
            .divext_reg = &PIK_CLUS0->AP_CLK_CTRL[2].CORECLK_DIV,
            .modulator_reg = &PIK_CLUS0->AP_CLK_CTRL[2].CORECLK_MOD,
            .rate_table = rate_table_cpu_a55,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a55),
            .initial_rate = 1330 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    {
        .name = "CLUS0_CPU3",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->AP_CLK_CTRL[3].CORECLK_CTRL,
            .divext_reg = &PIK_CLUS0->AP_CLK_CTRL[3].CORECLK_DIV,
            .modulator_reg = &PIK_CLUS0->AP_CLK_CTRL[3].CORECLK_MOD,
            .rate_table = rate_table_cpu_a55,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a55),
            .initial_rate = 1330 * FWK_MHZ,
            .defer_initialization = true,
        }),
    },
    /*
     * A75 CPUS
     */
    {
        .name = "CLUS0_CPU4",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->AP_CLK_CTRL[4].CORECLK_CTRL,
            .divext_reg = &PIK_CLUS0->AP_CLK_CTRL[4].CORECLK_DIV,
            .modulator_reg = &PIK_CLUS0->AP_CLK_CTRL[4].CORECLK_MOD,
            .rate_table = rate_table_cpu_a75,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a75),
            .initial_rate = 1750 * FWK_MHZ,
            .defer_initialization = true,
     }),
    },
    {
        .name = "CLUS0_CPU5",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->AP_CLK_CTRL[5].CORECLK_CTRL,
            .divext_reg = &PIK_CLUS0->AP_CLK_CTRL[5].CORECLK_DIV,
            .modulator_reg = &PIK_CLUS0->AP_CLK_CTRL[5].CORECLK_MOD,
            .rate_table = rate_table_cpu_a75,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a75),
            .initial_rate = 1750 * FWK_MHZ,
            .defer_initialization = true,
     }),
    },
    {
        .name = "CLUS0_CPU6",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->AP_CLK_CTRL[6].CORECLK_CTRL,
            .divext_reg = &PIK_CLUS0->AP_CLK_CTRL[6].CORECLK_DIV,
            .modulator_reg = &PIK_CLUS0->AP_CLK_CTRL[6].CORECLK_MOD,
            .rate_table = rate_table_cpu_a75,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a75),
            .initial_rate = 1750 * FWK_MHZ,
            .defer_initialization = true,
     }),
    },
    {
        .name = "CLUS0_CPU7",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER,
            .is_group_member = true,
            .control_reg = &PIK_CLUS0->AP_CLK_CTRL[7].CORECLK_CTRL,
            .divext_reg = &PIK_CLUS0->AP_CLK_CTRL[7].CORECLK_DIV,
            .modulator_reg = &PIK_CLUS0->AP_CLK_CTRL[7].CORECLK_MOD,
            .rate_table = rate_table_cpu_a75,
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_a75),
            .initial_rate = 1750 * FWK_MHZ,
            .defer_initialization = true,
     }),
    },
    /*
     * GPU
     */
    {
        .name = "GPU",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = true,
            .control_reg = &PIK_GPU->GPUCLK_CTRL,
            .divsys_reg = &PIK_GPU->GPUCLK_DIV1,
            .divext_reg = &PIK_GPU->GPUCLK_DIV2,
            .rate_table = rate_table_gpu,
            .rate_count = FWK_ARRAY_SIZE(rate_table_gpu),
            .initial_rate = 600 * FWK_MHZ,
            .defer_initialization = true,
     }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *pik_clock_get_element_table
    (fwk_id_t module_id)
{
    return pik_clock_element_table;
}

struct fwk_module_config config_pik_clock = {
    .get_element_table = pik_clock_get_element_table,
};
