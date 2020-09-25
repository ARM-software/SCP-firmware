/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_software_mmap.h"
#include "tc0_sds.h"

#include <mod_bootloader.h>

#include <fwk_module.h>

static const struct mod_bootloader_config bootloader_module_config = {
    .source_base = SCP_TRUSTED_RAM_BASE,
    .source_size = 512 * 1024,
    .destination_base = SCP_RAM_BASE,
    .destination_size = SCP_RAM_SIZE,
    .sds_struct_id = TC0_SDS_BOOTLOADER,
};

struct fwk_module_config config_bootloader = {
    .data = &bootloader_module_config,
};
