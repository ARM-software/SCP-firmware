/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <Mockfwk_module.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <mod_msg_smt.h>
#include <mod_optee_mbx.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/*
 * This represents index of the channel descriptor within
 * element table
 */
enum msg_channel_idx {
    FAKE_SCMI_SERVICE_IDX_OSPM_0,
    FAKE_SCMI_SERVICE_IDX_COUNT,
};

enum mbx_channel_idx {
    FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0,
    FAKE_SCMI_CHANNEL_DEVICE_IDX_COUNT
};

/* Provide a fake device info */
static const struct fwk_element mbx_element_table[] = {
    [FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0] = {
        .name = "OP-TEE for OSPM #0",
        .data = &((struct mod_optee_mbx_channel_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT,
                                         FAKE_SCMI_SERVICE_IDX_OSPM_0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                             MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        })
    },
    [FAKE_SCMI_CHANNEL_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mbx_get_element_table(fwk_id_t module_id)
{
    return (const struct fwk_element *)mbx_element_table;
}

struct fwk_module_config config_fake_optee_mbx = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mbx_get_element_table),
};
