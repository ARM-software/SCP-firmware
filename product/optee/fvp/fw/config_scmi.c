/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/scmi.h>
#include <scmi_agents.h>

#include <mod_msg_smt.h>
#include <mod_scmi.h>

#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element service_table[] = {
    [SCMI_SERVICE_IDX_PSCI] = {
        .name = "agent PSCI",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT,
                                                SCMI_SERVICE_IDX_PSCI),
            .transport_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                                MOD_MSG_SMT_API_IDX_SCMI_TRANSPORT),
            .scmi_agent_id = SCMI_AGENT_ID_PSCI,
            .transport_notification_init_id = FWK_ID_NONE_INIT,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "agent OSPM0",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT,
                                                SCMI_SERVICE_IDX_OSPM_0),
            .transport_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                                MOD_MSG_SMT_API_IDX_SCMI_TRANSPORT),
            .scmi_agent_id = SCMI_AGENT_ID_OSPM,
            .transport_notification_init_id = FWK_ID_NONE_INIT,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCMI_SERVICE_IDX_OSPM_1] = {
        .name = "agent OSPM1",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT,
                                                SCMI_SERVICE_IDX_OSPM_1),
            .transport_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                                MOD_MSG_SMT_API_IDX_SCMI_TRANSPORT),
            .scmi_agent_id = SCMI_AGENT_ID_PERF,
            .transport_notification_init_id = FWK_ID_NONE_INIT,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCMI_SERVICE_IDX_COUNT] = { 0 }
};

static const struct fwk_element *get_service_table(fwk_id_t module_id)
{
    return service_table;
}

static const struct mod_scmi_agent agent_table[] = {
    [SCMI_AGENT_ID_OSPM] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "OSPM",
    },
    [SCMI_AGENT_ID_PSCI] = {
        .type = SCMI_AGENT_TYPE_PSCI,
        .name = "PSCI",
    },
   [SCMI_AGENT_ID_PERF] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "PERF",
    },
};

struct fwk_module_config config_scmi = {
    .data = &((struct mod_scmi_config){
        .protocol_count_max = 9,
        .agent_count = FWK_ARRAY_SIZE(agent_table) - 1,
        .agent_table = agent_table,
        .vendor_identifier = "Linaro",
        .sub_vendor_identifier = "PMWG",
    }),
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_service_table),
};
