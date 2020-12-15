/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_css_mmap.h"

#include <mod_apremap.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

const struct fwk_module_config config_apremap = {
    .data =
        &(struct mod_apremap_config){
            .base = SCP_PIK_SCP_BASE + 0x34,
        }
};
