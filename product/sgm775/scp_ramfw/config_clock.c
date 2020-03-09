/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_devices.h"
#include "config_power_domain.h"
#include "sgm775_core.h"

#include <mod_clock.h>
#include <mod_css_clock.h>
#include <mod_pik_clock.h>
#include <mod_power_domain.h>
#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct fwk_element clock_dev_desc_table[] = {
    [CLOCK_DEV_IDX_BIG] = {
        .name = "CPU_GROUP_BIG",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK, 0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                                       MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_LITTLE] = {
        .name = "CPU_GROUP_LITTLE",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK, 1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                                       MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_GPU] = {
        .name = "GPU",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK, 2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                                       MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_VPU] = {
        .name = "VPU",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK, 3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                                       MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_DPU] = {
        .name = "DPU",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK, 4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                                       MOD_CSS_CLOCK_API_TYPE_CLOCK),
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
    [CLOCK_DEV_IDX_FCMCLK] = {
        .name = "FCMCLK",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK, 12),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                                      MOD_PIK_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_DEV_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *clock_get_dev_desc_table(fwk_id_t module_id)
{
    unsigned int i;
    unsigned int core_count;
    struct mod_clock_dev_config *dev_config;

    core_count = sgm775_core_get_count();

    /* Configure all clocks to respond to changes in SYSTOP power state */
    for (i = 0; i < CLOCK_DEV_IDX_COUNT; i++) {
        dev_config =
            (struct mod_clock_dev_config *)clock_dev_desc_table[i].data;
        dev_config->pd_source_id = FWK_ID_ELEMENT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            CONFIG_POWER_DOMAIN_SYSTOP_CHILD_COUNT + core_count);
    }

    return clock_dev_desc_table;
}

struct fwk_module_config config_clock = {
    .get_element_table = clock_get_dev_desc_table,
    .data = &((struct mod_clock_config) {
        .pd_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION),
        .pd_pre_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            MOD_PD_NOTIFICATION_IDX_POWER_STATE_PRE_TRANSITION),
    }),
};
