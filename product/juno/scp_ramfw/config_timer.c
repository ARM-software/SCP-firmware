/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_alarm_idx.h"
#include "juno_irq.h"
#include "system_clock.h"
#include "system_mmap.h"

#include <mod_gtimer.h>
#include <mod_timer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element gtimer_element_table[] = {
    [0] = {
        .name = "",
        .data = &(struct mod_gtimer_dev_config) {
            .hw_timer = REFCLK_CNTBASE0_BASE,
            .hw_counter = REFCLK_CNTCTL_BASE,
            .control = REFCLK_CNTCONTROL_BASE,
            .frequency = CLOCK_RATE_REFCLK,
            .clock_id = FWK_ID_NONE_INIT,
        }
    },
    [1] = { 0 },
};

static const struct fwk_element *gtimer_get_element_table(fwk_id_t module_id)
{
    return gtimer_element_table;
}

struct fwk_module_config config_gtimer = {
    .get_element_table = gtimer_get_element_table,
};

static const struct fwk_element timer_element_table[] = {
    [0] = {
        .name = "REFCLK",
        .sub_element_count = JUNO_ALARM_IDX_COUNT,
        .data = &(struct mod_timer_dev_config) {
            .id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_GTIMER, 0),
            .timer_irq = TIMREFCLK_IRQ,
        },
    },
    [1] = { 0 },
};

static const struct fwk_element *timer_get_element_table(fwk_id_t module_id)
{
    return timer_element_table;
}

struct fwk_module_config config_timer = {
    .get_element_table = timer_get_element_table,
};
