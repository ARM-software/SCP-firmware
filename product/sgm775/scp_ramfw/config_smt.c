/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_clock.h>
#include <mod_smt.h>
#include <sgm775_core.h>
#include <sgm775_mhu.h>
#include <sgm775_scmi.h>
#include <config_power_domain.h>
#include <clock_devices.h>
#include <software_mmap.h>

static const struct fwk_element smt_element_table[] = {
    [SGM775_SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &((struct mod_smt_channel_config) {
            .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_SMT_POLICY_INIT_MAILBOX | MOD_SMT_POLICY_SECURE,
            .mailbox_address = (uintptr_t)SCMI_PAYLOAD_S_A2P_BASE,
            .mailbox_size = SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_MHU,
                SGM775_MHU_DEVICE_IDX_S, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
        })
    },
    [SGM775_SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "OSPM0",
        .data = &((struct mod_smt_channel_config) {
            .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_SMT_POLICY_INIT_MAILBOX,
            .mailbox_address = (uintptr_t)SCMI_PAYLOAD0_NS_A2P_BASE,
            .mailbox_size = SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_MHU,
                 SGM775_MHU_DEVICE_IDX_NS_L, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
        })
    },
    [SGM775_SCMI_SERVICE_IDX_OSPM_1] = {
        .name = "OSPM1",
        .data = &((struct mod_smt_channel_config) {
            .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_SMT_POLICY_INIT_MAILBOX,
            .mailbox_address = (uintptr_t)SCMI_PAYLOAD1_NS_A2P_BASE,
            .mailbox_size = SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_MHU,
                 SGM775_MHU_DEVICE_IDX_NS_H, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
        })
    },
    [SGM775_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
    unsigned int idx;
    struct mod_smt_channel_config *config;

    for (idx = 0; idx < SGM775_SCMI_SERVICE_IDX_COUNT; idx++) {
        config = (struct mod_smt_channel_config *)(smt_element_table[idx].data);
        config->pd_source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN,
            CONFIG_POWER_DOMAIN_SYSTOP_CHILD_COUNT + sgm775_core_get_count());
    }

    return smt_element_table;
}

struct fwk_module_config config_smt = {
    .get_element_table = smt_get_element_table,
};
