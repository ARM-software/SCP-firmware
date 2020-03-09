/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_sds.h"
#include "scp_mmap.h"
#include "system_mmap.h"

#include <mod_bootloader.h>

#include <fwk_module.h>

static const struct mod_bootloader_config bootloader_module_config = {
    .source_base = TRUSTED_RAM_BASE,
    .source_size = 256 * 1024,
    .destination_base = SCP_RAM_BASE,
    .destination_size = SCP_RAM_SIZE,
    .sds_struct_id = JUNO_SDS_BOOTLOADER,
};

struct fwk_module_config config_bootloader = {
    .data = &bootloader_module_config,
};
