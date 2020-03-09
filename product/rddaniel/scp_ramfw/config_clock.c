/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "config_power_domain.h"
#include "rddaniel_core.h"

#include <mod_clock.h>
#include <mod_css_clock.h>
#include <mod_pik_clock.h>
#include <mod_power_domain.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element clock_dev_desc_table[] = {
    [CLOCK_IDX_INTERCONNECT] = {
        .name = "Interconnect",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                CLOCK_PIK_IDX_INTERCONNECT),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PIK_CLOCK,
                MOD_PIK_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP0] = {
        .name = "CPU_GROUP0",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP1] = {
        .name = "CPU_GROUP1",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP2] = {
        .name = "CPU_GROUP2",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP3] = {
        .name = "CPU_GROUP3",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP4] = {
        .name = "CPU_GROUP4",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP4),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP5] = {
        .name = "CPU_GROUP5",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP5),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP6] = {
        .name = "CPU_GROUP6",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP6),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP7] = {
        .name = "CPU_GROUP7",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP7),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP8] = {
        .name = "CPU_GROUP8",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP8),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP9] = {
        .name = "CPU_GROUP9",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP9),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP10] = {
        .name = "CPU_GROUP10",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP10),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP11] = {
        .name = "CPU_GROUP11",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP11),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP12] = {
        .name = "CPU_GROUP12",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP12),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP13] = {
        .name = "CPU_GROUP13",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP13),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP14] = {
        .name = "CPU_GROUP14",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP14),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
        }),
    },
    [CLOCK_IDX_CPU_GROUP15] = {
        .name = "CPU_GROUP15",
        .data = &((struct mod_clock_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                CLOCK_CSS_IDX_CPU_GROUP15),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_CSS_CLOCK,
                MOD_CSS_CLOCK_API_TYPE_CLOCK),
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
            rddaniel_core_get_core_count() + PD_STATIC_DEV_IDX_SYSTOP);
    }

    return clock_dev_desc_table;
}

const struct fwk_module_config config_clock = {
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
