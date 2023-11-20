/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lcp_mhu3.h>
#include <lcp_mmap.h>

#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element transport_element_table[] = {
    [0] = {
        .name = "LCP2SCP_TRANSPORT",
        .data = &((
            struct mod_transport_channel_config) {
#ifdef BUILD_HAS_FAST_CHANNELS
            .transport_type = MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
#endif
            .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_REQUESTER,
            .driver_id =
                FWK_ID_SUB_ELEMENT_INIT(
                    FWK_MODULE_IDX_MHU3,
                    MHU3_DEVICE_IDX_SCP_LCP,
                    0),
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
