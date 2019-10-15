/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <mod_n1sdp_i2c.h>
#include <n1sdp_scp_mmap.h>
#include <config_clock.h>

static const struct fwk_element i2c_element_desc_table[] = {
    [0] = {
        .name = "I2C0",
        .data = &((struct mod_n1sdp_i2c_device_config) {
            .reg_base = SCP_I2C0_BASE,
            .clock_rate_hz = OSC_FREQ_HZ,
            .bus_speed_hz = MOD_N1SDP_I2C_SPEED_NORMAL,
            .mode = MOD_N1SDP_I2C_MASTER_MODE,
            .ack_en = MOD_N1SDP_I2C_ACK_ENABLE,
            .addr_size = MOD_N1SDP_I2C_ADDRESS_7_BIT,
            .hold_mode = MOD_N1SDP_I2C_HOLD_ON,
        }),
    },
    [1] = { 0 }, /* Termination description. */
};

static const struct fwk_element *get_i2c_table(fwk_id_t module_id)
{
    return i2c_element_desc_table;
}

const struct fwk_module_config config_n1sdp_i2c = {
    .get_element_table = get_i2c_table,
};
