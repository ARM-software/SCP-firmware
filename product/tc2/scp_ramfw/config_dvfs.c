/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "tc2_dvfs.h"
#include "tc2_psu.h"
#include "tc2_timer.h"

#include <mod_dvfs.h>
#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/*
 * The power cost figures in this file are built using the dynamic power
 * consumption formula (P = CfV^2), where C represents the capacitance of one
 * processing element in the domain (a core or shader core). This power figure
 * is scaled linearly with the number of processing elements in the performance
 * domain to give a rough representation of the overall power draw. The
 * capacitance constants are given in mW/MHz/V^2 and were taken from the Linux
 * device trees, which provide a dynamic-power-coefficient field in uW/MHz/V^2.
 * This conversion of units, from uW/MHz/V^2 to mW/MHz/V^2, is done by dividing
 * by 1000.
 */

/* dynamic-power-coeffient/1000 */
#define HAYES_DPC      0.230
#define HUNTER_DPC     0.495
#define HUNTER_ELP_DPC 1.054

static struct mod_dvfs_opp operating_points_hayes[6] = {
    {
        .level = 768 * 1000000UL,
        .frequency = 768 * FWK_KHZ,
        .voltage = 550,
        .power = (uint32_t)(HAYES_DPC * 768 * 0.550 * 0.550),
    },
    {
        .level = 1153 * 1000000UL,
        .frequency = 1153 * FWK_KHZ,
        .voltage = 650,
        .power = (uint32_t)(HAYES_DPC * 1153 * 0.650 * 0.650),
    },
    {
        .level = 1537 * 1000000UL,
        .frequency = 1537 * FWK_KHZ,
        .voltage = 750,
        .power = (uint32_t)(HAYES_DPC * 1537 * 0.750 * 0.750),
    },
    {
        .level = 1844 * 1000000UL,
        .frequency = 1844 * FWK_KHZ,
        .voltage = 850,
        .power = (uint32_t)(HAYES_DPC * 1844 * 0.850 * 0.850),
    },
    {
        .level = 2152 * 1000000UL,
        .frequency = 2152 * FWK_KHZ,
        .voltage = 950,
        .power = (uint32_t)(HAYES_DPC * 2152 * 0.950 * 0.950),
    },
    { 0 }
};

static struct mod_dvfs_opp operating_points_hunter[6] = {
    {
        .level = 946 * 1000000UL,
        .frequency = 946 * FWK_KHZ,
        .voltage = 550,
        .power = (uint32_t)(HUNTER_DPC * 946 * 0.550 * 0.550),
    },
    {
        .level = 1419 * 1000000UL,
        .frequency = 1419 * FWK_KHZ,
        .voltage = 650,
        .power = (uint32_t)(HUNTER_DPC * 1419 * 0.650 * 0.650),
    },
    {
        .level = 1893 * 1000000UL,
        .frequency = 1893 * FWK_KHZ,
        .voltage = 750,
        .power = (uint32_t)(HUNTER_DPC * 1893 * 0.750 * 0.750),
    },
    {
        .level = 2271 * 1000000UL,
        .frequency = 2271 * FWK_KHZ,
        .voltage = 850,
        .power = (uint32_t)(HUNTER_DPC * 2271 * 0.850 * 0.850),
    },
    {
        .level = 2650 * 1000000UL,
        .frequency = 2650 * FWK_KHZ,
        .voltage = 950,
        .power = (uint32_t)(HUNTER_DPC * 2650 * 0.950 * 0.950),
    },
    { 0 }
};

static struct mod_dvfs_opp operating_points_hunter_elp[6] = {
    {
        .level = 1088 * 1000000UL,
        .frequency = 1088 * FWK_KHZ,
        .voltage = 550,
        .power = (uint32_t)(HUNTER_ELP_DPC * 1088 * 0.550 * 0.550),
    },
    {
        .level = 1632 * 1000000UL,
        .frequency = 1632 * FWK_KHZ,
        .voltage = 650,
        .power = (uint32_t)(HUNTER_ELP_DPC * 1632 * 0.650 * 0.650),
    },
    {
        .level = 2176 * 1000000UL,
        .frequency = 2176 * FWK_KHZ,
        .voltage = 750,
        .power = (uint32_t)(HUNTER_ELP_DPC * 2176 * 0.750 * 0.750),
    },
    {
        .level = 2612 * 1000000UL,
        .frequency = 2612 * FWK_KHZ,
        .voltage = 850,
        .power = (uint32_t)(HUNTER_ELP_DPC * 2612 * 0.850 * 0.850),
    },
    {
        .level = 3047 * 1000000UL,
        .frequency = 3047 * FWK_KHZ,
        .voltage = 950,
        .power = (uint32_t)(HUNTER_ELP_DPC * 3047 * 0.950 * 0.950),
    },
    { 0 }
};

static const struct mod_dvfs_domain_config cpu_group_hayes = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, PSU_ELEMENT_IDX_HAYES),
    .clock_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP_HAYES),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_TIMER,
        0,
        TC2_CONFIG_TIMER_DVFS_CPU_HAYES),
    .retry_ms = 1,
    .latency = 1200,
    .sustained_idx = 2,
    .opps = operating_points_hayes,
};

static const struct mod_dvfs_domain_config cpu_group_hunter = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, PSU_ELEMENT_IDX_HUNTER),
    .clock_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP_HUNTER),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_TIMER,
        0,
        TC2_CONFIG_TIMER_DVFS_CPU_HUNTER),
    .retry_ms = 1,
    .latency = 1200,
    .sustained_idx = 2,
    .opps = operating_points_hunter,
};

static const struct mod_dvfs_domain_config cpu_group_hunter_elp = {
    .psu_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, PSU_ELEMENT_IDX_HUNTER_ELP),
    .clock_id = FWK_ID_ELEMENT_INIT(
        FWK_MODULE_IDX_CLOCK,
        CLOCK_IDX_CPU_GROUP_HUNTER_ELP),
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_TIMER,
        0,
        TC2_CONFIG_TIMER_DVFS_CPU_HUNTER_ELP),
    .retry_ms = 1,
    .latency = 1200,
    .sustained_idx = 2,
    .opps = operating_points_hunter_elp,
};

static const struct fwk_element element_table[DVFS_ELEMENT_IDX_COUNT + 1] = {
    [DVFS_ELEMENT_IDX_HAYES] =
        {
            .name = "CPU_GROUP_HAYES",
            .data = &cpu_group_hayes,
        },
    [DVFS_ELEMENT_IDX_HUNTER] =
        {
            .name = "CPU_GROUP_HUNTER",
            .data = &cpu_group_hunter,
        },
    [DVFS_ELEMENT_IDX_HUNTER_ELP] =
        {
            .name = "CPU_GROUP_HUNTER_ELP",
            .data = &cpu_group_hunter_elp,
        },
    { 0 },
};

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

const struct fwk_module_config config_dvfs = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table),
};
