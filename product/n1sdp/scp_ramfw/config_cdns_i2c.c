/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_cdns_i2c.h"
#include "config_clock.h"
#include "n1sdp_core.h"
#include "n1sdp_scp_mmap.h"

#include <mod_cdns_i2c.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <fmw_cmsis.h>

#include <stdbool.h>

#define I2C_DEVICE_COUNT 2
static const struct fwk_element i2c_element_desc_table[] = {
    [CDNS_SPD_I2C_IDX] = {
        .name = "SPD-I2C",
        .data = &((struct mod_cdns_i2c_device_config) {
            .reg_base = DIMM_SPD_I2C_BASE,
            .clock_rate_hz = OSC_FREQ_HZ,
            .bus_speed_hz = MOD_CDNS_I2C_SPEED_NORMAL,
            .ack_en = MOD_CDNS_I2C_ACK_ENABLE,
            .addr_size = MOD_CDNS_I2C_ADDRESS_7_BIT,
            .hold_mode = MOD_CDNS_I2C_HOLD_ON,
            .irq = 0,
            .callback_mod_id = FWK_ID_NONE_INIT,
            .fifo_depth = 8,
            .max_xfr_size = 15,
        }),
    },
    [CDNS_C2C_I2C_IDX] = {
        .name = "C2C-I2C",
        .data = &((struct mod_cdns_i2c_device_config) {
            .reg_base = SCP_I2C0_BASE,
            .clock_rate_hz = (50UL * FWK_MHZ),
            .bus_speed_hz = MOD_CDNS_I2C_SPEED_NORMAL,
            .ack_en = MOD_CDNS_I2C_ACK_ENABLE,
            .addr_size = MOD_CDNS_I2C_ADDRESS_7_BIT,
            .hold_mode = MOD_CDNS_I2C_HOLD_OFF,
            .irq = SCP_I2C0_IRQ,
            .callback_mod_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_N1SDP_C2C),
            .fifo_depth = 8,
            .max_xfr_size = 15,
        }),
    },
    [CDNS_I2C_IDX_COUNT] = { 0 }, /* Termination description. */
};

static const struct fwk_element *get_i2c_table(fwk_id_t module_id)
{
    unsigned int i;
    struct mod_cdns_i2c_device_config *config;

    for (i = 0; i < CDNS_I2C_IDX_COUNT; i++) {
        config =
            (struct mod_cdns_i2c_device_config *)i2c_element_desc_table[i].data;

        if ((i == CDNS_C2C_I2C_IDX) && n1sdp_is_multichip_enabled() &&
            (n1sdp_get_chipid() != 0x0)) {
            config->mode = MOD_CDNS_I2C_TARGET_MODE;
            config->target_addr = 0x14;
        } else {
            config->mode = MOD_CDNS_I2C_CONTROLLER_MODE;
        }
    }

    return i2c_element_desc_table;
}

const struct fwk_module_config config_cdns_i2c = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_i2c_table),
};
