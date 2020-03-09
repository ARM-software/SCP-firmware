/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "n1sdp_scp_irq.h"
#include "n1sdp_scp_mmap.h"

#include <mod_n1sdp_i2c.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>

static const struct fwk_element i2c_element_desc_table[] = {
    [0] = {
        .name = "SPD-I2C",
        .data = &((struct mod_n1sdp_i2c_device_config) {
            .reg_base = DIMM_SPD_I2C_BASE,
            .clock_rate_hz = OSC_FREQ_HZ,
            .bus_speed_hz = MOD_N1SDP_I2C_SPEED_NORMAL,
            .mode = MOD_N1SDP_I2C_MASTER_MODE,
            .ack_en = MOD_N1SDP_I2C_ACK_ENABLE,
            .addr_size = MOD_N1SDP_I2C_ADDRESS_7_BIT,
            .hold_mode = MOD_N1SDP_I2C_HOLD_ON,
            .c2c_mode = false,
            .callback_mod_id = FWK_ID_NONE_INIT,
        }),
    },
    [1] = {
        .name = "C2C-I2C",
        .data = &((struct mod_n1sdp_i2c_device_config) {
            .reg_base = SCP_I2C0_BASE,
            .clock_rate_hz = (50UL * FWK_MHZ),
            .bus_speed_hz = MOD_N1SDP_I2C_SPEED_NORMAL,
            .mode = MOD_N1SDP_I2C_SLAVE_MODE,
            .ack_en = MOD_N1SDP_I2C_ACK_ENABLE,
            .addr_size = MOD_N1SDP_I2C_ADDRESS_7_BIT,
            .hold_mode = MOD_N1SDP_I2C_HOLD_OFF,
            .slave_addr = 0x14,
            .c2c_mode = true,
            .irq = SCP_I2C0_IRQ,
            .callback_mod_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_N1SDP_C2C),
        }),
    },
    [2] = { 0 }, /* Termination description. */
};

static const struct fwk_element *get_i2c_table(fwk_id_t module_id)
{
    return i2c_element_desc_table;
}

const struct fwk_module_config config_n1sdp_i2c = {
    .get_element_table = get_i2c_table,
};
