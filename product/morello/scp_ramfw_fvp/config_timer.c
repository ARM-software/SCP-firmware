/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "morello_alarm_idx.h"
#include "morello_scp_mmap.h"
#include "morello_system_clock.h"

#include <mod_gtimer.h>
#include <mod_timer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <fmw_cmsis.h>

/*
 * Generic timer driver config
 */
static const struct fwk_element gtimer_dev_table[] = {
    [0] = { .name = "REFCLK",
            .data = &((struct mod_gtimer_dev_config){
                .hw_timer = SCP_REFCLK_CNTBASE0_BASE,
                .hw_counter = SCP_REFCLK_CNTCTL_BASE,
                .control = SCP_REFCLK_CNTCONTROL_BASE,
                .frequency = CLOCK_RATE_REFCLK,
                .clock_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CLOCK,
                    CLOCK_IDX_INTERCONNECT) }) },
    [1] = { 0 },
};

static const struct fwk_element *gtimer_get_dev_table(fwk_id_t module_id)
{
    return gtimer_dev_table;
}

const struct fwk_module_config config_gtimer = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(gtimer_get_dev_table),
};

/*
 * Timer HAL config
 */
static const struct fwk_element timer_dev_table[] = {
    [0] =
        {
            .name = "REFCLK",
            .data = &((struct mod_timer_dev_config){
                .id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_GTIMER, 0),
                .timer_irq = TIMREFCLK_IRQ,
            }),
            .sub_element_count = MORELLO_ALARM_IDX_COUNT, /* Number of alarms */
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
