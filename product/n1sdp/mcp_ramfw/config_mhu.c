/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_mcp_mhu.h"
#include "n1sdp_mcp_mmap.h"

#include <mod_mhu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <fmw_cmsis.h>

static const struct fwk_element mhu_element_table[] = {
    [N1SDP_MHU_DEVICE_IDX_NS_SCP] = {
        .name = "SCP_MCP_MHU_NS",
        .sub_element_count = 1,
        .data = &((struct mod_mhu_device_config) {
            .irq = MHU_SCP_NONSEC_IRQ,
            .in = MHU_SCP_TO_MCP_NS,
            .out = MHU_MCP_TO_SCP_NS,
        }),
    },
    [N1SDP_MHU_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    return mhu_element_table;
}

struct fwk_module_config config_n1sdp_mhu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mhu_get_element_table),
};
