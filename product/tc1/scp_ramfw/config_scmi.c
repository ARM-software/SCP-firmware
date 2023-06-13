/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tc1_scmi.h"

#include <mod_scmi.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element service_table[
    SCP_TC1_SCMI_SERVICE_IDX_COUNT + 1] = {
    [SCP_TC1_SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                SCP_TC1_SCMI_SERVICE_IDX_PSCI),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id = FWK_ID_NONE_INIT,
            .scmi_agent_id = SCP_SCMI_AGENT_ID_PSCI,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCP_TC1_SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "OSPM0",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                SCP_TC1_SCMI_SERVICE_IDX_OSPM_0),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id = FWK_ID_NONE_INIT,
            .scmi_agent_id = SCP_SCMI_AGENT_ID_OSPM,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCP_TC1_SCMI_SERVICE_IDX_OSPM_1] = {
        .name = "OSPM1",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                SCP_TC1_SCMI_SERVICE_IDX_OSPM_1),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id = FWK_ID_NONE_INIT,
            .scmi_agent_id = SCP_SCMI_AGENT_ID_OSPM,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCP_TC1_SCMI_SERVICE_IDX_COUNT] = { 0 }
};

static const struct fwk_element *get_service_table(fwk_id_t module_id)
{
    return service_table;
}

#ifndef BUILD_HAS_MOD_RESOURCE_PERMS

/* PSCI agent has no access to clock, perf and sensor protocol */
static const uint32_t dis_protocol_list_psci[3] = {
    MOD_SCMI_PROTOCOL_ID_SENSOR,
    MOD_SCMI_PROTOCOL_ID_CLOCK,
    MOD_SCMI_PROTOCOL_ID_PERF,
};
#endif

static struct mod_scmi_agent agent_table[SCP_SCMI_AGENT_ID_COUNT] = {
    [SCP_SCMI_AGENT_ID_OSPM] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "OSPM",
    },
    [SCP_SCMI_AGENT_ID_PSCI] = {
        .type = SCMI_AGENT_TYPE_PSCI,
        .name = "PSCI",
    },
};

const struct fwk_module_config config_scmi = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_service_table),
    .data = &((struct mod_scmi_config){
        .protocol_count_max = 9,
#ifndef BUILD_HAS_MOD_RESOURCE_PERMS
        .dis_protocol_count_psci = FWK_ARRAY_SIZE(dis_protocol_list_psci),
        .dis_protocol_list_psci = dis_protocol_list_psci,
#endif
        .agent_count = FWK_ARRAY_SIZE(agent_table) - 1,
        .agent_table = agent_table,
        .vendor_identifier = "arm",
        .sub_vendor_identifier = "arm",
    }),
};
