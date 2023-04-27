/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_tc2_power_model.h>

#include <fwk_module.h>

static const struct fwk_element pm_elem_table[] = {
    [0] = {
        .name = "Power Model 0",
        .data = &(struct mod_tc2_power_model_dev_config){
            .coeff = 1,
        },
    },
    [1] = {
        .name = "Power Model 1",
        .data = &(struct mod_tc2_power_model_dev_config){
            .coeff = 1,
        },
    },
    [2] = { 0 } /* Termination description */
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return pm_elem_table;
};

const struct fwk_module_config config_tc2_power_model = {
    .data = NULL,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};
