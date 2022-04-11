/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_mcp_system_mmap.h"

#include <morello_fip.h>

#include <mod_fip.h>
#include <mod_morello_rom.h>

#include <fwk_module.h>

const struct fwk_module_config config_morello_rom = {
    .data = &((struct morello_rom_config){
        .fip_base_address = MCP_QSPI_FLASH_BASE_ADDR,
        .fip_nvm_size = MCP_QSPI_FLASH_SIZE,
        .ramfw_base = MCP_RAM0_BASE,
        .image_type =
            (enum mod_fip_toc_entry_type)MOD_MORELLO_FIP_TOC_ENTRY_MCP_BL2,
    })
};
