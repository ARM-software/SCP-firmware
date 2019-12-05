/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_mmap.h"
#include "scp_software_mmap.h"

#include <mod_mscp_rom.h>

#include <fwk_module.h>

const struct fwk_module_config config_mscp_rom = {
    .data = &((struct mscp_rom_config) {
        .ramfw_ram_base = SCP_ITC_RAM_BASE,
        .ramfw_flash_base = SCP_RAMFW_IMAGE_FLASH_BASE,
        .ramfw_flash_size = SCP_RAMFW_IMAGE_FLASH_SIZE,
    })
};
