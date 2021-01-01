/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <juno_scmi.h>
#include <mod_scmi_reset_domain.h>
#include <mod_juno_reset_domain.h>
#include <mod_reset_domain.h>

static const struct mod_scmi_reset_domain_device agent_device_table_ospm[] = {
   [JUNO_RESET_DOMAIN_IDX_UART] = {
        .element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                                JUNO_RESET_DOMAIN_IDX_UART),
        .permissions = MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES |
                       MOD_SCMI_RESET_DOMAIN_PERM_RESET,
    },
};

static const struct mod_scmi_reset_domain_agent agent_table[] = {
    [JUNO_SCMI_AGENT_IDX_PSCI] = { 0 /* No access */ },
    [JUNO_SCMI_AGENT_IDX_OSPM] = {
        .device_table = agent_device_table_ospm,
        .agent_domain_count = FWK_ARRAY_SIZE(agent_device_table_ospm),
    },
};

struct fwk_module_config config_scmi_reset_domain = {
    .data = &((struct mod_scmi_reset_domain_config) {
        .agent_table = agent_table,
        .agent_count = FWK_ARRAY_SIZE(agent_table),
    }),
};
