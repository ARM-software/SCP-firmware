/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "config_power_domain.h"
#include "tc_core.h"

#include <mod_clock.h>
#include <mod_css_clock.h>
#include <mod_pik_clock.h>
#include <mod_power_domain.h>
#include <mod_system_pll.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element clock_dev_desc_table[CLOCK_IDX_COUNT + 1] = {
    [CLOCK_IDX_CPU_GROUP_HAYES] = {
            .name = "CPU_GROUP_HAYES",
            .data = &((struct mod_clock_dev_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CSS_CLOCK,
                    CLOCK_CSS_IDX_CPU_GROUP_HAYES),
                .api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_CSS_CLOCK,
                    MOD_CSS_CLOCK_API_TYPE_CLOCK),
            }),
    },
    [CLOCK_IDX_CPU_GROUP_HUNTER] = {
            .name = "CPU_GROUP_HUNTER",
            .data = &((struct mod_clock_dev_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CSS_CLOCK,
                    CLOCK_CSS_IDX_CPU_GROUP_HUNTER),
                .api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_CSS_CLOCK,
                    MOD_CSS_CLOCK_API_TYPE_CLOCK),
            }),
    },
    [CLOCK_IDX_CPU_GROUP_HUNTER_ELP] = {
            .name = "CPU_GROUP_HUNTER_ELP",
            .data = &((struct mod_clock_dev_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CSS_CLOCK,
                    CLOCK_CSS_IDX_CPU_GROUP_HUNTER_ELP),
                .api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_CSS_CLOCK,
                    MOD_CSS_CLOCK_API_TYPE_CLOCK),
            }),
    },
    [CLOCK_IDX_DPU] = {
            .name = "DPU",
            .data = &((struct mod_clock_dev_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CSS_CLOCK,
                    CLOCK_CSS_IDX_DPU),
                .api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_CSS_CLOCK,
                    MOD_CSS_CLOCK_API_TYPE_CLOCK),
            }),
    },
    [CLOCK_IDX_PIXEL_0] = {
            .name = "PIXEL_0",
            .data = &((struct mod_clock_dev_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_SYSTEM_PLL,
                    CLOCK_PLL_IDX_PIX0),
                .api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_SYSTEM_PLL,
                    MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            }),
    },
    [CLOCK_IDX_PIXEL_1] = {
            .name = "PIXEL_1",
            .data = &((struct mod_clock_dev_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_SYSTEM_PLL,
                    CLOCK_PLL_IDX_PIX1),
                .api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_SYSTEM_PLL,
                    MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            }),
        },
    [CLOCK_IDX_GPU] =
        {
            .name = "GPU",
            .data = &((struct mod_clock_dev_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_SYSTEM_PLL,
                    CLOCK_PLL_IDX_GPU),
                .api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_SYSTEM_PLL,
                    MOD_SYSTEM_PLL_API_TYPE_DEFAULT),
            }),
        },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *clock_get_dev_desc_table(fwk_id_t module_id)
{
    unsigned int i;
    struct mod_clock_dev_config *dev_config;

    for (i = 0; i < CLOCK_IDX_COUNT; i++) {
        dev_config =
            (struct mod_clock_dev_config *)clock_dev_desc_table[i].data;
        dev_config->pd_source_id = fwk_id_build_element_id(
            fwk_module_id_power_domain,
            tc_core_get_core_count() + tc_core_get_cluster_count() +
                PD_STATIC_DEV_IDX_SYSTOP);
    }

    return clock_dev_desc_table;
}

const struct fwk_module_config config_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(clock_get_dev_desc_table),
    .data = &((struct mod_clock_config){
        .pd_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION),
        .pd_pre_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            MOD_PD_NOTIFICATION_IDX_POWER_STATE_PRE_TRANSITION),
    }),

};
