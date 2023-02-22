/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_mcp_mhu.h"
#include "morello_mcp_software_mmap.h"

#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element transport_element_table[2] = {
    [0] = { .name = "MANAGEMENT-NS",
            .data = &((struct mod_transport_channel_config){
                .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_REQUESTER,
                .policies = MOD_TRANSPORT_POLICY_INIT_MAILBOX,
                .out_band_mailbox_address = (uintptr_t)MCP_SCP_NS_MAILBOX_SRAM,
                .out_band_mailbox_size = MCP_SCMI_PAYLOAD_SIZE,
                .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                    FWK_MODULE_IDX_MHU,
                    MORELLO_MHU_DEVICE_IDX_NS_SCP,
                    0),
                .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
            }),
          },
    [1] = { 0 },
};

static const struct fwk_element *transport_get_element_table(fwk_id_t module_id)
{
    return transport_element_table;
}

const struct fwk_module_config config_transport = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(transport_get_element_table),
};
