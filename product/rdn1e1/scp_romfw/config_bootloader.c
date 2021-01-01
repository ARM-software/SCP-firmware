/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_rdn1e1_mmap_scp.h"
#include "scp_software_mmap.h"

#include <mod_bootloader.h>

#include <fwk_module.h>

static const struct mod_bootloader_config bootloader_module_config = {
    .source_base = SCP_NOR_BASE,
    .source_size = SCP_IMAGE_SIZE,
    .destination_base = SCP_RAM0_BASE,
    .destination_size = SCP_IMAGE_SIZE,
};

struct fwk_module_config config_bootloader = {
    .data = &bootloader_module_config,
};
