/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"

#include <mod_dvfs.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct mod_dvfs_opp opps[] = {
        {
            .frequency = 1313 * FWK_MHZ,
            .voltage = 100,
        },
        {
            .frequency = 1531 * FWK_MHZ,
            .voltage = 200,
        },
        {
            .frequency = 1750 * FWK_MHZ,
            .voltage = 300,
        },
        {
            .frequency = 2100 * FWK_MHZ,
            .voltage = 400,
        },
        {
            .frequency = 2600 * FWK_MHZ,
            .voltage = 500,
        },
        { 0 }
};

static const struct mod_dvfs_domain_config cpu_group0 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP0),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 0),
    .retry_ms = 1,
    .latency = 1200,
    .sustained_idx = 2,
    .opps = opps,
};

static const struct mod_dvfs_domain_config cpu_group1 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 1),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP1),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 1),
    .retry_ms = 1,
    .latency = 1200,
    .sustained_idx = 2,
    .opps = opps,
};

static const struct fwk_element element_table[] = {
    [0] = {
        .name = "GROUP0",
        .data = &cpu_group0,
    },
    [1] = {
        .name = "GROUP1",
        .data = &cpu_group1,
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
