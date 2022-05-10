/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mod_platform_system.h"

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

struct mod_platform_system_config system_config = {
    .primary_cpu_mpid = 0,
};

struct fwk_module_config config_platform_system = {
    .data = &system_config,
};
