/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rddaniel_scmi.h"

#include <mod_scmi.h>
#include <mod_smt.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element service_table[] = {
    [SCP_RDDANIEL_SCMI_SERVICE_IDX_PSCI] = {
        .name = "SERVICE0",
        .data = &((struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_SMT,
                SCP_RDDANIEL_SCMI_SERVICE_IDX_PSCI),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_SMT,
                MOD_SMT_API_IDX_SCMI_TRANSPORT),
            .transport_notification_init_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_SMT,
                MOD_SMT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = SCP_SCMI_AGENT_ID_PSCI,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        }),
    },
    [SCP_RDDANIEL_SCMI_SERVICE_IDX_COUNT] = { 0 }
};

static const struct fwk_element *get_service_table(fwk_id_t module_id)
{
    return service_table;
}

static struct mod_scmi_agent agent_table[] = {
    [SCP_SCMI_AGENT_ID_PSCI] = {
        .type = SCMI_AGENT_TYPE_PSCI,
        .name = "PSCI",
    },
};

const struct fwk_module_config config_scmi = {
    .data =
        &(struct mod_scmi_config){
            .protocol_count_max = 9,
            .agent_count = FWK_ARRAY_SIZE(agent_table),
            .agent_table = agent_table,
            .vendor_identifier = "arm",
            .sub_vendor_identifier = "arm",
        },

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_service_table),
};
