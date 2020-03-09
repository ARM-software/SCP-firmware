/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_mmap.h"

#include <mod_ccn512.h>

#include <fwk_module.h>

#include <stddef.h>

/*
 * CCN512 module
 */
struct fwk_module_config config_ccn512 = {
    .get_element_table = NULL,
    .data = &((struct mod_ccn512_module_config){
        .reg_base = (ccn512_reg_t *)CCN512_BASE,
    }),
};
