/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_mmap.h"

#include <mod_pl011.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>

struct fwk_module_config config_pl011 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({
        [0] = {
            .name = "scp-uart0",
            .data =
                &(struct mod_pl011_element_cfg){
                    .reg_base = SCP_UART_BASE,
                    .baud_rate_bps = 115200,
                    .clock_rate_hz = 62500 * FWK_KHZ,
                    .clock_id = FWK_ID_NONE_INIT,
                    .pd_id = FWK_ID_NONE_INIT,
                },
        },

        [1] = { 0 },
    }),
};
