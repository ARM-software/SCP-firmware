/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mcp_software_mmap.h"

#include <mod_bootloader.h>

#include <fwk_module.h>

static const struct mod_bootloader_config bootloader_module_config = {
    .source_base = MCP_RAMFW_IMAGE_FLASH_BASE,
    .source_size = MCP_RAMFW_IMAGE_FLASH_SIZE,
    .destination_base = MCP_ITC_RAM_BASE,
    .destination_size = MCP_RAMFW_IMAGE_FLASH_SIZE,
};

struct fwk_module_config config_bootloader = {
    .data = &bootloader_module_config,
};
