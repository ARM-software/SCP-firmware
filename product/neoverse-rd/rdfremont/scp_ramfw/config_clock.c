/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'clock'.
 */

#include "platform_core.h"
#include "scp_cfgd_power_domain.h"
#include "scp_clock.h"

#include <mod_clock.h>
#include <mod_css_clock.h>
#include <mod_pik_clock.h>
#include <mod_power_domain.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/* Module 'clock' element count */
#define MOD_CLOCK_ELEMENT_COUNT (CFGD_MOD_CLOCK_EIDX_COUNT + 1)

/*
 * Helper macro to instantiate 'clock' module element config data.
 */
#define CFGD_MOD_CLOCK_ELEMENT_CPU(n) \
    [CFGD_MOD_CLOCK_EIDX_CPU##n] = { \
        .name = "CPU" #n, \
        .data = &((struct mod_clock_dev_config){ \
            .driver_id = FWK_ID_ELEMENT_INIT( \
                FWK_MODULE_IDX_PIK_CLOCK, CFGD_MOD_PIK_CLOCK_EIDX_CPU##n), \
            .api_id = FWK_ID_API_INIT( \
                FWK_MODULE_IDX_PIK_CLOCK, MOD_PIK_CLOCK_API_TYPE_CLOCK), \
            .default_on = true, \
        }), \
    }

/*
 * Module 'clock' element configuration data.
 */
static const struct fwk_element clock_dev_table[MOD_CLOCK_ELEMENT_COUNT] = {
    CFGD_MOD_CLOCK_ELEMENT_CPU(0),
    CFGD_MOD_CLOCK_ELEMENT_CPU(1),
    CFGD_MOD_CLOCK_ELEMENT_CPU(2),
    CFGD_MOD_CLOCK_ELEMENT_CPU(3),
    CFGD_MOD_CLOCK_ELEMENT_CPU(4),
    CFGD_MOD_CLOCK_ELEMENT_CPU(5),
    CFGD_MOD_CLOCK_ELEMENT_CPU(6),
    CFGD_MOD_CLOCK_ELEMENT_CPU(7),
    CFGD_MOD_CLOCK_ELEMENT_CPU(8),
    CFGD_MOD_CLOCK_ELEMENT_CPU(9),
    CFGD_MOD_CLOCK_ELEMENT_CPU(10),
    CFGD_MOD_CLOCK_ELEMENT_CPU(11),
    CFGD_MOD_CLOCK_ELEMENT_CPU(12),
    CFGD_MOD_CLOCK_ELEMENT_CPU(13),
    CFGD_MOD_CLOCK_ELEMENT_CPU(14),
    CFGD_MOD_CLOCK_ELEMENT_CPU(15),
    [CFGD_MOD_CLOCK_EIDX_CMN] = {
        .name = "CMN-Cyprus",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                CFGD_MOD_PIK_CLOCK_EIDX_CMN),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CLOCK),
        }),
    },
    { 0 },
};

static const struct fwk_element *clock_get_dev_desc_table(fwk_id_t module_id)
{
    unsigned int i;
    struct mod_clock_dev_config *dev_config;

    for (i = 0; i < CFGD_MOD_CLOCK_EIDX_COUNT; i++) {
        dev_config = (struct mod_clock_dev_config *)clock_dev_table[i].data;
        dev_config->pd_source_id = fwk_id_build_element_id(
            fwk_module_id_power_domain,
            platform_get_core_count() + platform_get_cluster_count() +
                PD_STATIC_DEV_IDX_SYSTOP);
    }

    return clock_dev_table;
}

const struct fwk_module_config config_clock = {
    .data =
        &(struct mod_clock_config){
            .pd_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_POWER_DOMAIN,
                MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION),
            .pd_pre_transition_notification_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_POWER_DOMAIN,
                MOD_PD_NOTIFICATION_IDX_POWER_STATE_PRE_TRANSITION),
        },

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(clock_get_dev_desc_table),
};
