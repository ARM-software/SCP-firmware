/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_dvfs.h>
#include <mod_scmi_perf.h>
#include <config_psu.h>
#include <config_dvfs.h>
#include "rcar_alarm_idx.h"

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

/*
 * The power cost figures from this file are built using the dynamic power
 * consumption formula (P = CfV^2), where C represents the capacitance of one
 * processing element in the domain (a core or shader core). This power figure
 * is scaled linearly with the number of processing elements in the performance
 * domain to give a rough representation of the overall power draw. The
 * capacitance constants are given in mW/MHz/V^2 and were taken from the Linux
 * device trees, which provide a dynamic-power-coefficient field in uW/MHz/V^2.
 */

static const struct mod_dvfs_domain_config cpu_group_little_r0 = {
    .psu_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_ELEMENT_IDX_VLITTLE),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 0),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_TIMER,
        0,
        RCAR_DVFS_ALARM_LITTLE_IDX),
    .retry_ms = 1,
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]){ {
                                         .level = 800 * 1000000UL,
                                         .frequency = 800 * FWK_KHZ,
                                         .voltage = 820000,
                                         .power = (0.14 * 800 * 0.820 * 0.820),
                                     },
                                     {
                                         .level = 1000 * 1000000UL,
                                         .frequency = 1000 * FWK_KHZ,
                                         .voltage = 820000,
                                         .power = (0.14 * 1000 * 0.950 * 0.950),
                                     },
                                     {
                                         .level = 1200 * 1000000UL,
                                         .frequency = 1200 * FWK_KHZ,
                                         .voltage = 820000,
                                         .power = (0.14 * 1200 * 1.000 * 1.000),
                                     },
                                     { 0 } }
};

static const struct mod_dvfs_domain_config cpu_group_big_r0 = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_ELEMENT_IDX_VBIG),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 1),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_TIMER,
        0,
        RCAR_DVFS_ALARM_BIG_IDX),
    .retry_ms = 1,
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]){ {
                                         .level = 500 * 1000000UL,
                                         .frequency = 500 * FWK_KHZ,
                                         .voltage = 830000,
                                         .power = (0.53 * 500 * 0.820 * 0.820),
                                     },
                                     {
                                         .level = 1000 * 1000000UL,
                                         .frequency = 1000 * FWK_KHZ,
                                         .voltage = 830000,
                                         .power = (0.53 * 1000 * 0.850 * 0.850),
                                     },
                                     {
                                         .level = 1500 * 1000000UL,
                                         .frequency = 1500 * FWK_KHZ,
                                         .voltage = 830000,
                                         .power = (0.53 * 1500 * 0.900 * 0.900),
                                     },
                                     {
                                         .level = 1600 * 1000000UL,
                                         .frequency = 1600 * FWK_KHZ,
                                         .voltage = 900000,
                                         .power = (0.53 * 1600 * 0.950 * 0.950),
                                     },
                                     {
                                         .level = 1700 * 1000000UL,
                                         .frequency = 1700 * FWK_KHZ,
                                         .voltage = 960000,
                                         .power = (0.53 * 1700 * 1.000 * 1.000),
                                     },
                                     { 0 } }
};

static const struct fwk_element element_table_r0[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .name = "LITTLE_CPU",
        .data = &cpu_group_little_r0,
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .name = "BIG_CPU",
        .data = &cpu_group_big_r0,
    },
    { 0 }
};

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    return element_table_r0;
}

struct fwk_module_config config_dvfs = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table),
};
