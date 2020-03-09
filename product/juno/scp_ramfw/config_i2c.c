/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_irq.h"
#include "juno_mmap.h"

#include <mod_dw_apb_i2c.h>
#include <mod_i2c.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element dw_apb_i2c_element_table[] = {
    [0] = {
        .name = "",
        .data = &(struct mod_dw_apb_i2c_dev_config) {
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .i2c_irq = I2C_IRQ,
            .reg = (uintptr_t)I2C_BASE,
        }
    },
    [1] = {0},
};

static const struct fwk_element *dw_apb_i2c_get_element_table(
    fwk_id_t module_id)
{
    return dw_apb_i2c_element_table;
}

struct fwk_module_config config_dw_apb_i2c = {
    .get_element_table = dw_apb_i2c_get_element_table,
};

static const struct fwk_element i2c_element_table[] = {
    [0] = {
        .name = "I2C",
        .data = &(struct mod_i2c_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DW_APB_I2C, 0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_DW_APB_I2C,
                MOD_DW_APB_I2C_API_IDX_DRIVER),
        },
    },
    [1] = {0},
};

static const struct fwk_element *i2c_get_element_table(fwk_id_t module_id)
{
    return i2c_element_table;
}

struct fwk_module_config config_i2c = {
    .get_element_table = i2c_get_element_table,
};
