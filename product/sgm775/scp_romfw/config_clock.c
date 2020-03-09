/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_devices.h"

#include <mod_clock.h>
#include <mod_css_clock.h>
#include <mod_msys_rom.h>
#include <mod_pik_clock.h>
#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element clock_dev_desc_table[] = {
    [CLOCK_DEV_IDX_BIG] = {
        .name = "CPU_GROUP_BIG",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK, 0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                                      MOD_CSS_CLOCK_API_TYPE_CLOCK),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_LITTLE] = {
        .name = "CPU_GROUP_LITTLE",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK, 1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                                      MOD_CSS_CLOCK_API_TYPE_CLOCK),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_GPU] = {
        .name = "GPU",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK, 2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                                      MOD_CSS_CLOCK_API_TYPE_CLOCK),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_SYS_ACLKNCI] = {
        .name = "SYS_ACLKNCI",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                      MOD_PIK_CLOCK_API_TYPE_CLOCK),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_SYS_FCMCLK] = {
        .name = "SYS_FCMCLK",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                      MOD_PIK_CLOCK_API_TYPE_CLOCK),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_SYS_GICCLK] = {
        .name = "SYS_GICCLK",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                      MOD_PIK_CLOCK_API_TYPE_CLOCK),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_SYS_PCLKSCP] = {
        .name = "SYS_PCLKSCP",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                      MOD_PIK_CLOCK_API_TYPE_CLOCK),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_SYS_SYSPERCLK] = {
        .name = "SYS_SYSPERCLK",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                      MOD_PIK_CLOCK_API_TYPE_CLOCK),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_PLL_SWTCLKTCK] = {
        .name = "PLL_SWTCLKTCK",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                      MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_PLL_SYSTEM] = {
        .name = "PLL_SYSTEM",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                      MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        }),
    },
    [CLOCK_DEV_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *clock_get_dev_desc_table(fwk_id_t module_id)
{
    return clock_dev_desc_table;
}

const struct fwk_module_config config_clock = {
    .get_element_table = clock_get_dev_desc_table,
    .data = &((struct mod_clock_config) {
        .pd_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
            FWK_MODULE_IDX_MSYS_ROM,
            MOD_MSYS_ROM_NOTIFICATION_IDX_POWER_SYSTOP),
        .pd_pre_transition_notification_id = FWK_ID_NONE_INIT,
    }),
};
