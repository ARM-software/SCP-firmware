/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <scmi_agents.h>
#include <mod_scmi_clock.h>
#include "clock_devices.h"

static const struct mod_scmi_clock_device agent_device_table_ospm[] = {
    {
        /* MOCK_0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_MOCK_0),
        .starts_enabled = true,
    },
    {
        /* MOCK_1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_MOCK_1),
        .starts_enabled = true,
    },
    {
        /* VPU */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_VPU),
        .starts_enabled = true,
    },
    {
        /* MOCK_3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_MOCK_3),
        .starts_enabled = true,
    },
    {
        /* DPU */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_DPU),
        .starts_enabled = true,
    },
    {
        /* PIXEL_0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_PIXEL_0),
        .starts_enabled = true,
    },
    {
        /* PIXEL_1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_PIXEL_1),
        .starts_enabled = true,
    },
};

static const struct mod_scmi_clock_device agent_device_table_perf[] = {
    {
        /* MOCK_0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_MOCK_0),
    },
    {
        /* MOCK_2 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_MOCK_2),
    },
    {
        /* MOCK_3 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_MOCK_3),
    },
    {
        /* PIXEL_0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_PIXEL_0),
    },
    {
        /* PIXEL_1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_PIXEL_1),
    },
};

static const struct mod_scmi_clock_agent agent_table[SCMI_AGENT_ID_COUNT] = {
    [SCMI_AGENT_ID_OSPM] = {
        .device_table = agent_device_table_ospm,
        .device_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
    },
    [SCMI_AGENT_ID_PSCI] = { 0 /* No access */ },
    [SCMI_AGENT_ID_PERF] = {
        .device_table = agent_device_table_perf,
        .device_count = FWK_ARRAY_SIZE(agent_device_table_perf),
    },
};

struct fwk_module_config config_scmi_clock = {
    .data = &((struct mod_scmi_clock_config) {
        .max_pending_transactions = 0,
        .agent_table = agent_table,
        .agent_count = FWK_ARRAY_SIZE(agent_table),
    }),
};
