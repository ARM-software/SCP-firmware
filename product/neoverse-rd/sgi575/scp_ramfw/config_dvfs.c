/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"

#include <mod_dvfs.h>
#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct mod_dvfs_opp opps[] = { {
                                          .level = 1313 * 1000000UL,
                                          .frequency = 1313 * FWK_KHZ,
                                          .voltage = 100,
                                      },
                                      {
                                          .level = 1531 * 1000000UL,
                                          .frequency = 1531 * FWK_KHZ,
                                          .voltage = 200,
                                      },
                                      {
                                          .level = 1750 * 1000000UL,
                                          .frequency = 1750 * FWK_KHZ,
                                          .voltage = 300,
                                      },
                                      {
                                          .level = 2100 * 1000000UL,
                                          .frequency = 2100 * FWK_KHZ,
                                          .voltage = 400,
                                      },
                                      {
                                          .level = 2600 * 1000000UL,
                                          .frequency = 2600 * FWK_KHZ,
                                          .voltage = 500,
                                      },
                                      { 0 } };

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
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table),
};
