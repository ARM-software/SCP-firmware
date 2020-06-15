/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_mcp_system_mmap.h"

#include <mod_fip.h>
#include <mod_morello_rom.h>

#include <fwk_module.h>

const struct fwk_module_config config_morello_rom = {
    .data = &((struct morello_rom_config){
        .ramfw_base = MCP_RAM0_BASE,
        .image_type = MOD_FIP_TOC_ENTRY_MCP_BL2,
    })
};
