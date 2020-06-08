/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "morello_core.h"
#include "morello_scp_mhu.h"
#include "morello_scp_mmap.h"
#include "morello_scp_scmi.h"
#include "morello_scp_software_mmap.h"

#include <mod_smt.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element smt_element_table[] = {
    [SCP_MORELLO_SCMI_SERVICE_IDX_PSCI] =
        {
            .name = "PSCI",
            .data =
                &(struct mod_smt_channel_config){
                    .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
                    .policies =
                        MOD_SMT_POLICY_INIT_MAILBOX | MOD_SMT_POLICY_SECURE,
                    .mailbox_address = SCP_AP_SHARED_SECURE_RAM,
                    .mailbox_size = SCP_SCMI_PAYLOAD_SIZE,
                    .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                        FWK_MODULE_IDX_MHU,
                        MORELLO_MHU_DEVICE_IDX_S_CLUS0,
                        0),
                    .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
                },
        },
    [SCP_MORELLO_SCMI_SERVICE_IDX_OSPM] =
        {
            .name = "OSPM",
            .data =
                &(struct mod_smt_channel_config){
                    .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
                    .policies = MOD_SMT_POLICY_INIT_MAILBOX,
                    .mailbox_address = SCP_AP_SHARED_NONSECURE_RAM,
                    .mailbox_size = SCP_SCMI_PAYLOAD_SIZE,
                    .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                        FWK_MODULE_IDX_MHU,
                        MORELLO_MHU_DEVICE_IDX_NS_CLUS0,
                        0),
                    .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
                },
        },
    [SCP_MORELLO_SCMI_SERVICE_IDX_MCP] =
        {
            .name = "MCP",
            .data =
                &(struct mod_smt_channel_config){
                    .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
                    .policies =
                        MOD_SMT_POLICY_INIT_MAILBOX | MOD_SMT_POLICY_SECURE,
                    .mailbox_address = SCP_MCP_SHARED_SECURE_RAM,
                    .mailbox_size = SCP_SCMI_PAYLOAD_SIZE,
                    .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                        FWK_MODULE_IDX_MHU,
                        MORELLO_MHU_DEVICE_IDX_S_MCP,
                        0),
                    .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
                },
        },
    [SCP_MORELLO_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
    unsigned int idx;
    struct mod_smt_channel_config *config;

    for (idx = 0; idx < SCP_MORELLO_SCMI_SERVICE_IDX_COUNT; idx++) {
        config = (struct mod_smt_channel_config *)(smt_element_table[idx].data);
        config->pd_source_id = FWK_ID_ELEMENT(
            FWK_MODULE_IDX_POWER_DOMAIN, PD_SINGLE_CHIP_IDX_SYSTOP0);
    }

    return smt_element_table;
}

const struct fwk_module_config config_smt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(smt_get_element_table),
};
