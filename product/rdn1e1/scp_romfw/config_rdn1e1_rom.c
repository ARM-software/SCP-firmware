/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_rdn1e1_mmap_scp.h"
#include "scp_software_mmap.h"

#include <mod_rdn1e1_rom.h>

#include <fwk_module.h>

const struct fwk_module_config config_rdn1e1_rom = {
    .data = &((struct rdn1e1_rom_config) {
        .ramfw_base = SCP_RAM0_BASE,
        .nor_base = SCP_NOR_BASE,
        .load_ram_size = SCP_IMAGE_SIZE,
    })
};
