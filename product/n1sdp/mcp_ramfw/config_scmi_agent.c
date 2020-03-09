/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_mcp_scmi.h"

#include <mod_scmi_agent.h>
#include <mod_smt.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>

static const struct fwk_element agent_table[] = {
    [MCP_N1SDP_SCMI_AGENT_IDX_MANAGEMENT] = {
        .name = "MCP-AGENT",
        .data = &((struct mod_scmi_agent_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_N1SDP_SMT,
                MCP_N1SDP_SCMI_AGENT_IDX_MANAGEMENT),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_N1SDP_SMT,
                MOD_SMT_API_IDX_SCMI_AGENT_TRANSPORT),
        }),
    },
    [MCP_N1SDP_SCMI_AGENT_IDX_COUNT] = { 0 }
};

static const struct fwk_element *get_agent_table(fwk_id_t module_id)
{
    return agent_table;
}

const struct fwk_module_config config_scmi_agent = {
    .get_element_table = get_agent_table,
    .data = NULL,
};
