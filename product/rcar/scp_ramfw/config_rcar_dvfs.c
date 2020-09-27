/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_rcar_dvfs.h>

#include <mod_dvfs.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct mod_dvfs_domain_config cpu_group_little = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PMIC, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 0),
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]){ {
                                         .frequency = 800 * FWK_MHZ,
                                         .voltage = 820000,
                                     },
                                     {
                                         .frequency = 1000 * FWK_MHZ,
                                         .voltage = 820000,
                                     },
                                     {
                                         .frequency = 1200 * FWK_MHZ,
                                         .voltage = 820000,
                                     },
                                     { 0 } }
};

static const struct mod_dvfs_domain_config cpu_group_big = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PMIC, 1),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 1),
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]){ {
                                         .frequency = 500 * FWK_MHZ,
                                         .voltage = 830000,
                                     },
                                     {
                                         .frequency = 1000 * FWK_MHZ,
                                         .voltage = 830000,
                                     },
                                     {
                                         .frequency = 1500 * FWK_MHZ,
                                         .voltage = 830000,
                                     },
#if 0 /* The prototype does not support boost mode. */
        {
            .frequency = 1600 * FWK_MHZ,
            .voltage = 900000,
        },
        {
            .frequency = 1700 * FWK_MHZ,
            .voltage = 960000,
        },
#endif
                                     { 0 } }
};

static const struct fwk_element element_table[] = {
    [DVFS_ELEMENT_IDX_LITTLE] =
        {
            .name = "CPU_GROUP_LITTLE",
            .data = &cpu_group_little,
        },
    [DVFS_ELEMENT_IDX_BIG] =
        {
            .name = "CPU_GROUP_BIG",
            .data = &cpu_group_big,
        },
    { 0 }
};

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_rcar_dvfs = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table),
    .data = NULL,
};
