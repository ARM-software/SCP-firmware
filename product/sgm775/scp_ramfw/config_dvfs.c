/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <config_dvfs.h>
#include <mod_dvfs.h>

static const struct mod_dvfs_domain_config cpu_group_little = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 1),
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 665 * FWK_MHZ,
            .voltage = 100,
        },
        {
            .frequency = 998 * FWK_MHZ,
            .voltage = 200,
        },
        {
            .frequency = 1330 * FWK_MHZ,
            .voltage = 300,
        },
        {
            .frequency = 1463 * FWK_MHZ,
            .voltage = 400,
        },
        {
            .frequency = 1596 * FWK_MHZ,
            .voltage = 500,
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config cpu_group_big = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 1),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 0),
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]) {
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
            .frequency = 2450 * FWK_MHZ,
            .voltage = 500,
        },
        { 0 }
    }
};

static const struct mod_dvfs_domain_config gpu = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 2),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 2),
    .latency = 1200,
    .sustained_idx = 4,
    .opps = (struct mod_dvfs_opp[]) {
        {
            .frequency = 450 * FWK_MHZ,
            .voltage = 100,
        },
        {
            .frequency = 487500 * FWK_KHZ,
            .voltage = 200,
        },
        {
            .frequency = 525 * FWK_MHZ,
            .voltage = 300,
        },
        {
            .frequency = 562500 * FWK_KHZ,
            .voltage = 400,
        },
        {
            .frequency = 600 * FWK_MHZ,
            .voltage = 500,
        },
        { 0 }
    }
};

static const struct fwk_element element_table[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .name = "CPU_GROUP_LITTLE",
        .data = &cpu_group_little,
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .name = "CPU_GROUP_BIG",
        .data = &cpu_group_big,
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
    .get_element_table = dvfs_get_element_table,
    .data = NULL,
};
