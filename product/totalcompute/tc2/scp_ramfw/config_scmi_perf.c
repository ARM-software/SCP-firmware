/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tc2_dvfs.h"
#include "tc2_timer.h"
#include "tc_scmi.h"

#include <scp_mmap.h>

#include <mod_scmi_perf.h>

#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

#define FC_LEVEL_SET_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LIMIT_SET_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LEVEL_GET_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LIMIT_GET_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LEVEL_SET_AP_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

#define FC_LIMIT_SET_AP_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

#define FC_LEVEL_GET_AP_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

#define FC_LIMIT_GET_AP_ADDR(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - \
     SCP_SYSTEM_ACCESS_PORT1_BASE)

static const struct mod_scmi_perf_domain_config
    domains[DVFS_ELEMENT_IDX_COUNT] = {
        [DVFS_ELEMENT_IDX_HAYES] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
         .fast_channels_addr_scp = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET_ADDR(DVFS_ELEMENT_IDX_HAYES),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET_ADDR(DVFS_ELEMENT_IDX_HAYES),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET_ADDR(DVFS_ELEMENT_IDX_HAYES),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET_ADDR(DVFS_ELEMENT_IDX_HAYES),
        },
         .fast_channels_addr_ap = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET_AP_ADDR(DVFS_ELEMENT_IDX_HAYES),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET_AP_ADDR(DVFS_ELEMENT_IDX_HAYES),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET_AP_ADDR(DVFS_ELEMENT_IDX_HAYES),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET_AP_ADDR(DVFS_ELEMENT_IDX_HAYES),
        },
#endif
        },
        [DVFS_ELEMENT_IDX_HUNTER] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
         .fast_channels_addr_scp = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET_ADDR(DVFS_ELEMENT_IDX_HUNTER),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET_ADDR(DVFS_ELEMENT_IDX_HUNTER),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET_ADDR(DVFS_ELEMENT_IDX_HUNTER),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET_ADDR(DVFS_ELEMENT_IDX_HUNTER),
        },
         .fast_channels_addr_ap = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET_AP_ADDR(DVFS_ELEMENT_IDX_HUNTER),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET_AP_ADDR(DVFS_ELEMENT_IDX_HUNTER),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET_AP_ADDR(DVFS_ELEMENT_IDX_HUNTER),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET_AP_ADDR(DVFS_ELEMENT_IDX_HUNTER),
        },
#endif
        },
        [DVFS_ELEMENT_IDX_HUNTER_ELP] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
         .fast_channels_addr_scp = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET_ADDR(DVFS_ELEMENT_IDX_HUNTER_ELP),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET_ADDR(DVFS_ELEMENT_IDX_HUNTER_ELP),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET_ADDR(DVFS_ELEMENT_IDX_HUNTER_ELP),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET_ADDR(DVFS_ELEMENT_IDX_HUNTER_ELP),
        },
         .fast_channels_addr_ap = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET_AP_ADDR(DVFS_ELEMENT_IDX_HUNTER_ELP),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET_AP_ADDR(DVFS_ELEMENT_IDX_HUNTER_ELP),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET_AP_ADDR(DVFS_ELEMENT_IDX_HUNTER_ELP),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET_AP_ADDR(DVFS_ELEMENT_IDX_HUNTER_ELP),
        },
#endif
        },
    };

#if (PLATFORM_VARIANT == TC2_VAR_EXPERIMENT_POWER)
static const struct mod_scmi_plugin_config plugins_table[] = {
    [0] = {
        .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TRAFFIC_COP),
        .dom_type = PERF_PLUGIN_DOM_TYPE_PHYSICAL,
    },
    [1] = {
        .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MPMM),
        .dom_type = PERF_PLUGIN_DOM_TYPE_PHYSICAL,
    },
    [2] = {
        .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_THERMAL_MGMT),
        .dom_type = PERF_PLUGIN_DOM_TYPE_FULL,
    },
};
#endif

const struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config) {
        .domains = &domains, .perf_doms_count = FWK_ARRAY_SIZE(domains),
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fast_channels_alarm_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_TIMER, 0, TC2_CONFIG_TIMER_FAST_CHANNEL_TIMER_IDX),
        .fast_channels_rate_limit = (4 * 1000),
#else
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
#endif
#if (PLATFORM_VARIANT == TC2_VAR_EXPERIMENT_POWER)
        .plugins = plugins_table,
        .plugins_count = FWK_ARRAY_SIZE(plugins_table),
#endif
    }),
};
