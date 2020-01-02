/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_id.h>
#include <fwk_module.h>
#include <mod_gtimer.h>
#include <scp_sgi575_mmap.h>
#include <system_clock.h>

/*
 * Generic timer driver config
 */
static const struct fwk_element gtimer_dev_table[] = {
    [0] = {
        .name = "REFCLK",
        .data = &((struct mod_gtimer_dev_config) {
            .hw_timer   = SCP_REFCLK_CNTBASE0_BASE,
            .hw_counter = SCP_REFCLK_CNTCTL_BASE,
            .control    = SCP_REFCLK_CNTCONTROL_BASE,
            .frequency  = CLOCK_RATE_REFCLK,
            .clock_id   = FWK_ID_NONE_INIT,
        })
    },
    [1] = { 0 },
};

static const struct fwk_element *gtimer_get_dev_table(fwk_id_t module_id)
{
    return gtimer_dev_table;
}

const struct fwk_module_config config_gtimer = {
    .get_element_table = gtimer_get_dev_table,
};
