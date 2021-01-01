/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rcar_alarm_idx.h"
#include "system_clock.h"
#include "rcar_irq.h"

#include <mod_timer.h>
#include <mod_rcar_arch_timer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_time.h>

static const struct fwk_element rcar_arch_timer_element_table[] = {
    [0] = {
        .name = "",
        .data = &(struct mod_arch_timer_dev_config) {
            .clock_id = FWK_ID_NONE_INIT,
        }
    },
    [1] = { 0 },
};

struct fwk_module_config config_rcar_arch_timer = {
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(rcar_arch_timer_element_table),
};

static const struct fwk_element timer_element_table[] = {
    [0] = {
        .name = "REFCLK",
        .sub_element_count = RCAR_ALARM_IDX_COUNT,
        .data = &(struct mod_timer_dev_config) {
            .id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_ARCH_TIMER, 0),
            .timer_irq = NS_PHYSICAL_TIMER_IRQ,
        },
    },
    [1] = { 0 },
};

struct fwk_time_driver fmw_time_driver(const void **ctx)
{
    return mod_arch_timer_driver(ctx, config_rcar_arch_timer.elements.table[0].data);
}

static const struct fwk_element *timer_get_element_table(fwk_id_t module_id)
{
    return timer_element_table;
}

struct fwk_module_config config_timer = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(timer_get_element_table),
};
