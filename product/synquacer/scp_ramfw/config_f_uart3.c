/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_mmap.h"

#include <mod_f_uart3.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

struct fwk_module_config config_f_uart3 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({
        [0] = {
            .name = "F_UART3",
            .data =
                &(struct mod_f_uart3_device_config){
                    .reg_base = F_UART3_BASE_ADDR,
                    .dla_reg_base = F_UART3_BASE_ADDR,
                    .baud_rate_bps = 115200,
                    .clock_rate_hz = 62500 * FWK_KHZ,
                    .clock_id = FWK_ID_NONE_INIT,
                },
        },

        [1] = { 0 },
    }),
};
