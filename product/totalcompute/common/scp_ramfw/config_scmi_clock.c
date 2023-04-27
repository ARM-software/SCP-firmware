/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "tc_scmi.h"

#include <mod_scmi_clock.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct mod_scmi_clock_device agent_device_table_ospm[3] = {
    {
        /* DPU */
        .element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_DPU),
    },
    {
        /* PIXEL_0 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_PIXEL_0),
    },
    {
        /* PIXEL_1 */
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_IDX_PIXEL_1),
    },
};

static const struct mod_scmi_clock_agent agent_table[SCP_SCMI_AGENT_ID_COUNT] = {
    [SCP_SCMI_AGENT_ID_PSCI] = { 0 /* No access */ },
    [SCP_SCMI_AGENT_ID_OSPM] = {
            .device_table = agent_device_table_ospm,
            .device_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
    },
};

const struct fwk_module_config config_scmi_clock = {
    .data = &((struct mod_scmi_clock_config){
        .max_pending_transactions = 0,
        .agent_table = agent_table,
        .agent_count = FWK_ARRAY_SIZE(agent_table),
    }),
};
