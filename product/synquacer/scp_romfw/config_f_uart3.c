/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
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
                &(struct mod_f_uart3_element_cfg){
                    .reg_base = F_UART3_BASE_ADDR,
                    .dla_reg_base = F_UART3_BASE_ADDR,
                    .parity_enable_flag = false,
                    .even_parity_flag = false,
                    .baud_rate = MOD_F_UART3_BAUD_RATE_115200,
                },
        },

        [1] = { 0 },
    }),
};
