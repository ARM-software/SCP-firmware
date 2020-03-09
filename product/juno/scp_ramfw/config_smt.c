/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "juno_mhu.h"
#include "juno_scmi.h"
#include "software_mmap.h"

#include <mod_smt.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

static const struct fwk_element element_table[] = {
    [JUNO_SCMI_SERVICE_IDX_PSCI] = {
        .name = "",
        .data = &(struct mod_smt_channel_config) {
            .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
            .policies = (MOD_SMT_POLICY_INIT_MAILBOX | MOD_SMT_POLICY_SECURE),
            .mailbox_address = (uintptr_t)SCMI_PAYLOAD_S_A2P_BASE,
            .mailbox_size = SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                FWK_MODULE_IDX_MHU,
                JUNO_MHU_DEVICE_IDX_S,
                0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
            .pd_source_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_SYSTOP),
        }
    },
    [JUNO_SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "",
        .data = &(struct mod_smt_channel_config) {
            .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_SMT_POLICY_INIT_MAILBOX,
            .mailbox_address = (uintptr_t)SCMI_PAYLOAD_LOW_A2P_BASE,
            .mailbox_size = SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                FWK_MODULE_IDX_MHU,
                JUNO_MHU_DEVICE_IDX_NS_L,
                0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
            .pd_source_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_SYSTOP),
        }
    },
    [JUNO_SCMI_SERVICE_IDX_OSPM_1] = {
        .name = "",
        .data = &(struct mod_smt_channel_config) {
            .type = MOD_SMT_CHANNEL_TYPE_SLAVE,
            .policies = MOD_SMT_POLICY_INIT_MAILBOX,
            .mailbox_address = (uintptr_t)SCMI_PAYLOAD_HIGH_A2P_BASE,
            .mailbox_size = SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                FWK_MODULE_IDX_MHU,
                JUNO_MHU_DEVICE_IDX_NS_H,
                0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
            .pd_source_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_POWER_DOMAIN,
                POWER_DOMAIN_IDX_SYSTOP),
        }
    },
    [JUNO_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_smt = {
    .get_element_table = get_element_table,
    .data = NULL,
};
