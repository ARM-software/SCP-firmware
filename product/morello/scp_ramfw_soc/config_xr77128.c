/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_cdns_i2c.h"
#include "config_psu.h"
#include "config_xr77128.h"

#include <morello_alarm_idx.h>

#include <mod_cdns_i2c.h>
#include <mod_morello_sensor.h>
#include <mod_psu.h>
#include <mod_xr77128.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define XR77128_BUS_CHANNEL_IDX_CLUS0 1
#define XR77128_BUS_CHANNEL_IDX_CLUS1 3

#define PSU_MAX_VOUT_MV 1100

static struct mod_xr77128_channel_config
                channel_config[XR77128_CHANNEL_COUNT] = {
        [0] = {
            .driver_response_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
                CONFIG_PSU_ELEMENT_IDX_CLUS0),
            .driver_response_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PSU,
                MOD_PSU_API_IDX_DRIVER_RESPONSE),
            .psu_bus_idx = XR77128_BUS_CHANNEL_IDX_CLUS0,
            .alarm_hal_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
                MORELLO_PSU_XR77128_CLUS0_ALARM_IDX),
            .psu_max_vout = PSU_MAX_VOUT_MV,
            .enabled = true,
        },

        [1] = {
            .driver_response_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PSU,
                CONFIG_PSU_ELEMENT_IDX_CLUS1),
            .driver_response_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PSU,
                MOD_PSU_API_IDX_DRIVER_RESPONSE),
            .psu_bus_idx = XR77128_BUS_CHANNEL_IDX_CLUS1,
            .alarm_hal_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0,
                MORELLO_PSU_XR77128_CLUS1_ALARM_IDX),
            .psu_max_vout = PSU_MAX_VOUT_MV,
            .enabled = true,
        },
};

static const struct fwk_element
    xr77128_element_table[CONFIG_XR77128_ELEMENT_IDX_COUNT + 1] = {
        [CONFIG_XR77128_ELEMENT_IDX_0] = {
            .name = "SOC-PMIC",
            .data =
                &(struct mod_xr77128_dev_config){
                    .target_addr = 0x28,
                    .i2c_hal_id = FWK_ID_ELEMENT_INIT(
                        FWK_MODULE_IDX_CDNS_I2C,
                        CONFIG_CDNS_I2C_ELEMENT_IDX_PMIC_OSC),
                    .i2c_api_id = FWK_ID_API_INIT(
                        FWK_MODULE_IDX_CDNS_I2C,
                        MOD_CDNS_I2C_API_CONTROLLER_POLLED),
                    .channels = &channel_config[0],
                },
            /* All PSU channels except the GPU */
            .sub_element_count = CONFIG_PSU_ELEMENT_IDX_COUNT - 1,
        },

        [CONFIG_XR77128_ELEMENT_IDX_COUNT] = { 0 },
};

static const struct fwk_element *xr77128_get_element_table(fwk_id_t module_id)
{
    return xr77128_element_table;
}

const struct fwk_module_config config_xr77128 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(xr77128_get_element_table),
    .data = NULL,
};
