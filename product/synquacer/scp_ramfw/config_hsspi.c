/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_hsspi.h>
#include <synquacer_mmap.h>

/* Configuration of the HSSPI module. */
const struct fwk_module_config config_hsspi = {
    .get_element_table = NULL,
};
