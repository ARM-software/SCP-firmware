/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "tc0_timer.h"

#include <mod_dvfs.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct mod_dvfs_opp opps[] = {
        {
            .frequency = 946 * FWK_MHZ,
            .voltage = 550,
        },
        {
            .frequency = 1419 * FWK_MHZ,
            .voltage = 650,
        },
        {
            .frequency = 1893 * FWK_MHZ,
            .voltage = 750,
        },
        {
            .frequency = 2271 * FWK_MHZ,
            .voltage = 850,
        },
        {
            .frequency = 2650 * FWK_MHZ,
            .voltage = 950,
        },
        { 0 }
};

static const struct mod_dvfs_domain_config cpu_group = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP0),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        CONFIG_TIMER_DVFS_CPU),
    .notification_id = FWK_ID_NONE_INIT,
    .retry_ms = 1,
    .latency = 1200,
    .sustained_idx = 2,
    .opps = opps,
};

static const struct fwk_element element_table[] = {
    [0] = {
        .name = "CPU_GROUP",
        .data = &cpu_group,
    },
    { 0 }
};

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

const struct fwk_module_config config_dvfs = {
    .get_element_table = dvfs_get_element_table,
};
