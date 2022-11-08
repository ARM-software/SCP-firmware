/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "cpu_pik.h"
#include "platform_core.h"
#include "scp_pik.h"
#include "system_pik.h"

#include <mod_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>

#define CLOCK_PLL_CLUSn_CPU(n) \
    [CLOCK_PIK_IDX_CLUS##n##_CPU0] = { \
        .name = "CLUS" #n "_CPU0", \
        .data = &((struct mod_pik_clock_dev_config){ \
            .type = MOD_PIK_CLOCK_TYPE_CLUSTER, \
            .is_group_member = true, \
            .control_reg = &CLUSTER_PIK_PTR(n)->CORECLK_CTRL, \
            .divext_reg = &CLUSTER_PIK_PTR(n)->CORECLK_DIV1, \
            .modulator_reg = &CLUSTER_PIK_PTR(n)->CORECLK_MOD1, \
            .rate_table = rate_table_cpu_group, \
            .rate_count = FWK_ARRAY_SIZE(rate_table_cpu_group), \
        }), \
    }

/*
 * Rate lookup tables
 */
static struct mod_pik_clock_rate rate_table_cpu_group[] = {
    {
        .rate = 2600 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_CLUSCLK_SOURCE_PLL0,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT,
        .divider = 1,
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
        .rate = 800 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = CLOCK_RATE_SYSPLLCLK / (800 * FWK_MHZ),
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
    CLOCK_PLL_CLUSn_CPU(0),
    CLOCK_PLL_CLUSn_CPU(1),
    CLOCK_PLL_CLUSn_CPU(2),
    CLOCK_PLL_CLUSn_CPU(3),
#if (NUMBER_OF_CLUSTERS > 4)
    CLOCK_PLL_CLUSn_CPU(4),
    CLOCK_PLL_CLUSn_CPU(5),
    CLOCK_PLL_CLUSn_CPU(6),
    CLOCK_PLL_CLUSn_CPU(7),
#    if (NUMBER_OF_CLUSTERS > 8)
    CLOCK_PLL_CLUSn_CPU(8),
    CLOCK_PLL_CLUSn_CPU(9),
    CLOCK_PLL_CLUSn_CPU(10),
    CLOCK_PLL_CLUSn_CPU(11),
    CLOCK_PLL_CLUSn_CPU(12),
    CLOCK_PLL_CLUSn_CPU(13),
    CLOCK_PLL_CLUSn_CPU(14),
    CLOCK_PLL_CLUSn_CPU(15),
#    endif
#endif
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
            .initial_rate = 800 * FWK_MHZ,
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
            .control_reg = &SYSTEM_PIK_PTR->SCPPIKCLK_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->SCPPIKCLK_DIV1,
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
            .control_reg = &SYSTEM_PIK_PTR->APUARTCLK_CTRL,
            .divsys_reg = &SYSTEM_PIK_PTR->APUARTCLK_DIV1,
            .rate_table = rate_table_uartclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_uartclk),
            .initial_rate = 250 * FWK_MHZ,
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
