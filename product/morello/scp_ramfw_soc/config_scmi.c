/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <morello_scp_scmi.h>

#include <internal/scmi.h>

#include <mod_scmi.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element
        service_table[SCP_MORELLO_SCMI_SERVICE_IDX_COUNT + 1] = {
    [SCP_MORELLO_SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                SCP_MORELLO_SCMI_SERVICE_IDX_PSCI),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = SCP_SCMI_AGENT_ID_PSCI,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCP_MORELLO_SCMI_SERVICE_IDX_OSPM] = {
        .name = "OSPM",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                SCP_MORELLO_SCMI_SERVICE_IDX_OSPM),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = SCP_SCMI_AGENT_ID_OSPM,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCP_MORELLO_SCMI_SERVICE_IDX_MCP] = {
        .name = "MCP",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                SCP_MORELLO_SCMI_SERVICE_IDX_MCP),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id = FWK_ID_NONE_INIT,
            .scmi_agent_id = SCP_SCMI_AGENT_ID_MCP,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCP_MORELLO_SCMI_SERVICE_IDX_COUNT] = { 0 }
};

static const struct fwk_element *get_service_table(fwk_id_t module_id)
{
    return service_table;
}

static struct mod_scmi_agent agent_table[] = {
    [SCP_SCMI_AGENT_ID_OSPM] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "OSPM",
    },
    [SCP_SCMI_AGENT_ID_PSCI] = {
        .type = SCMI_AGENT_TYPE_PSCI,
        .name = "PSCI",
    },
    [SCP_SCMI_AGENT_ID_MCP] = {
        .type = SCMI_AGENT_TYPE_MANAGEMENT,
        .name = "MANAGEMENT",
    },
};

const struct fwk_module_config config_scmi = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_service_table),
    .data = &((struct mod_scmi_config){
        .protocol_count_max = 8,
        .agent_count = FWK_ARRAY_SIZE(agent_table) - 1,
        .agent_table = agent_table,
        .vendor_identifier = "arm",
        .sub_vendor_identifier = "arm",
    }),
};
