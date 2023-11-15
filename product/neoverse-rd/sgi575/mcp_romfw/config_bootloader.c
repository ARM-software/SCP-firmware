/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mcp_sgi575_mmap_mcp.h"
#include "mcp_software_mmap.h"

#include <mod_bootloader.h>

#include <fwk_module.h>

static const struct mod_bootloader_config bootloader_module_config = {
    .source_base = MCP_NOR_BASE,
    .source_size = MCP_IMAGE_SIZE,
    .destination_base = MCP_RAM0_BASE,
    .destination_size = MCP_IMAGE_SIZE,
};

struct fwk_module_config config_bootloader = {
    .data = &bootloader_module_config,
};
