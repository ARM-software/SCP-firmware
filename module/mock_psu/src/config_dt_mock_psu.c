/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mod_mock_psu.h>

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fwk_dt_config_common.h>

#define PSU_COMPAT arm_scp_mock_psu
#define PSU_PROP(n, prop) FWK_DT_INST_PROP(PSU_COMPAT, n ,prop)

/* single element .data structure */
#define PSU_ELEM_DATA(n) \
        .async_alarm_id = FWK_DT_PH_OPT_IDX_SCP_ID_ELEM(PSU_COMPAT, n, 0), \
        .default_enabled = PSU_PROP(n, default_enabled), \
        .default_voltage = PSU_PROP(n, default_voltage),

/* single element table entry */
#define PSU_ELEM_INIT(n) \
    [PSU_PROP(n, elem_idx)] = {  \
        .name = PSU_PROP(n, label), \
        .data = &(struct mod_mock_psu_element_cfg) { \
            PSU_ELEM_DATA(n) \
        }, \
    },

/* table generator */
static struct fwk_element element_table[] = {
    /* Macro for array elements */
    DT_FOREACH_OKAY_INST_arm_scp_mock_psu(PSU_ELEM_INIT)
    /* last null element */
    [DT_N_INST_arm_scp_mock_psu_NUM_OKAY] = { 0 },
};

/* access data table using function */
const struct fwk_element *_static_get_element_table_mock_psu() {
    /* 
     * using elem_idx property to bind table array elements to 
     * framework indexes. Check these result in a properly formed table
     * which is not too small for the largest index.
     * This will cause a build error if table is incorrect.
     */
    FWK_DT_BUILD_BUG_ON(!FWK_DT_TABLE_CHECK(element_table, DT_N_INST_arm_scp_mock_psu_NUM_OKAY + 1));

    return element_table;
}

/* config structure */
struct fwk_module_config config_dt_mock_psu = {
#ifdef FWK_MODULE_GEN_DYNAMIC_MOCK_PSU
    /* use custom external element callback to allow runtime modification */
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_mock_psu),
#else
    /* use local access function */
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(_static_get_element_table_mock_psu),
#endif
};
