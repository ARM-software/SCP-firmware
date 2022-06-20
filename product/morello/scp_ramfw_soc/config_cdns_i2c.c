/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_cdns_i2c.h"
#include "config_clock.h"
#include "morello_scp_mmap.h"

#include <mod_cdns_i2c.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <fmw_cmsis.h>

#include <stdbool.h>

static const struct fwk_element
    i2c_element_desc_table[CONFIG_CDNS_I2C_ELEMENT_IDX_COUNT + 1] = {
            [CONFIG_CDNS_I2C_ELEMENT_IDX_SPD] = {
                .name = "SPD-I2C",
                .data = &((struct mod_cdns_i2c_device_config) {
                    .reg_base = DIMM_SPD_I2C_BASE,
                    .clock_rate_hz = OSC_FREQ_HZ,
                    .bus_speed_hz = MOD_CDNS_I2C_SPEED_NORMAL,
                    .mode = MOD_CDNS_I2C_CONTROLLER_MODE,
                    .ack_en = MOD_CDNS_I2C_ACK_ENABLE,
                    .addr_size = MOD_CDNS_I2C_ADDRESS_7_BIT,
                    .hold_mode = MOD_CDNS_I2C_HOLD_ON,
                    .callback_mod_id = FWK_ID_NONE_INIT,
                    .fifo_depth = 8,
                    .max_xfr_size = 15,
                }),
            },
            [CONFIG_CDNS_I2C_ELEMENT_IDX_C2C] = {
                .name = "C2C-I2C",
                .data = &((struct mod_cdns_i2c_device_config) {
                    .reg_base = SCP_I2C0_BASE,
                    .clock_rate_hz = (50UL * FWK_MHZ),
                    .bus_speed_hz = MOD_CDNS_I2C_SPEED_NORMAL,
                    .mode = MOD_CDNS_I2C_TARGET_MODE,
                    .ack_en = MOD_CDNS_I2C_ACK_ENABLE,
                    .addr_size = MOD_CDNS_I2C_ADDRESS_7_BIT,
                    .hold_mode = MOD_CDNS_I2C_HOLD_OFF,
                    .target_addr = 0x14,
                    .irq = SCP_I2C0_IRQ,
                    .callback_mod_id = FWK_ID_NONE_INIT,
                    .fifo_depth = 8,
                    .max_xfr_size = 15,
                }),
            },
            [CONFIG_CDNS_I2C_ELEMENT_IDX_PMIC_OSC] = {
                .name = "PMIC-OSC-I2C",
                .data = &((struct mod_cdns_i2c_device_config) {
                    .reg_base = SCP_I2C1_BASE,
                    .clock_rate_hz = OSC_FREQ_HZ,
                    .bus_speed_hz = MOD_CDNS_I2C_SPEED_NORMAL,
                    .mode = MOD_CDNS_I2C_CONTROLLER_MODE,
                    .ack_en = MOD_CDNS_I2C_ACK_ENABLE,
                    .addr_size = MOD_CDNS_I2C_ADDRESS_7_BIT,
                    .hold_mode = MOD_CDNS_I2C_HOLD_ON,
                    .callback_mod_id = FWK_ID_NONE_INIT,
                    .fifo_depth = 8,
                    .max_xfr_size = 15,
                }),
            },
            [CONFIG_CDNS_I2C_ELEMENT_IDX_PCC] = {
                .name = "PCC-I2C",
                .data = &((struct mod_cdns_i2c_device_config) {
                    .reg_base = SCP_I2C2_BASE,
                    .clock_rate_hz = (100UL * FWK_MHZ),
                    .bus_speed_hz = MOD_CDNS_I2C_SPEED_NORMAL,
                    .mode = MOD_CDNS_I2C_CONTROLLER_MODE,
                    .ack_en = MOD_CDNS_I2C_ACK_ENABLE,
                    .addr_size = MOD_CDNS_I2C_ADDRESS_7_BIT,
                    .hold_mode = MOD_CDNS_I2C_HOLD_ON,
                    .callback_mod_id = FWK_ID_NONE_INIT,
                    .fifo_depth = 8,
                    .max_xfr_size = 15,
                 })
            },
            /* Termination description */
            [CONFIG_CDNS_I2C_ELEMENT_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_i2c_table(fwk_id_t module_id)
{
    return i2c_element_desc_table;
}

const struct fwk_module_config config_cdns_i2c = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_i2c_table),
};
