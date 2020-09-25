/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "scp_css_mmap.h"

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

const struct fwk_module_config config_pl011 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({
        [0] = {
            .name = "uart",
            .data =
                &(struct mod_pl011_element_cfg){
                    .reg_base = SCP_UART_BOARD_BASE,
                    .baud_rate_bps = 115200,
                    .clock_rate_hz = 24 * FWK_MHZ,
                    .clock_id = FWK_ID_NONE_INIT,
                    .pd_id = FWK_ID_ELEMENT_INIT(
                        FWK_MODULE_IDX_POWER_DOMAIN,
                        PD_STATIC_DEV_IDX_SYSTOP),
                },
        },

        [1] = { 0 },
    }),
};
