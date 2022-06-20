/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_xr77128.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct mod_xr77128_channel_config channel_config[FWK_CHANNEL_IDX_COUNT] = {
        [FWK_CHANNEL_IDX_DUMMY] = {
            .driver_response_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU, 0),
            .driver_response_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PSU, 0),
            .psu_bus_idx = 1,
            .alarm_hal_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 0),
            .psu_max_vout = 1100,
            .enabled = true,
        },
};
static const struct fwk_element xr77128_element_table[FWK_ELEMENT_IDX_COUNT] = {
        [FWK_ELEMENT_IDX_DUMMY] = {
            .name = "SOC-PMIC",
            .data = &(struct mod_xr77128_dev_config){
                    .target_addr = 0x28,
                    .i2c_hal_id = FWK_ID_ELEMENT_INIT(
                        FWK_MODULE_IDX_CDNS_I2C, 0),
                    .i2c_api_id = FWK_ID_API_INIT(
                        FWK_MODULE_IDX_CDNS_I2C, 0),
                    .channels = &channel_config[0],
                },
            .sub_element_count = 1,
        },
};
static const struct fwk_element *xr77128_get_element_table(fwk_id_t module_id)
{
    return xr77128_element_table;
}
const struct fwk_module_config config_xr77128 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(xr77128_get_element_table),
    .data = NULL,
};
