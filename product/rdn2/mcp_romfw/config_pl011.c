/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mcp_platform_mmap.h"

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

static const struct fwk_element config_pl011_elements[] = {
    [0] = {
        .name = "uart",
        .data =
            &(struct mod_pl011_element_cfg){
                .reg_base = MCP_UART0_BASE,
                .baud_rate_bps = 115200,
                .clock_rate_hz = 24 * FWK_MHZ,
                .clock_id = FWK_ID_NONE_INIT,

#ifdef BUILD_HAS_MOD_POWER_DOMAIN
                .pd_id = FWK_ID_NONE_INIT,
#endif
            },
    },

    [1] = { 0 },
};

const struct fwk_module_config config_pl011 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(config_pl011_elements),
};
