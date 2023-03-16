
/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_gtimer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_time.h>

#include <fwk_dt_config_common.h>

/* Framework Generic Timer configuration */
#define GTIMER_COMPAT arm_scp_gtimer

#define CFG_GTIMER_ELEM_DATA(n) \
    .hw_timer = FWK_DT_INST_IDX_PROP(GTIMER_COMPAT, n, timer_addr, 0),    \
    .hw_counter = FWK_DT_INST_IDX_PROP(GTIMER_COMPAT, n, counter_addr, 0), \
    .control = FWK_DT_INST_IDX_PROP(GTIMER_COMPAT, n, control_addr, 0), \
    .frequency = FWK_DT_INST_PROP(GTIMER_COMPAT, n, frequency), \
    .clock_id = FWK_DT_PH_OPT_IDX_SCP_ID_ELEM(GTIMER_COMPAT, n,  0),

#define CFG_GTIMER_ELEM_INIT(n) \
    [n] = { \
        .name = FWK_DT_INST_PROP(GTIMER_COMPAT, n, label), \
        .data = &(struct mod_gtimer_dev_config) { \
            CFG_GTIMER_ELEM_DATA(n) \
        }, \
    },

static const struct fwk_element gtimer_element_table[] = {
    // Macro for array elements 
    DT_FOREACH_OKAY_INST_arm_scp_gtimer(CFG_GTIMER_ELEM_INIT)
    // last null element
    [DT_N_INST_arm_scp_gtimer_NUM_OKAY] = { 0 },
};

/* 
 * Due to the gtimer specific static callback, 
 * we do not support a dynamic configuration option
 */
struct fwk_module_config config_dt_gtimer = {
    #ifdef FWK_MODULE_GEN_DYNAMIC_GTIMER
    #error("GTIMER module does not support dynamic configuration")
    #else
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(gtimer_element_table),
    #endif
};

/* GTIMER has specific static access callback */
struct fwk_time_driver fmw_time_driver(const void **ctx)
{
    return mod_gtimer_driver(ctx, config_dt_gtimer.elements.table[0].data);
}
