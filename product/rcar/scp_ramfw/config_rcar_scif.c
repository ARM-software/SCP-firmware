/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <system_mmap.h>

#include <mod_rcar_scif.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

struct fwk_module_config config_rcar_scif = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({
        [0] =
            {
                .name = "board-uart1",
                .data =
                    &(struct mod_rcar_scif_element_cfg){
                        .reg_base = BOARD_UART1_BASE,
                    },
            },

        [1] = { 0 },
    }),
};
