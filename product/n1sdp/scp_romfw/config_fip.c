/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_fip.h>

#include <fwk_module.h>

static struct mod_fip_module_config fip_data = {
    .custom_fip_uuid_desc_arr = NULL,
    .custom_uuid_desc_count = 0,
};

struct fwk_module_config config_fip = { .data = &fip_data };
