/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

static struct pl011_reg mod_reg;

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

enum mhu3_fake_device_idx {
    PL011_FAKE_ELEMENT_IDX_0,
    PL011_FAKE_ELEMENT_IDX_COUNT,
};

struct fwk_module_config config_pl011_ut = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({
        [0] = {
            .name = "UART OUTPUT",
            .data =
                &(struct mod_pl011_element_cfg){
                    .reg_base = (uintptr_t)&mod_reg,
                    .baud_rate_bps = 115200,
                    .clock_rate_hz = 24 * FWK_MHZ,
                },
        },

        [1] = { 0 },
    }),
};
