/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_mcp_mhu.h"
#include "n1sdp_mcp_software_mmap.h"

#include <mod_smt.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element smt_element_table[] = {
    [0] = {
        .name = "MANAGEMENT-S",
        .data = &((struct mod_smt_channel_config) {
            .type = MOD_SMT_CHANNEL_TYPE_MASTER,
            .policies = MOD_SMT_POLICY_INIT_MAILBOX | MOD_SMT_POLICY_SECURE,
            .mailbox_address = (uintptr_t)SCMI_PAYLOAD_SCP_TO_MCP_S,
            .mailbox_size = MCP_SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_MHU,
                N1SDP_MHU_DEVICE_IDX_S_SCP, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_MHU, 0),
        })
    },
    [1] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
    return smt_element_table;
}

const struct fwk_module_config config_n1sdp_smt = {
    .get_element_table = smt_get_element_table,
};
