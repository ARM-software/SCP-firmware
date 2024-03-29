/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_scp_mmap.h"

#include <mod_ssc.h>

#include <fwk_module.h>

#include <stddef.h>

const struct fwk_module_config config_ssc = {
    .data =
        &(struct mod_ssc_config){
            .ssc_base = SCP_SSC_BASE,
            .ssc_debug_cfg_set = 0xFF,
            .product_name = "Morello SOC Platform",
        },
};
