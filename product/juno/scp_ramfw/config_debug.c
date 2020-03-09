/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_debug.h>
#include <mod_juno_debug.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/* Configuration of the Debug elements */
static const struct fwk_element debug_element_table[] = {
    [0] = {
        .name = "JUNO DEBUG",
        .data = &((const struct mod_debug_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_DEBUG, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_DEBUG,
                                             MOD_JUNO_DEBUG_API_IDX_DRIVER),
        }),
    },
    [1] = { 0 }, /* Termination description */
};

static const struct fwk_element *get_debug_element_table(fwk_id_t module_id)
{
    return debug_element_table;
}

/* Configuration of the Debug module */
struct fwk_module_config config_debug = {
    .get_element_table = get_debug_element_table,
    .data = NULL,
};
