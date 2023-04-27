/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <scp_platform_mhu.h>

#include <mod_mhu2.h>
#include <mod_tc2_bl1.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

static const struct fwk_element transport_element_table[] = {
    [0] = {
        .name = "SCP2RSS_EVENT",
        .data = &((
            struct mod_transport_channel_config) {
            .transport_type = MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE,
            .signal_api_id = FWK_ID_API_INIT(
                                FWK_MODULE_IDX_TC2_BL1,
                                MOD_TC2_BL1_API_TRANSPORT_FIRMWARE_SIGNAL_INPUT),
            .driver_id =
                FWK_ID_SUB_ELEMENT_INIT(
                    FWK_MODULE_IDX_MHU2,
                    0,
                    0),
            .driver_api_id =
                FWK_ID_API_INIT(
                    FWK_MODULE_IDX_MHU2,
                    MOD_MHU2_API_IDX_TRANSPORT_DRIVER),
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
