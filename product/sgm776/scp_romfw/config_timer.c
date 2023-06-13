/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_devices.h"
#include "sgm776_mmap.h"
#include "system_clock.h"

#include <mod_gtimer.h>
#include <mod_timer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_time.h>

/*
 * Generic timer driver config
 */
static const struct fwk_element gtimer_dev_table[] = {
    [0] = { .name = "REFCLK",
            .data = &((struct mod_gtimer_dev_config){
                .hw_timer = REFCLK_CNTBASE0_BASE,
                .hw_counter = REFCLK_CNTCTL_BASE,
                .control = REFCLK_CNTCONTROL_BASE,
                .frequency = CLOCK_RATE_REFCLK,
                .clock_id = FWK_ID_NONE_INIT,
            }) },
    [1] = { 0 },
};

struct fwk_module_config config_gtimer = {
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(gtimer_dev_table),
};

struct fwk_time_driver fmw_time_driver(const void **ctx)
{
    return mod_gtimer_driver(ctx, config_gtimer.elements.table[0].data);
}

/*
 * Timer HAL config
 */
static const struct mod_timer_dev_config refclk_config = {
    .id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_GTIMER, 0),
};

static const struct fwk_element timer_dev_table[] = {
    [0] = {
        .name = "REFCLK",
        .data = &refclk_config,
    },
    [1] = { 0 },
};

static const struct fwk_element *timer_get_dev_table(fwk_id_t module_id)
{
    return timer_dev_table;
}

struct fwk_module_config config_timer = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(timer_get_dev_table),
};
