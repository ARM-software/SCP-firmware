/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_smt.h>
#include <rdn1e1_core.h>
#include <scp_rdn1e1_mhu.h>
#include <scp_rdn1e1_scmi.h>
#include <scp_software_mmap.h>
#include <config_power_domain.h>

static const struct fwk_element smt_element_table[] = {
    [SCP_RDN1E1_SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &((struct mod_smt_channel_config) {
            .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_SMT_POLICY_INIT_MAILBOX | MOD_SMT_POLICY_SECURE,
            .mailbox_address = (uintptr_t)SCP_SCMI_PAYLOAD_S_A2P_BASE,
            .mailbox_size = SCP_SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_MHU2,
                SCP_RDN1E1_MHU_DEVICE_IDX_SCP_AP_S_CLUS0, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU2, 0),
        })
    },
    [SCP_RDN1E1_SCMI_SERVICE_IDX_OSPM] = {
        .name = "OSPM",
        .data = &((struct mod_smt_channel_config) {
            .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_SMT_POLICY_INIT_MAILBOX,
            .mailbox_address = (uintptr_t)SCP_SCMI_PAYLOAD_NS_A2P_BASE,
            .mailbox_size = SCP_SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_MHU2,
                 SCP_RDN1E1_MHU_DEVICE_IDX_SCP_AP_NS_CLUS0, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU2, 0),
        })
    },
    [SCP_RDN1E1_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
    unsigned int idx;
    struct mod_smt_channel_config *config;

    for (idx = 0; idx < SCP_RDN1E1_SCMI_SERVICE_IDX_COUNT; idx++) {
        config = (struct mod_smt_channel_config *)(smt_element_table[idx].data);
        config->pd_source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN,
            rdn1e1_core_get_core_count() + PD_STATIC_DEV_IDX_SYSTOP);
    }

    return smt_element_table;
}

const struct fwk_module_config config_smt = {
    .get_element_table = smt_get_element_table,
};
