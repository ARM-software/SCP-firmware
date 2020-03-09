/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_css_mmap.h"

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

static const struct fwk_element pl011_element_desc_table[] = {
       [0] = {
           .name = "uart",
           .data = &((struct mod_pl011_device_config) {
               .reg_base = SCP_UART_BASE,
               .baud_rate_bps = 115200,
               .clock_rate_hz = 24 * FWK_MHZ,
               .clock_id = FWK_ID_NONE_INIT,
               .pd_id = FWK_ID_NONE_INIT,
            }),
       },
       [1] = { 0 },
};

static const struct fwk_element *get_pl011_table(fwk_id_t id)
{
    return pl011_element_desc_table;
}

const struct fwk_module_config config_pl011 = {
    .get_element_table = get_pl011_table,
};
