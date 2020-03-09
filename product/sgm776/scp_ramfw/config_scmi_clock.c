/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_devices.h"
#include "sgm776_scmi.h"

#include <mod_scmi_clock.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct mod_scmi_clock_device agent_device_table_ospm[] = {
    {
        /* VPU */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_VPU),
        .permissions = MOD_SCMI_CLOCK_PERM_ATTRIBUTES |
                       MOD_SCMI_CLOCK_PERM_DESCRIBE_RATES |
                       MOD_SCMI_CLOCK_PERM_GET_RATE |
                       MOD_SCMI_CLOCK_PERM_SET_RATE,
    },
    {
        /* DPU */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_DPU),
        .permissions = MOD_SCMI_CLOCK_PERM_ATTRIBUTES |
                       MOD_SCMI_CLOCK_PERM_DESCRIBE_RATES |
                       MOD_SCMI_CLOCK_PERM_GET_RATE |
                       MOD_SCMI_CLOCK_PERM_SET_RATE,
    },
    {
        /* PIXEL_0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_PIXEL_0),
        .permissions = MOD_SCMI_CLOCK_PERM_ATTRIBUTES |
                       MOD_SCMI_CLOCK_PERM_DESCRIBE_RATES |
                       MOD_SCMI_CLOCK_PERM_GET_RATE |
                       MOD_SCMI_CLOCK_PERM_SET_RATE,
    },
    {
        /* PIXEL_1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_PIXEL_1),
        .permissions = MOD_SCMI_CLOCK_PERM_ATTRIBUTES |
                       MOD_SCMI_CLOCK_PERM_DESCRIBE_RATES |
                       MOD_SCMI_CLOCK_PERM_GET_RATE |
                       MOD_SCMI_CLOCK_PERM_SET_RATE,
    },
};

static const struct mod_scmi_clock_agent agent_table[SCMI_AGENT_ID_COUNT] = {
    [SCMI_AGENT_ID_PSCI] = { 0 /* No access */ },
    [SCMI_AGENT_ID_OSPM] = {
        .device_table = agent_device_table_ospm,
        .device_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
    },
};

const struct fwk_module_config config_scmi_clock = {
    .data = &((struct mod_scmi_clock_config) {
        .max_pending_transactions = 0,
        .agent_table = agent_table,
        .agent_count = FWK_ARRAY_SIZE(agent_table),
    }),
};
