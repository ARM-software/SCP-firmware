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

enum scmi_service_channel_idx {
    FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0,
    FAKE_SCMI_CHANNEL_DEVICE_IDX_COUNT
};

#define FAKE_MAILBOX_SIZE 128

/* Provide a fake device info */
static struct fwk_element smt_element_table[] = {
    [FAKE_SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "OSPM0",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .mailbox_size = FAKE_MAILBOX_SIZE,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MBX,
                                                 FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MBX, 0),
        })
    },
    [FAKE_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
    return (const struct fwk_element *)smt_element_table;
}

struct fwk_module_config config_fake_msg_smt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(smt_get_element_table),
};
