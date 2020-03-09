/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_juno_ram.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

struct fwk_module_config config_juno_ram = {
    .data = &(struct mod_juno_ram_config) {
        .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
        },
};
