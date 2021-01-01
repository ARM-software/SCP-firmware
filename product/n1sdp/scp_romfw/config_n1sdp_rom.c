/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_scp_system_mmap.h"

#include <mod_fip.h>
#include <mod_n1sdp_rom.h>

#include <fwk_module.h>

const struct fwk_module_config config_n1sdp_rom = {
    .data = &((struct n1sdp_rom_config){
        .fip_base_address = SCP_QSPI_FLASH_BASE_ADDR,
        .fip_nvm_size = SCP_QSPI_FLASH_SIZE,
        .ramfw_base = SCP_RAM0_BASE,
        .image_type = MOD_FIP_TOC_ENTRY_SCP_BL2,
    })
};
