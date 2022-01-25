/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_pl011.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_devicetree.h>
#include <fwk_dt_config.h>
#include <fwk_dt_config_dyn.h>


#define PL011_COMPAT arm_pl011

#define PL011_PROP(n, prop) DT_PROP(DT_INST(n, PL011_COMPAT), prop)

#ifdef BUILD_HAS_MOD_CLOCK

#define CFG_PL011_ELEMENT_DATA(n) \
    .reg_base = DT_REG_ADDR_BY_IDX(DT_INST(n, PL011_COMPAT), 0),    \
    .baud_rate_bps = PL011_PROP(n, current_speed), \
    .clock_rate_hz = PL011_PROP(n, clock_frequency), \
    .clock_id = FWK_ID_NONE_INIT, \
    .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, \
                                 PL011_PROP(n, pd_id) \
                                 ),

#else

#define CFG_PL011_ELEMENT_DATA(n)                                   \
    .reg_base = DT_REG_ADDR_BY_IDX(DT_INST(n, PL011_COMPAT), 0),    \
    .baud_rate_bps = PL011_PROP(n, current_speed), \
    .clock_rate_hz = PL011_PROP(n, clock_frequency), \
    .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, \
                                 PL011_PROP(n, pd_id) \
                                 ),

#endif

#define CFG_PL011_ELEMENT_INIT(n) \
    [n] = { \
        .name = PL011_PROP(n, label), \
        .data = &(struct mod_pl011_element_cfg) { \
            CFG_PL011_ELEMENT_DATA(n)             \
        },                                        \
   },

/* data table */
static struct fwk_element pl011_element_table[] = {
            // Macro for array elements
            DT_FOREACH_OKAY_INST_arm_pl011(CFG_PL011_ELEMENT_INIT)
            // last null element
            [DT_N_INST_arm_pl011_NUM_OKAY] = { 0 },
};


/* access data table from dynamic element callback */
#ifdef FWK_MODULE_GEN_DYNAMIC_PL011
const struct fwk_element *_static_get_element_table_pl011() {
    return pl011_element_table;
}
#endif

/* config structure */
struct fwk_module_config config_dt_pl011 = {
    #ifdef FWK_MODULE_GEN_DYNAMIC_PL011
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_pl011),
    #else
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(pl011_element_table),
    #endif
};

/*struct fwk_module_config config_dt_pl011 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS ({
            // Macro for array elements
            DT_FOREACH_OKAY_INST_arm_pl011(CFG_PL011_ELEMENT_INIT)
                // last null element
            [DT_N_INST_arm_pl011_NUM_OKAY] = { 0 },
    }),
};
*/          

/* - original declaration in juno pl011 
struct fwk_module_config config_pl011 = {
    .elements = FWK_MODULE_STATIC_ELEMENTS({
        [0] = {
            .name = "",
            .data =
                &(struct mod_pl011_element_cfg){
                    .reg_base = FPGA_UART1_BASE,
                    .baud_rate_bps = 115200,
                    .clock_rate_hz = 24 * FWK_MHZ,
                    .clock_id = FWK_ID_NONE_INIT,
                    .pd_id = FWK_ID_ELEMENT_INIT(
                        FWK_MODULE_IDX_POWER_DOMAIN,
                        POWER_DOMAIN_IDX_SYSTOP),
                },
        },

        [1] = { 0 },
    }),
};
*/
