/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_scp_mmap.h"
#include "morello_system_clock.h"

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

/*
 * PL011 module
 */

struct fwk_module_config config_pl011 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({
        [0] = {
            .name = "SCP-UART",
            .data =
                &(struct mod_pl011_element_cfg){
                    .reg_base = SCP_UART_BASE,
                    .baud_rate_bps = 115200,
                    .clock_rate_hz = CLOCK_RATE_REFCLK,
                    .clock_id = FWK_ID_NONE_INIT,
                    .pd_id = FWK_ID_NONE_INIT,
                },
        },

        [1] = { 0 },
    }),
};
