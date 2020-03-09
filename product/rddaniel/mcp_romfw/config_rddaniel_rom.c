/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mcp_software_mmap.h"

#include <mod_mscp_rom.h>

#include <fwk_module.h>

const struct fwk_module_config config_mscp_rom = {
    .data = &((struct mscp_rom_config) {
        .ramfw_ram_base = MCP_RAM0_BASE,
        .ramfw_flash_size = 0,
    })
};
