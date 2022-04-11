/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_mcp_system_mmap.h"

#include <n1sdp_fip.h>

#include <mod_fip.h>
#include <mod_n1sdp_rom.h>

#include <fwk_module.h>

const struct fwk_module_config config_n1sdp_rom = {
    .data = &((struct n1sdp_rom_config){
        .fip_base_address = MCP_QSPI_FLASH_BASE_ADDR,
        .fip_nvm_size = MCP_QSPI_FLASH_SIZE,
        .ramfw_base = MCP_RAM0_BASE,
        .image_type =
            (enum mod_fip_toc_entry_type)MOD_N1SDP_FIP_TOC_ENTRY_MCP_BL2,
    })
};
