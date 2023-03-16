/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_timer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_dt_config_common.h>

#define TIMER_COMPAT arm_scp_timer

#define CFG_TIMER_ELEM_DATA(n) \
    .id = FWK_DT_PH_IDX_SCP_ID_MOD_OR_ELEM(TIMER_COMPAT, n, 0), \
    .timer_irq = FWK_DT_INST_PROP(TIMER_COMPAT, n, timer_irq),

#define CFG_TIMER_ELEM_INIT(n) \
    [n] = { \
    .name = FWK_DT_INST_PROP(TIMER_COMPAT, n, label), \
    .sub_element_count = (size_t) FWK_DT_INST_PROP_DEFAULT(TIMER_COMPAT, n, sub_elem_count, 0), \
    .data = &((struct mod_timer_dev_config) { \
        CFG_TIMER_ELEM_DATA(n) \
        }), \
    },

/* data table */
static struct fwk_element timer_element_table[] = {
        // Macro for array elements 
        DT_FOREACH_OKAY_INST_arm_scp_timer(CFG_TIMER_ELEM_INIT)
        // last null element
        [DT_N_INST_arm_scp_timer_NUM_OKAY] = { 0 },
};

/* access data table from dynamic element callback */
#ifdef FWK_MODULE_GEN_DYNAMIC_TIMER
const struct fwk_element *_static_get_element_table_timer() {
    return timer_element_table;
}
#endif

/* config structure */
struct fwk_module_config config_dt_timer = {
    #ifdef FWK_MODULE_GEN_DYNAMIC_TIMER
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_timer),
    #else
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(timer_element_table),
    #endif
};
