/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"

#include <mod_dvfs.h>
#include <mod_sid.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

static struct mod_dvfs_opp opps[] = {
        {
            .frequency = 1313 * FWK_MHZ,
            .voltage = 800,
        },
        {
            .frequency = 1531 * FWK_MHZ,
            .voltage = 850,
        },
        {
            .frequency = 1750 * FWK_MHZ,
            .voltage = 900,
        },
        {
            .frequency = 2100 * FWK_MHZ,
            .voltage = 950,
        },
        {
            .frequency = 2600 * FWK_MHZ,
            .voltage = 1000,
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
    int status;
    const struct mod_sid_info *system_info;
    double cluster_core_coefficient;
    unsigned int cluster_core_count;
    struct mod_dvfs_opp *opp;

    status = mod_sid_get_system_info(&system_info);
    fwk_assert(status == FWK_SUCCESS);

    switch (system_info->config_number) {
    case 1:
        cluster_core_count = 4;
        cluster_core_coefficient = 0.338;
        break;
    case 2:
        cluster_core_count = 8;
        cluster_core_coefficient = 0.107;
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
    for (opp = opps; opp->frequency != 0; opp++) {
        opp->power = cluster_core_coefficient *
                     cluster_core_count *
                     (opp->frequency / FWK_MHZ) *
                     (opp->voltage * opp->voltage) / 1000000;
    }

    return element_table;
}

const struct fwk_module_config config_dvfs = {
    .get_element_table = dvfs_get_element_table,
};
