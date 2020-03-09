/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_system_mmap_scp.h"

#include <mod_synquacer_rom.h>

#include <fwk_module.h>

const struct fwk_module_config config_synquacer_rom = {
    .data = &((struct synquacer_rom_config){
        .ramfw_base = SCP_RAM0_BASE,
        .nor_base = SCP_RAMFW_ROM_BASE,
        .load_ram_size = SCP_RAMFW_IMAGE_SIZE,
    })
};
