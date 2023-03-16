/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_clock.h>
#include <mod_power_domain.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_devicetree.h>
#include <fwk_dt_config_common.h>

#define CLK_DEV_COMPAT arm_scp_clk_dev

/*
 * driver id and api id are required, pd source id is optional
 * DT array order must match this.
 */
#define CFG_CLK_DEV_ELEM_DATA_PH(n) \
    .driver_id = FWK_DT_PH_IDX_SCP_ID_ELEM(CLK_DEV_COMPAT, n, 0), \
    .api_id = FWK_DT_PH_IDX_SCP_ID_ELEM(CLK_DEV_COMPAT, n, 1), \
    .pd_source_id = FWK_DT_PH_OPT_IDX_SCP_ID_ELEM(CLK_DEV_COMPAT, n, 2),

#define CFG_CLK_DEV_ELEM_INIT(n) \
    [n] = { \
    .name = FWK_DT_INST_PROP(CLK_DEV_COMPAT, n, name), \
    .data = &((struct mod_clock_dev_config) { \
        CFG_CLK_DEV_ELEM_DATA_PH(n) \
        }), \
    },

/* build tables if at least one instance in device tree */
#if DT_HAS_COMPAT_STATUS_OKAY(CLK_DEV_COMPAT)

/* data table */
static struct fwk_element clock_element_table[] = {
        // Macro for array elements 
        DT_FOREACH_OKAY_INST_arm_scp_clk_dev(CFG_CLK_DEV_ELEM_INIT)
        // last null element
        [DT_N_INST_arm_scp_clk_dev_NUM_OKAY] = { 0 },
};

/* access data table from dynamic element callback */
#ifdef FWK_MODULE_GEN_DYNAMIC_CLOCK
const struct fwk_element *_static_get_element_table_clock() {
    return clock_element_table;
}
#endif

/* config structure */
struct fwk_module_config config_dt_clock = {
    #ifdef FWK_MODULE_GEN_DYNAMIC_CLOCK
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_clock),
    #else
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(clock_element_table),
    #endif
};

#else 
/* no DT references - set up NULL object */
struct fwk_module_config config_dt_clock = { 0 };
#endif
