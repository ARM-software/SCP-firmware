/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_mcp_mmap.h"
#include "n1sdp_system_clock.h"

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

struct fwk_module_config config_pl011 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({
        [0] = {
            .name = "MCP-UART",
            .data =
                &(struct mod_pl011_element_cfg){
                    .reg_base = MCP_UART0_BASE,
                    .baud_rate_bps = 115200,
                    .clock_rate_hz = CLOCK_RATE_REFCLK,
                    .clock_id = FWK_ID_NONE_INIT,
                },
        },

        [1] = { 0 },
    }),
};
