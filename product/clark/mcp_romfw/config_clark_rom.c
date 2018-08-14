/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <mod_clark_rom.h>
#include <mcp_clark_mmap_mcp.h>

const struct fwk_module_config config_clark_rom = {
    .data = &((struct clark_rom_config) {
        .ramfw_base = MCP_RAM0_BASE,
        .load_ram_size = 0,
    })
};
