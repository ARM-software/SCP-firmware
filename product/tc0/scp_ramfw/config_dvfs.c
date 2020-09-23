/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "tc0_timer.h"

#include <mod_dvfs.h>
#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct mod_dvfs_opp opps[] = { {
                                          .level = 946 * 1000000UL,
                                          .frequency = 946 * FWK_KHZ,
                                          .voltage = 550,
                                      },
                                      {
                                          .level = 1419 * 1000000UL,
                                          .frequency = 1419 * FWK_KHZ,
                                          .voltage = 650,
                                      },
                                      {
                                          .level = 1893 * 1000000UL,
                                          .frequency = 1893 * FWK_KHZ,
                                          .voltage = 750,
                                      },
                                      {
                                          .level = 2271 * 1000000UL,
                                          .frequency = 2271 * FWK_KHZ,
                                          .voltage = 850,
                                      },
                                      {
                                          .level = 2650 * 1000000UL,
                                          .frequency = 2650 * FWK_KHZ,
                                          .voltage = 950,
                                      },
                                      { 0 } };

static const struct mod_dvfs_domain_config cpu_group = {
    .psu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_CPU_GROUP0),
    .alarm_id =
        FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, CONFIG_TIMER_DVFS_CPU),
    .notification_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_PERF),
    .updates_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_PERF,
        MOD_SCMI_PERF_DVFS_UPDATE_API),
    .retry_ms = 1,
    .latency = 1200,
    .sustained_idx = 2,
    .opps = opps,
};

static const struct fwk_element element_table[] = { [0] =
                                                        {
                                                            .name = "CPU_GROUP",
                                                            .data = &cpu_group,
                                                        },
                                                    { 0 } };

static const struct fwk_element *dvfs_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

const struct fwk_module_config config_dvfs = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dvfs_get_element_table),
};
