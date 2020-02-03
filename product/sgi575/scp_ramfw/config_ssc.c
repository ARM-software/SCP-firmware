/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_module.h>
#include <mod_ssc.h>
#include <scp_sgi575_mmap.h>

const struct fwk_module_config config_ssc = {
    .get_element_table = NULL,
    .data = &(struct mod_ssc_config) {
        .ssc_base = SSC_BASE,
        .product_name = "System Guidance for Infrastructure - 575"
    },
};
