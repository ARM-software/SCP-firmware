/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "lcp_mmap.h"

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

struct fwk_module_config config_pl011 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({
        [0] = {
            .name = "lcp-uart",
            .data =
                &(struct mod_pl011_element_cfg){
                    .reg_base = LCP_UART_BASE,
                    .baud_rate_bps = 115200,
                    .clock_rate_hz = 24 * FWK_MHZ,
                },
        },

        [1] = { 0 },
    }),
};
