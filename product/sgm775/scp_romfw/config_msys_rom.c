/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_msys_rom.h>
#include <software_mmap.h>
#include <sgm775_mmap_scp.h>
#include <sgm775_core.h>

const struct fwk_module_config config_msys_rom = {
    .data = &((struct msys_rom_config) {
        .ap_context_base = AP_CONTEXT_BASE,
        .ap_context_size = AP_CONTEXT_SIZE,
        .id_primary_cluster = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 0),
        .id_primary_core = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 1),
    })
};
