/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_synquacer_memc.h>

/* Configuration of the SynQuacerMEMC module. */
const struct fwk_module_config config_synquacer_memc = {
    .get_element_table = NULL,
};
