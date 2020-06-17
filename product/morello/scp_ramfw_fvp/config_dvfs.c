/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "config_dvfs.h"

#include <mod_dvfs.h>
#include <mod_scmi_perf.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

static const struct mod_dvfs_domain_config cpu_group_0 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP0),
    .notification_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_PERF),
    .updates_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_PERF,
        MOD_SCMI_PERF_DVFS_UPDATE_API),
    .latency = 1200,
    .sustained_idx = 2,
    .opps =
        (struct mod_dvfs_opp[]){
            {
                .level = 2000 * 1000000UL,
                .frequency = 2000 * FWK_KHZ,
                .voltage = 800,
                .power = (0.16 * 2000 * 0.800 * 0.800),
            },
            {
                .level = 2100 * 1000000UL,
                .frequency = 2100 * FWK_KHZ,
                .voltage = 850,
                .power = (0.16 * 2100 * 0.850 * 0.850),
            },
            {
                .level = 2200 * 1000000UL,
                .frequency = 2200 * FWK_KHZ,
                .voltage = 900,
                .power = (0.16 * 2200 * 0.900 * 0.900),
            },
            {
                .level = 2300 * 1000000UL,
                .frequency = 2300 * FWK_KHZ,
                .voltage = 950,
                .power = (0.16 * 2300 * 0.950 * 0.950),
            },
            {
                .level = 2400 * 1000000UL,
                .frequency = 2400 * FWK_KHZ,
                .voltage = 1000,
                .power = (0.16 * 2400 * 1.000 * 1.000),
            },
            { 0 },
        }
};

static const struct mod_dvfs_domain_config cpu_group_1 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP1),
    .notification_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_PERF),
    .updates_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_PERF,
        MOD_SCMI_PERF_DVFS_UPDATE_API),
    .latency = 1200,
    .sustained_idx = 2,
    .opps =
        (struct mod_dvfs_opp[]){
            {
                .level = 2000 * 1000000UL,
                .frequency = 2000 * FWK_KHZ,
                .voltage = 800,
                .power = (0.16 * 2000 * 0.800 * 0.800),
            },
            {
                .level = 2100 * 1000000UL,
                .frequency = 2100 * FWK_KHZ,
                .voltage = 850,
                .power = (0.16 * 2100 * 0.850 * 0.850),
            },
            {
                .level = 2200 * 1000000UL,
                .frequency = 2200 * FWK_KHZ,
                .voltage = 900,
                .power = (0.16 * 2200 * 0.900 * 0.900),
            },
            {
                .level = 2300 * 1000000UL,
                .frequency = 2300 * FWK_KHZ,
                .voltage = 950,
                .power = (0.16 * 2300 * 0.950 * 0.950),
            },
            {
                .level = 2400 * 1000000UL,
                .frequency = 2400 * FWK_KHZ,
                .voltage = 1000,
                .power = (0.16 * 2400 * 1.000 * 1.000),
            },
            { 0 },
        }
};

static const struct fwk_element element_table[] = {
    [DVFS_ELEMENT_IDX_CLUS0] =
        {
            .name = "CLUSTER_0_CPUS",
            .data = &cpu_group_0,
        },
    [DVFS_ELEMENT_IDX_CLUS1] =
        {
            .name = "CLUSTER_1_CPUS",
            .data = &cpu_group_1,
        },
    { 0 }
};

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

const struct fwk_module_config config_dvfs = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table),
    .data = NULL,
};
