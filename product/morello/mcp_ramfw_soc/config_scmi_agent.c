/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_mcp_scmi.h"

#include <mod_scmi.h>
#include <mod_scmi_agent.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

enum scmi_agent_element_idx {
    SCMI_AGENT_MCP_ELEMENT,
    SCMI_AGENT_ELEMENT_COUNT,
};

static const struct fwk_element
    scmi_agent_element_table[SCMI_AGENT_ELEMENT_COUNT+1] = {
        [SCMI_AGENT_MCP_ELEMENT] = {
            .name = "MCP-AGENT",
            .data = &((struct mod_scmi_agent_config) {
                .service_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI,
                        MCP_MORELLO_SCMI_SERVICE_IDX_SCP),
            }),
        },
        [SCMI_AGENT_ELEMENT_COUNT] = { 0 }
};

static const struct fwk_element *get_scmi_agent_element_table(
    fwk_id_t module_id)
{
    return scmi_agent_element_table;
}

const struct fwk_module_config config_scmi_agent = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_scmi_agent_element_table),
    .data = NULL,
};
