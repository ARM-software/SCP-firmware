/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_scmi_clock.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define FAKE_MODULE_IDX 0x5
#define FAKE_SCMI_AGENT_IDX_PSCI 0x1
#define FAKE_SCMI_AGENT_IDX_OSPM 0x2
#define FAKE_SCMI_AGENT_IDX_COUNT 0x3

/*!
 * \brief Clock device indexes.
 */
enum clock_dev_idx {
    CLOCK_DEV_IDX_VPU,
    CLOCK_DEV_IDX_DPU,
    CLOCK_DEV_IDX_PIXEL_0,
    CLOCK_DEV_IDX_PIXEL_1,
    CLOCK_DEV_IDX_COUNT
};

static const struct mod_scmi_clock_device agent_device_table_ospm[] = {
    {
        /* VPU */
        .element_id =
            FWK_ID_ELEMENT_INIT(FAKE_MODULE_IDX, CLOCK_DEV_IDX_VPU),
        .starts_enabled = true,
    },
    {
        /* DPU */
        .element_id =
            FWK_ID_ELEMENT_INIT(FAKE_MODULE_IDX, CLOCK_DEV_IDX_DPU),
        .starts_enabled = true,
    },
    {
        /* PIXEL_0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FAKE_MODULE_IDX, CLOCK_DEV_IDX_PIXEL_0),
        .starts_enabled = true,
    },
    {
        /* PIXEL_1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FAKE_MODULE_IDX, CLOCK_DEV_IDX_PIXEL_1),
        .starts_enabled = true,
    },
};

static const struct mod_scmi_clock_agent agent_table[FAKE_SCMI_AGENT_IDX_COUNT] = {
    [FAKE_SCMI_AGENT_IDX_PSCI] = { 0 /* No access */ },
    [FAKE_SCMI_AGENT_IDX_OSPM] = {
        .device_table = agent_device_table_ospm,
        .device_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
    },
};

struct fwk_module_config config_scmi_clock = {
    .data = &((struct mod_scmi_clock_config) {
        .max_pending_transactions = 0,
        .agent_table = agent_table,
        .agent_count = FWK_ARRAY_SIZE(agent_table),
    }),
};
