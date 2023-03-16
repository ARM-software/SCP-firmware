/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mock_clock.h>

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fwk_dt_config_common.h>

#define MCLK_COMPAT arm_scp_mock_clock
#define MCLK_PROP(n, prop) FWK_DT_INST_PROP(MCLK_COMPAT, n, prop)

/* rate table generator */
#define MCLK_RATE_TABLE_NAME(n) DT_CAT(rate_table_, n)
#define MCLK_RATE_TABLE_ENTRY(node_id, prop, idx) \
    { .rate = DT_PROP_BY_IDX(node_id, prop, idx) }, 

#define MCLK_RATE_TABLE_INIT(n) \
static const struct mod_mock_clock_rate MCLK_RATE_TABLE_NAME(n)[] = { \
    DT_FOREACH_PROP_ELEM(DT_INST(n, MCLK_COMPAT), rate_table, MCLK_RATE_TABLE_ENTRY) \
};

DT_FOREACH_OKAY_INST_arm_scp_mock_clock(MCLK_RATE_TABLE_INIT)

/* main data table generator */

#define MCLK_ELEM_DATA(n) \
    .rate_table = MCLK_RATE_TABLE_NAME(n), \
    .rate_count = DT_PROP_LEN(DT_INST(n, MCLK_COMPAT), rate_table), \
    .default_rate = MCLK_PROP(n, default_rate),

#define MCLK_ELEM_INIT(n) \
    [n] = { \
            .name = MCLK_PROP(n, label), \
            .data = &(const struct mod_mock_clock_element_cfg) { \
                MCLK_ELEM_DATA(n) \
            }, \
    },

static struct fwk_element element_table[] = {
    // Macro for array elements
    DT_FOREACH_OKAY_INST_arm_scp_mock_clock(MCLK_ELEM_INIT)
    // last null element
    [DT_N_INST_arm_scp_mock_clock_NUM_OKAY] = { 0 },
};

/* access data table from dynamic element callback */
#ifdef FWK_MODULE_GEN_DYNAMIC_MOCK_CLOCK
const struct fwk_element *_static_get_element_table_mock_clock() {
    return element_table;
}
#endif

/* config structure */
struct fwk_module_config config_dt_mock_clock = {
    #ifdef FWK_MODULE_GEN_DYNAMIC_MOCK_CLOCK
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_mock_clock),
    #else
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(element_table),
    #endif
};
