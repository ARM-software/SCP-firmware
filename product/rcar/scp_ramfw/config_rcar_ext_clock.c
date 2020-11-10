/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clock_ext_devices.h>
#include <system_clock.h>

#include <mod_rcar_ext_clock.h>
#include <mod_clock.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

static const struct fwk_element rcar_ext_clock_element_table[] = {
    {
        .name = "x12_clk",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 24576000,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "x21_clk",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 33000000,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "x22_clk",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 33000000,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "x23_clk",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 25000000,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "audio_clkout",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 12288000,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "audio_clk_a",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 22579200,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "audio_clk_c",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 0,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "can_clk",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 0,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "pcie_bus_clk",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 100000000,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "scif_clk",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 14745600,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "usb3s0_clk",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 100000000,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    {
        .name = "usb_extal_clk",
        .data = &((struct mod_rcar_ext_clock_dev_config) {
            .clock_rate = 50000000,
            .clock_state = MOD_CLOCK_STATE_RUNNING,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *rcar_ext_clock_get_element_table
    (fwk_id_t module_id)
{
    return rcar_ext_clock_element_table;
}

struct fwk_module_config config_rcar_ext_clock = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(rcar_ext_clock_get_element_table),
};
