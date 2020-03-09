/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "pik_scp.h"
#include "pik_system.h"
#include "synquacer_mmap.h"
#include "system_clock.h"

#include <mod_synquacer_pik_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>

/*
 * Rate lookup tables
 */
static const struct mod_pik_clock_rate rate_table_coreclk[] = {
    {
        .rate = CONFIG_SOC_CORE_CLOCK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, CONFIG_SOC_CORE_CLOCK),
    },
};

static const struct mod_pik_clock_rate rate_table_aclk[] = {
    {
        .rate = 125 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 125 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_ppuclk[] = {
    {
        .rate = 125 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 125 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_ccnclk[] = {
    {
        .rate = 1000 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 1000 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_gicclk[] = {
    {
        .rate = 500 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 500 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_pclkscp[] = {
    {
        .rate = 125 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 125 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_sysperclk[] = {
    {
        .rate = 500 * FWK_MHZ,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 500 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_syspclkdbg[] = {
    {
        .rate = 125 * FWK_MHZ,
        .source = (MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK | (0x1U << 16)),
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, 125 * FWK_MHZ),
    },
};

static const struct mod_pik_clock_rate rate_table_uartclk[] = {
    {
        .rate = CLOCK_RATE_AP_PL011CLK,
        .source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_SYSPLLCLK,
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_SYSINCLK, CLOCK_RATE_AP_PL011CLK),
    },
};

static const struct mod_pik_clock_rate rate_table_dmcclk[] = {
    {
        .rate = 1066 * FWK_MHZ,
        .source = (MOD_PIK_CLOCK_DMCCLK_SOURCE_DDRPLL | (0x1U << 16)),
        .divider_reg = MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS,
        .divider = DIV_FROM_CLOCK(CLOCK_RATE_DDRPLLCLK, 1066 * FWK_MHZ),
    },
};

static const struct fwk_element pik_clock_element_table[] = {
    [CLOCK_PIK_IDX_CORECLK] = {
        .name = "CORECLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SCP->CORECLK_CNTRL,
            .divsys_reg = &PIK_SCP->CORECLK_DIV1,
            .rate_table = rate_table_coreclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_coreclk),
            .initial_rate = CONFIG_SOC_CORE_CLOCK,
        }),
    },
    [CLOCK_PIK_IDX_ACLK] = {
        .name = "ACLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SCP->ACLK_CNTRL,
            .divsys_reg = &PIK_SCP->ACLK_DIV1,
            .rate_table = rate_table_aclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_aclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_PPUCLK] = {
        .name = "PPUCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->PPUCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->PPUCLK_DIV1,
            .rate_table = rate_table_ppuclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_ppuclk),
            .initial_rate = 125 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_CCNCLK] = {
        .name = "CCNCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->CCNCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->CCNCLK_DIV1,
            .rate_table = rate_table_ccnclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_ccnclk),
            .initial_rate = 1000 * FWK_MHZ,
        }),
    },
    [CLOCK_PIK_IDX_GICCLK] = {
        .name = "GICCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->GICCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->GICCLK_DIV1,
            .rate_table = rate_table_gicclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_gicclk),
            .initial_rate = 500 * FWK_MHZ,
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
            .initial_rate = 125 * FWK_MHZ,
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
    [CLOCK_PIK_IDX_SYSPCLKDBG] = {
        .name = "SYSPCLKDBG",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->SYSPCLKDBG_CTRL,
            .divsys_reg = &PIK_SYSTEM->SYSPCLKDBG_DIV1,
            .rate_table = rate_table_syspclkdbg,
            .rate_count = FWK_ARRAY_SIZE(rate_table_syspclkdbg),
            .initial_rate = 125 * FWK_MHZ,
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
            .initial_rate = CLOCK_RATE_AP_PL011CLK,
        }),
    },
    [CLOCK_PIK_IDX_DMCCLK] = {
        .name = "DMCCLK",
        .data = &((struct mod_pik_clock_dev_config) {
            .type = MOD_PIK_CLOCK_TYPE_MULTI_SOURCE,
            .is_group_member = false,
            .control_reg = &PIK_SYSTEM->DMCCLK_CTRL,
            .divsys_reg = &PIK_SYSTEM->DMCCLK_DIV1,
            .rate_table = rate_table_dmcclk,
            .rate_count = FWK_ARRAY_SIZE(rate_table_dmcclk),
            .initial_rate = 1066 * FWK_MHZ,
        }),
    },

    [CLOCK_PIK_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *pik_clock_get_element_table(fwk_id_t module_id)
{
    return pik_clock_element_table;
}

const struct fwk_module_config config_synquacer_pik_clock = {
    .get_element_table = pik_clock_get_element_table,
};
