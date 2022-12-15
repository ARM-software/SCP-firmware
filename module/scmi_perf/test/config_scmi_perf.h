/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_scmi_perf.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

#define TEST_MODULE_IDX       0x5
#define TEST_SCMI_AGENT_IDX_0 0x1

#define TEST_OPP_COUNT 0x5

enum scmi_perf_element_idx {
    SCMI_PERF_ELEMENT_IDX_0 = 0,
    SCMI_PERF_ELEMENT_IDX_1,
    SCMI_PERF_ELEMENT_IDX_2,
    SCMI_PERF_ELEMENT_IDX_COUNT,
};

enum dvfs_element_idx {
    DVFS_ELEMENT_IDX_0 = SCMI_PERF_ELEMENT_IDX_0,
    DVFS_ELEMENT_IDX_1 = SCMI_PERF_ELEMENT_IDX_1,
    DVFS_ELEMENT_IDX_COUNT,
};

#define PERF_DOMAINS_IDX_COUNT DVFS_ELEMENT_IDX_COUNT
static const struct mod_scmi_perf_domain_config domains[] = {
    [SCMI_PERF_ELEMENT_IDX_0] = {
        .fast_channels_addr_scp =
            (uint64_t[]){
                [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] = 1,
            },
        .fast_channels_addr_ap =
            (uint64_t[]){
                [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] = 1,
            },
        .phy_group_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0),
    },
    [SCMI_PERF_ELEMENT_IDX_1] = {
        .phy_group_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1),
    },
    [SCMI_PERF_ELEMENT_IDX_2] = {
        .phy_group_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1),
    },
};

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
enum plugins_list {
    PERF_PLUGIN_IDX_0,
    PERF_PLUGIN_IDX_COUNT,
};

static struct mod_scmi_plugin_config plugins_table[] = {
    [PERF_PLUGIN_IDX_0] = {
        .id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PERF_PLUGIN),
        .dom_type = PERF_PLUGIN_DOM_TYPE_PHYSICAL,
    },
};
#endif

static struct mod_scmi_perf_config perf_config = {
    .domains = &domains,
    .perf_doms_count = SCMI_PERF_ELEMENT_IDX_COUNT,
    .fast_channels_alarm_id = FWK_ID_NONE_INIT,
#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
    .plugins = plugins_table,
#endif
};

struct fwk_module_config config_scmi_perf = {
    .data = &perf_config,
};

static const struct mod_dvfs_domain_config test_dvfs_config = {
    .latency = 1200,
    .sustained_idx = 2,
    .opps =
        (struct mod_dvfs_opp[]){
            {
                .level = 100 * 1000000UL,
                .frequency = 101 * FWK_KHZ,
                .voltage = 102,
            },
            {
                .level = 200 * 1000000UL,
                .frequency = 201 * FWK_KHZ,
                .voltage = 202,
            },
            {
                .level = 300 * 1000000UL,
                .frequency = 301 * FWK_KHZ,
                .voltage = 302,
            },
            {
                .level = 400 * 1000000UL,
                .frequency = 401 * FWK_KHZ,
                .voltage = 402,
            },
            {
                .level = 500 * 1000000UL,
                .frequency = 501 * FWK_KHZ,
                .voltage = 502,
            },
            { 0 },
        }
};

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
static struct perf_plugins_api test_perf_plugins_api = {
    .update = plugin_update,
    .report = plugin_report,
};
#endif
