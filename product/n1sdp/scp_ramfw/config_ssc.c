/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_scp_mmap.h"

#include <mod_ssc.h>

#include <fwk_module.h>

#include <stddef.h>

const struct fwk_module_config config_ssc = {
    .get_element_table = NULL,
    .data = &(struct mod_ssc_config) {
        .ssc_base = SCP_SSC_BASE,
        .ssc_debug_cfg_set = 0xFF,
        .product_name = "N1 System Development Platform"
    },
};
