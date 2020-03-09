/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "system_clock.h"
#include "system_mmap.h"

#include <mod_gtimer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element element_table[] = {
    [0] = {
        .name = "REFCLK",
        .data = &(struct mod_gtimer_dev_config) {
            .hw_timer   = REFCLK_CNTBASE0_BASE,
            .hw_counter = REFCLK_CNTCTL_BASE,
            .control    = REFCLK_CNTCONTROL_BASE,
            .frequency  = CLOCK_RATE_REFCLK,
            .clock_id   = FWK_ID_NONE_INIT,
        }
    },
    [1] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_gtimer = {
    .get_element_table = get_element_table,
};
