/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_devices.h"

#include <mod_clock.h>
#include <mod_mock_clock.h>
#include <mod_system_pll.h>

#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct fwk_element clock_dev_desc_table[] = {
    [CLOCK_DEV_IDX_BIG] = {
        .name = "CPU_GROUP_BIG",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                       MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
        }),
    },
    [CLOCK_DEV_IDX_LITTLE] = {
        .name = "CPU_GROUP_LITTLE",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                       MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
        }),
    },
    [CLOCK_DEV_IDX_GPU] = {
        .name = "GPU",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                       MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
        }),
    },
    [CLOCK_DEV_IDX_VPU] = {
        .name = "VPU",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                       MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
        }),
    },
    [CLOCK_DEV_IDX_DPU] = {
        .name = "DPU",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                       MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
        }),
    },
    [CLOCK_DEV_IDX_PIXEL_0] = {
        .name = "PIXEL_0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 5),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                       MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
        }),
    },
    [CLOCK_DEV_IDX_PIXEL_1] = {
        .name = "PIXEL_1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SYSTEM_PLL, 6),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_PLL,
                                       MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
        }),
    },
    [CLOCK_DEV_IDX_MOCK_0] = {
        .name = "CLOCK_0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MOCK_CLOCK, 0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_CLOCK,
                                       MOD_MOCK_CLOCK_API_TYPE_DRIVER),
        }),
    },
    [CLOCK_DEV_IDX_MOCK_1] = {
        .name = "CLOCK_1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MOCK_CLOCK, 1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_CLOCK,
                                       MOD_MOCK_CLOCK_API_TYPE_DRIVER),
        }),
    },
    [CLOCK_DEV_IDX_MOCK_2] = {
        .name = "CLOCK_2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MOCK_CLOCK, 2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_CLOCK,
                                       MOD_MOCK_CLOCK_API_TYPE_DRIVER),
        }),
    },
    [CLOCK_DEV_IDX_MOCK_3] = {
        .name = "CLOCK_3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MOCK_CLOCK, 3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_CLOCK,
                                       MOD_MOCK_CLOCK_API_TYPE_DRIVER),
        }),
    },
   [CLOCK_DEV_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *clock_get_dev_desc_table(fwk_id_t module_id)
{
    unsigned int i;
    struct mod_clock_dev_config *dev_config;

    /* Configure all clocks to respond to changes in SYSTOP power state */
    for (i = 0; i < CLOCK_DEV_IDX_COUNT; i++) {
        dev_config =
            (struct mod_clock_dev_config *)clock_dev_desc_table[i].data;
        dev_config->pd_source_id = FWK_ID_NONE;
    }
    return clock_dev_desc_table;
}

struct fwk_module_config config_clock = {
    .data = &((struct mod_clock_config){
        .pd_transition_notification_id = FWK_ID_NONE_INIT,
        .pd_pre_transition_notification_id = FWK_ID_NONE_INIT,
    }),
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(clock_get_dev_desc_table),
};
