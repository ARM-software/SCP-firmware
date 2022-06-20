/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "config_dvfs.h"
#include "config_psu.h"

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
    .psu_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, CONFIG_PSU_ELEMENT_IDX_CLUS0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP0),
    .latency = 1200,
    .sustained_idx = 4,
    .opps =
        (struct mod_dvfs_opp[]){
            {
                .level = CPU_CLOCK_LEVEL1 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL1 * FWK_KHZ,
                .voltage = 750,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL1) * 0.750 * 0.750),
            },
            {
                .level = CPU_CLOCK_LEVEL2 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL2 * FWK_KHZ,
                .voltage = 775,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL2) * 0.775 * 0.775),
            },
            {
                .level = CPU_CLOCK_LEVEL3 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL3 * FWK_KHZ,
                .voltage = 825,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL3) * 0.825 * 0.825),
            },
            {
                .level = CPU_CLOCK_LEVEL4 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL4 * FWK_KHZ,
                .voltage = 875,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL4) * 0.875 * 0.875),
            },
            {
                .level = CPU_CLOCK_LEVEL5 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL5 * FWK_KHZ,
                .voltage = 925,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL5) * 0.925 * 0.925),
            },
            { 0 },
        }
};

static const struct mod_dvfs_domain_config cpu_group_1 = {
    .psu_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, CONFIG_PSU_ELEMENT_IDX_CLUS1),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP1),
    .latency = 1200,
    .sustained_idx = 4,
    .opps =
        (struct mod_dvfs_opp[]){
            {
                .level = CPU_CLOCK_LEVEL1 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL1 * FWK_KHZ,
                .voltage = 750,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL1) * 0.750 * 0.750),
            },
            {
                .level = CPU_CLOCK_LEVEL2 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL2 * FWK_KHZ,
                .voltage = 775,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL2) * 0.775 * 0.775),
            },
            {
                .level = CPU_CLOCK_LEVEL3 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL3 * FWK_KHZ,
                .voltage = 825,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL3) * 0.825 * 0.825),
            },
            {
                .level = CPU_CLOCK_LEVEL4 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL4 * FWK_KHZ,
                .voltage = 875,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL4) * 0.875 * 0.875),
            },
            {
                .level = CPU_CLOCK_LEVEL5 * 1000000UL,
                .frequency = CPU_CLOCK_LEVEL5 * FWK_KHZ,
                .voltage = 925,
                .power = ((250 + 1.645 * CPU_CLOCK_LEVEL5) * 0.925 * 0.925),
            },
            { 0 },
        }
};

static const struct mod_dvfs_domain_config gpu = {
    .psu_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, CONFIG_PSU_ELEMENT_IDX_GPU),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_GPU),
    .latency = 1200,
    .sustained_idx = 4,
    .opps =
        (struct mod_dvfs_opp[]){
            {
                .level = 450 * 1000000UL,
                .frequency = 450 * FWK_KHZ,
                .voltage = 800,
                .power = (0.16 * 450 * 0.800 * 0.800),
            },
            {
                .level = 500 * 1000000UL,
                .frequency = 500 * FWK_KHZ,
                .voltage = 850,
                .power = (0.16 * 500 * 0.850 * 0.850),
            },
            {
                .level = 550 * 1000000UL,
                .frequency = 550 * FWK_KHZ,
                .voltage = 900,
                .power = (0.16 * 550 * 0.900 * 0.900),
            },
            {
                .level = 600 * 1000000UL,
                .frequency = 600 * FWK_KHZ,
                .voltage = 950,
                .power = (0.16 * 600 * 0.950 * 0.950),
            },
            {
                .level = 650 * 1000000UL,
                .frequency = 650 * FWK_KHZ,
                .voltage = 1000,
                .power = (0.16 * 650 * 1.000 * 1.000),
            },
            { 0 },
        }
};

static const struct fwk_element element_table[DVFS_ELEMENT_IDX_COUNT + 1] = {
    [DVFS_ELEMENT_IDX_CLUS0] = {
        .name = "CLUSTER_0_CPUS",
        .data = &cpu_group_0,
    },
    [DVFS_ELEMENT_IDX_CLUS1] = {
        .name = "CLUSTER_1_CPUS",
        .data = &cpu_group_1,
    },
    [DVFS_ELEMENT_IDX_GPU] = {
        .name = "GPU",
        .data = &gpu,
    },
    { 0 }
};

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_dvfs = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table),
    .data = NULL,
};
