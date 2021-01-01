/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "software_mmap.h"
#include "juno_alarm_idx.h"

#include <mod_stats.h>

#include <fwk_module.h>

static const struct mod_stats_config_info stats_config_info = {
    .ap_stats_addr = SCMI_PERF_STATS_BASE - EXTERNAL_DEV_BASE,
    .scp_stats_addr = SCMI_PERF_STATS_BASE,
    .stats_region_size = SCMI_PERF_STATS_SIZE,
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
        JUNO_STATISTICS_ALARM_IDX),
};

struct fwk_module_config config_statistics = {
    .data = &stats_config_info,
};
