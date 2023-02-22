/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_mcp_scmi.h"

#include <mod_scmi.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element
    service_table[MCP_MORELLO_SCMI_SERVICE_IDX_COUNT+1] = {
        [MCP_MORELLO_SCMI_SERVICE_IDX_SCP] = {
            .name = "MCP-AGENT",
            .data = &((struct mod_scmi_service_config) {
                .transport_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_TRANSPORT,
                    MCP_MORELLO_SCMI_SERVICE_IDX_SCP),
                .transport_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_TRANSPORT,
                    MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
                .transport_notification_init_id = FWK_ID_NONE_INIT,
                .scmi_agent_id = MCP_MORELLO_SCMI_AGENT_IDX_MANAGEMENT,
                .scmi_entity_role = MOD_SCMI_ROLE_AGENT,
            }),
        },
        [MCP_MORELLO_SCMI_SERVICE_IDX_COUNT] = { 0 }
};

static const struct fwk_element *get_service_table(fwk_id_t module_id)
{
    return service_table;
}

static struct mod_scmi_agent agent_table[MCP_MORELLO_SCMI_AGENT_IDX_COUNT] = {
    [MCP_MORELLO_SCMI_AGENT_IDX_MANAGEMENT] = {
        .type = SCMI_AGENT_TYPE_MANAGEMENT,
        .name = "MANAGEMENT",
    },
};

const struct fwk_module_config config_scmi = {
    .data =
        &(struct mod_scmi_config){
            .protocol_requester_count_max = 1,
            .agent_count = FWK_ARRAY_SIZE(agent_table) - 1,
            .agent_table = agent_table,
            .vendor_identifier = "arm",
            .sub_vendor_identifier = "arm",
        },
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_service_table),
};
