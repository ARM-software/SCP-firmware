/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"

#include <mod_dvfs.h>
#include <mod_sid.h>
#include <mod_scmi_perf.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

static const struct mod_dvfs_domain_config cpu_group_little = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 1),
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]){ {
                                         .level = 665 * 1000000UL,
                                         .frequency = 665 * FWK_KHZ,
                                         .voltage = 800,
                                     },
                                     {
                                         .level = 998 * 1000000UL,
                                         .frequency = 998 * FWK_KHZ,
                                         .voltage = 850,
                                     },
                                     {
                                         .level = 1330 * 1000000UL,
                                         .frequency = 1330 * FWK_KHZ,
                                         .voltage = 900,
                                     },
                                     {
                                         .level = 1463 * 1000000UL,
                                         .frequency = 1463 * FWK_KHZ,
                                         .voltage = 950,
                                     },
                                     {
                                         .level = 2200 * 1000000UL,
                                         .frequency = 2200 * FWK_KHZ,
                                         .voltage = 1000,
                                     },
                                     { 0 } }
};

static const struct mod_dvfs_domain_config cpu_group_big = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 1),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 0),
    .latency = 1200,
    .sustained_idx = 2,
    .opps = (struct mod_dvfs_opp[]){ {
                                         .level = 1313 * 1000000UL,
                                         .frequency = 1313 * FWK_KHZ,
                                         .voltage = 800,
                                     },
                                     {
                                         .level = 1531 * 1000000UL,
                                         .frequency = 1531 * FWK_KHZ,
                                         .voltage = 850,
                                     },
                                     {
                                         .level = 1750 * 1000000UL,
                                         .frequency = 1750 * FWK_KHZ,
                                         .voltage = 900,
                                     },
                                     {
                                         .level = 2100 * 1000000UL,
                                         .frequency = 2100 * FWK_KHZ,
                                         .voltage = 950,
                                     },
                                     {
                                         .level = 2700 * 1000000UL,
                                         .frequency = 2700 * FWK_KHZ,
                                         .voltage = 1000,
                                     },
                                     { 0 } }
};

static const struct mod_dvfs_domain_config gpu = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 2),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, 2),
    .latency = 1200,
    .sustained_idx = 4,
    .opps = (struct mod_dvfs_opp[]){ {
                                         .level = 450 * 1000000UL,
                                         .frequency = 450 * FWK_KHZ,
                                         .voltage = 800,
                                     },
                                     {
                                         .level = 487500 * 1000UL,
                                         .frequency = 487500,
                                         .voltage = 850,
                                     },
                                     {
                                         .level = 525 * 1000000UL,
                                         .frequency = 525 * FWK_KHZ,
                                         .voltage = 900,
                                     },
                                     {
                                         .level = 562500 * 1000UL,
                                         .frequency = 562500,
                                         .voltage = 950,
                                     },
                                     {
                                         .level = 800 * 1000000UL,
                                         .frequency = 800 * FWK_KHZ,
                                         .voltage = 1000,
                                     },
                                     { 0 } }
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
    int status;
    const struct mod_sid_info *system_info;
    double big_cpu_core_coefficient;
    unsigned int big_cpu_core_count;
    double little_cpu_core_coefficient;
    unsigned int little_cpu_core_count;
    double gpu_core_coefficient;
    unsigned int gpu_core_count;
    struct mod_dvfs_opp *opp;

    status = mod_sid_get_system_info(&system_info);
    fwk_assert(status == FWK_SUCCESS);

    big_cpu_core_coefficient = 0.08;
    little_cpu_core_coefficient = 0.338;
    gpu_core_coefficient = 0.604;

    switch (system_info->config_number) {
    case 1:
    case 5:
        big_cpu_core_count = 2;
        little_cpu_core_count = 6;
        gpu_core_count = 12;
        break;
    case 2:
    case 6:
        big_cpu_core_count = 4;
        little_cpu_core_count = 4;
        gpu_core_count = 12;
        break;
    case 3:
    case 4:
        big_cpu_core_count = 4;
        little_cpu_core_count = 4;
        gpu_core_count = 16;
        break;
    case 7:
        big_cpu_core_count = 2;
        little_cpu_core_count = 4;
        gpu_core_count = 16;
        break;
    case 8:
        big_cpu_core_count = 2;
        little_cpu_core_count = 4;
        gpu_core_count = 12;
        break;
    default:
        fwk_unreachable();
    }

    /*
     * The power cost figures below are built using the dynamic power
     * consumption formula (P = CfV^2), where C represents the capacitance of
     * one processing element in the domain (a core or shader core). This power
     * figure is scaled linearly with the number of processing elements in the
     * performance domain to give a rough representation of the overall power
     * draw. The capacitance constants are given in mW/MHz/V^2.
     */
    for (opp = cpu_group_little.opps; opp->frequency != 0; opp++) {
        opp->power = little_cpu_core_coefficient * little_cpu_core_count *
            (opp->frequency / FWK_KHZ) * (opp->voltage * opp->voltage) /
            1000000;
    }

    for (opp = cpu_group_big.opps; opp->frequency != 0; opp++) {
        opp->power = big_cpu_core_coefficient * big_cpu_core_count *
            (opp->frequency / FWK_KHZ) * (opp->voltage * opp->voltage) /
            1000000;
    }

    for (opp = gpu.opps; opp->frequency != 0; opp++) {
        opp->power = gpu_core_coefficient * gpu_core_count *
            (opp->frequency / FWK_KHZ) * (opp->voltage * opp->voltage) /
            1000000;
    }

    return element_table;
}

const struct fwk_module_config config_dvfs = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table),
};
