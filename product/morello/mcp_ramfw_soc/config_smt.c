/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <morello_mcp_mhu.h>
#include <morello_mcp_software_mmap.h>

#include <mod_smt.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element smt_element_table[2] = {
    [0] = {
        .name = "MANAGEMENT-NS",
        .data = &((struct mod_smt_channel_config){
            .type = MOD_SMT_CHANNEL_TYPE_REQUESTER,
            .mailbox_address = (uintptr_t)MCP_SCP_NS_MAILBOX_SRAM,
            .mailbox_size = MCP_SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                FWK_MODULE_IDX_MORELLO_MHU,
                MORELLO_MHU_DEVICE_IDX_NS_SCP,
                0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MORELLO_MHU, 0),
        }),
    },
    [1] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
    return smt_element_table;
}

const struct fwk_module_config config_morello_smt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(smt_get_element_table),
};
