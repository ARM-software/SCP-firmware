/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tc1_timer.h"

#include <mod_timer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <fmw_cmsis.h>

/*
 * Timer HAL config
 */
static const struct fwk_element timer_dev_table[2] = {
    [0] = {
            .name = "REFCLK",
            .data = &((struct mod_timer_dev_config){
                .id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_GTIMER, 0),
                .timer_irq = TIMREFCLK_IRQ,
            }),
            /* Number of alarms */
            .sub_element_count = TC1_CONFIG_TIMER_SUB_ELEMENT_IDX_COUNT,
    },
    [1] = { 0 },
};

static const struct fwk_element *timer_get_dev_table(fwk_id_t module_id)
{
    return timer_dev_table;
}

const struct fwk_module_config config_timer = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(timer_get_dev_table),
};
