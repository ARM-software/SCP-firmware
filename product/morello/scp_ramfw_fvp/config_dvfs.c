/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2022, Arm Limited and Contributors. All rights reserved.
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
    .latency = 1200,
    .sustained_idx = 4,
    .opps =
        (struct mod_dvfs_opp[]){
            {
                .level = CPU_CLOCK_SUD * 1000000UL,
                .frequency = CPU_CLOCK_SUD * FWK_KHZ,
                .voltage = 800,
                .power = (0.16 * CPU_CLOCK_SUD * 0.800 * 0.800),
            },
            {
                .level = CPU_CLOCK_UD * 1000000UL,
                .frequency = CPU_CLOCK_UD * FWK_KHZ,
                .voltage = 850,
                .power = (0.16 * CPU_CLOCK_UD * 0.850 * 0.850),
            },
            {
                .level = CPU_CLOCK_NOM * 1000000UL,
                .frequency = CPU_CLOCK_NOM * FWK_KHZ,
                .voltage = 900,
                .power = (0.16 * CPU_CLOCK_NOM * 0.900 * 0.900),
            },
            {
                .level = CPU_CLOCK_OD * 1000000UL,
                .frequency = CPU_CLOCK_OD * FWK_KHZ,
                .voltage = 950,
                .power = (0.16 * CPU_CLOCK_OD * 0.950 * 0.950),
            },
            {
                .level = CPU_CLOCK_SOD * 1000000UL,
                .frequency = CPU_CLOCK_SOD * FWK_KHZ,
                .voltage = 1000,
                .power = (0.16 * CPU_CLOCK_SOD * 1.000 * 1.000),
            },
            { 0 },
        }
};

static const struct mod_dvfs_domain_config cpu_group_1 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP1),
    .latency = 1200,
    .sustained_idx = 4,
    .opps =
        (struct mod_dvfs_opp[]){
            {
                .level = CPU_CLOCK_SUD * 1000000UL,
                .frequency = CPU_CLOCK_SUD * FWK_KHZ,
                .voltage = 800,
                .power = (0.16 * CPU_CLOCK_SUD * 0.800 * 0.800),
            },
            {
                .level = CPU_CLOCK_UD * 1000000UL,
                .frequency = CPU_CLOCK_UD * FWK_KHZ,
                .voltage = 850,
                .power = (0.16 * CPU_CLOCK_UD * 0.850 * 0.850),
            },
            {
                .level = CPU_CLOCK_NOM * 1000000UL,
                .frequency = CPU_CLOCK_NOM * FWK_KHZ,
                .voltage = 900,
                .power = (0.16 * CPU_CLOCK_NOM * 0.900 * 0.900),
            },
            {
                .level = CPU_CLOCK_OD * 1000000UL,
                .frequency = CPU_CLOCK_OD * FWK_KHZ,
                .voltage = 950,
                .power = (0.16 * CPU_CLOCK_OD * 0.950 * 0.950),
            },
            {
                .level = CPU_CLOCK_SOD * 1000000UL,
                .frequency = CPU_CLOCK_SOD * FWK_KHZ,
                .voltage = 1000,
                .power = (0.16 * CPU_CLOCK_SOD * 1.000 * 1.000),
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
