/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lcp_css_mmap.h>
#include <lcp_mhu3.h>
#include <lcp_platform_transport.h>

#include <mod_mhu3.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element transport_element_table[] = {
    [LCP_PLATFORM_TRANSPORT_IDX_DVFS_SET_LVL] = {
        .name = "AP2LCP_TRANSPORT DVFS SET LEVEL",
        .data = &((
            struct mod_transport_channel_config) {
            .transport_type = MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
            .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .driver_id =
                FWK_ID_SUB_ELEMENT_INIT(
                    FWK_MODULE_IDX_MHU3,
                    MHU3_DEVICE_IDX_LCP_AP_FCH_DVFS_SET_LVL,
                    0),
            .driver_api_id =
                FWK_ID_API_INIT(
                    FWK_MODULE_IDX_MHU3,
                    MOD_MHU3_API_IDX_TRANSPORT_DRIVER),
        }),
    },
    [LCP_PLATFORM_TRANSPORT_IDX_COUNT] = { 0 },
};

static const struct fwk_element *transport_get_element_table(fwk_id_t module_id)
{
    return transport_element_table;
}

const struct fwk_module_config config_transport = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(transport_get_element_table),
};
