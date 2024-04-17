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
#include <mod_optee_smt.h>
#include <mod_scmi.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element service_table[] = {
    [SCMI_SERVICE_IDX_NS_CHANNEL0] = {
        .name = "service-0",
        .data = &((struct mod_scmi_service_config) {
#if defined(CFG_SCPFW_MOD_OPTEE_SMT)
            .transport_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_SMT, 0),
            .transport_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_SMT,
                                                MOD_OPTEE_SMT_API_IDX_SCMI_TRANSPORT),
#elif defined(CFG_SCPFW_MOD_MSG_SMT)
            .transport_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT, 0),
            .transport_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                                MOD_MSG_SMT_API_IDX_SCMI_TRANSPORT),
#else
#    error None of CFG_SCPFW_MOD_OPTEE_SMT and CFG_SCPFW_MOD_MSG_SMT is defined.
#endif
            .scmi_agent_id = SCMI_AGENT_ID_NSEC0,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCMI_SERVICE_IDX_COUNT] = { 0 }
};

static const struct fwk_element *get_scmi_service_table(fwk_id_t module_id)
{
    return service_table;
}

static const struct mod_scmi_agent agent_table[] = {
    [SCMI_AGENT_ID_NSEC0] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "OSPM0",
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
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_scmi_service_table),
};
