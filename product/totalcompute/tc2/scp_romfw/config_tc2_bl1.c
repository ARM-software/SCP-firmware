/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_mmap.h"

#include <mod_tc2_bl1.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

const struct fwk_module_config config_tc2_bl1 = {
    .data = &((struct mod_tc2_bl1_config){
        .ap_context_base = SCP_AP_CONTEXT_BASE,
        .ap_context_size = SCP_AP_CONTEXT_SIZE,
        .id_primary_cluster = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 1),
        .id_primary_core = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 0),
    })
};
