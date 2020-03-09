/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "scp_pik.h"
#include "scp_soc_mmap.h"

#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

static const struct fwk_element system_pll_element_table[] = {
    [CLOCK_PLL_IDX_CPU0] = {
        .name = "CPU_PLL_0",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU0,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(0),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU1] = {
        .name = "CPU_PLL_1",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU1,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(1),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU2] = {
        .name = "CPU_PLL_2",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU2,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(2),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU3] = {
        .name = "CPU_PLL_3",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU3,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(3),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU4] = {
        .name = "CPU_PLL_4",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU4,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(4),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU5] = {
        .name = "CPU_PLL_5",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU5,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(5),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU6] = {
        .name = "CPU_PLL_6",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU6,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(6),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU7] = {
        .name = "CPU_PLL_7",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU7,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(7),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU8] = {
        .name = "CPU_PLL_8",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU8,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(8),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU9] = {
        .name = "CPU_PLL_9",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU9,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(9),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU10] = {
        .name = "CPU_PLL_10",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU10,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(10),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU11] = {
        .name = "CPU_PLL_11",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU11,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(11),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU12] = {
        .name = "CPU_PLL_12",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU12,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(12),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU13] = {
        .name = "CPU_PLL_13",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU13,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(13),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU14] = {
        .name = "CPU_PLL_14",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU14,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(14),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_CPU15] = {
        .name = "CPU_PLL_15",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_CPU15,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[1],
            .lock_flag_mask = PLL_STATUS_CPUPLLLOCK(15),
            .initial_rate = 2600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_SYS] = {
        .name = "SYS_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_SYSPLL,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[0],
            .lock_flag_mask = PLL_STATUS_0_SYSPLLLOCK,
            .initial_rate = 2000 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_DMC] = {
        .name = "DMC_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_DMC,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[0],
            .lock_flag_mask = PLL_STATUS_0_DDRPLLLOCK,
            .initial_rate = 1600 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_INTERCONNECT] = {
        .name = "INT_PLL",
        .data = &((struct mod_system_pll_dev_config) {
            .control_reg = (void *)SCP_PLL_INTERCONNECT,
            .status_reg = (void *)&SCP_PIK_PTR->PLL_STATUS[0],
            .lock_flag_mask = PLL_STATUS_0_INTPLLLOCK,
            .initial_rate = 2000 * FWK_MHZ,
            .min_rate = MOD_SYSTEM_PLL_MIN_RATE,
            .max_rate = MOD_SYSTEM_PLL_MAX_RATE,
            .min_step = MOD_SYSTEM_PLL_MIN_INTERVAL,
        }),
    },
    [CLOCK_PLL_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *system_pll_get_element_table
    (fwk_id_t module_id)
{
    return system_pll_element_table;
}

const struct fwk_module_config config_system_pll = {
    .get_element_table = system_pll_get_element_table,
};
