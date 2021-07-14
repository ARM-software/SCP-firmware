/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tc0_scmi.h"

#include <mod_scmi_voltage_domain.h>
#include <mod_voltage_domain.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>

static struct mod_scmi_voltd_device scmi_voltd_device[] = {
    {
        .element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_VOLTAGE_DOMAIN, 0),
        .config = 0,
    },
};

static const struct mod_scmi_voltd_agent scmi_voltd_agent_table[SCP_SCMI_AGENT_ID_COUNT] = {
    [SCP_SCMI_AGENT_ID_OSPM] = {
        .device_table = scmi_voltd_device,
        .domain_count = FWK_ARRAY_SIZE(scmi_voltd_device),
    },
    [SCP_SCMI_AGENT_ID_PSCI] = { 0 /* No access */ },
};

struct fwk_module_config config_scmi_voltage_domain = {
    .data = &((struct mod_scmi_voltd_config){
        .agent_table = scmi_voltd_agent_table,
        .agent_count = FWK_ARRAY_SIZE(scmi_voltd_agent_table),
    }),
};
