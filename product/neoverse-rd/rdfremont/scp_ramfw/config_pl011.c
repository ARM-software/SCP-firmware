/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'pl011'.
 */

#include "scp_css_mmap.h"

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

/* Module 'PL011' element count */
#define MOD_PL011_ELEMENT_COUNT 2

static const struct fwk_element pl011_table[MOD_PL011_ELEMENT_COUNT] = {
    {
        .name = "scp_uart",
        .data =
            &(struct mod_pl011_element_cfg){
                .reg_base = SCP_UART_BASE,
                .baud_rate_bps = 115200,
                .clock_rate_hz = 24 * FWK_MHZ,
                .clock_id = FWK_ID_NONE_INIT,
                .pd_id = FWK_ID_NONE_INIT,
            },
    },
    { 0 },
};

const struct fwk_module_config config_pl011 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(pl011_table),
};
