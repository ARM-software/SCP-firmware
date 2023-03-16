/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mod_power_domain.h>
#include <mod_system_power.h>

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fwk_dt_config_common.h>

/* Generates system power configuation tables, and system power state 
   tables from the device tree */

/* generate system power tables . 
 * 
e.g. creates:

    static const uint8_t sys_power_to_ppu_state[] = {
        [MOD_PD_STATE_ON]                     = (uint8_t)MOD_PD_STATE_ON,
        [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] =
            (uint8_t)MOD_SYSTEM_POWER_POWER_STATE_SLEEP0,
        [MOD_PD_STATE_OFF]                    = (uint8_t)MOD_PD_STATE_OFF,
    };

from 


    sys_pwr_state_table: sys-pwr-table@0 {
            compatible = "arm,scp-sys-power-table";
            label = "sys_power_to_ppu_state";
            power-states = <MOD_PD_STATE_ON MOD_SYSTEM_POWER_POWER_STATE_SLEEP0 MOD_PD_STATE_OFF>;
            power-values = <MOD_PD_STATE_ON MOD_SYSTEM_POWER_POWER_STATE_SLEEP0 MOD_PD_STATE_OFF>;
    };

Note: this creates a 4 element array, with indexs 0, 1, 3 filled in.

*/

#define SPT_COMPAT arm_scp_sys_power_table
#define SPT_NODE_ID(n) DT_INST(n, SPT_COMPAT)

#define SPT_TABLE_NAME(n) DT_STRING_TOKEN(SPT_NODE_ID(n), label)

/* generate a single table entry */
#define SPT_TABLE_ENTRY(node_id, prop, idx) \
    [DT_PROP_BY_IDX(node_id, power_states, idx)] = (uint8_t)DT_PROP_BY_IDX(node_id, prop, idx),

/* generator for a single table */
#define SPT_TABLE_INIT(n) \
static const uint8_t SPT_TABLE_NAME(n)[] = { \
    DT_FOREACH_PROP_ELEM(SPT_NODE_ID(n), power_values, SPT_TABLE_ENTRY) \
};

/* generate all arm_scp_pd_mask_table table instances */
DT_FOREACH_OKAY_INST_arm_scp_sys_power_table(SPT_TABLE_INIT)

/* generate system power device configuration */
/*

    sys-pwr-device@0 {
            compatible = "arm,scp-sys-power-dev";
            label = "system_power";
            scp-ids = <&elem_id FWK_MODULE_IDX_JUNO_PPU JUNO_PPU_DEV_IDX_SYSTOP>,
                      <&elem_id FWK_MODULE_IDX_JUNO_PPU MOD_JUNO_PPU_API_IDX_PD>;
            sys-state-table = <&sys_pwr_state_table>;
        };

generates:

    static const struct fwk_element system_power_element_table[] = {
        [0] = {
            .name = "",
            .data = &((struct mod_system_power_dev_config) {
                .sys_ppu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU,
                    JUNO_PPU_DEV_IDX_SYSTOP),
                .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PPU,
                    MOD_JUNO_PPU_API_IDX_PD),
                .sys_state_table = sys_power_to_ppu_state,
        }),
    },

    [1] = { 0 }, 
};*/

#define SPD_COMPAT arm_scp_sys_power_dev
#define SPD_PROP(n, prop) FWK_DT_INST_PROP(SPD_COMPAT, n , prop)

#define SPD_ELEM_DATA(n) \
    .sys_ppu_id = FWK_DT_PH_IDX_SCP_ID_ELEM(SPD_COMPAT, n, 0), \
    .api_id = FWK_DT_PH_IDX_SCP_ID_API(SPD_COMPAT, n, 1), \
    .sys_state_table = DT_STRING_TOKEN(SPD_PROP(n, sys_state_table), label), \

/* single element table entry */
#define SPD_ELEM_INIT(n) \
    [n] = {  \
        .name = SPD_PROP(n, label), \
        .data = &(struct mod_system_power_dev_config) { \
            SPD_ELEM_DATA(n) \
        } \
    },

/* system power dev table generator */
static struct fwk_element element_table[] = {
    /* Macro for array elements */
    DT_FOREACH_OKAY_INST_arm_scp_sys_power_dev(SPD_ELEM_INIT)
    /* last null element */
    [DT_N_INST_arm_scp_sys_power_dev_NUM_OKAY] = { 0 },
};

/* generate system power init data */
/*
    system-power-config@0 {
            compatible = "arm,scp-system-power";
            soc-wakeup-irq = <EXT_WAKEUP_IRQ>;
            scp-ids = <&module_id FWK_MODULE_IDX_JUNO_SYSTEM>,
                      <&elem_id FWK_MODULE_IDX_JUNO_SYSTEM 0>;
            init-pwr-state = <MOD_PD_STATE_ON>;

    };

initialises the sys_pwr_config_data structure.

*/
#define SYSPWR_COMPAT arm_scp_system_power 
#define SYSP_PROP(n, prop) FWK_DT_INST_PROP(SYSPWR_COMPAT, n ,prop)

static struct mod_system_power_config sys_pwr_config_data = {
    .soc_wakeup_irq = SYSP_PROP(0, soc_wakeup_irq),
    .driver_id = FWK_DT_PH_IDX_SCP_ID_MOD(SYSPWR_COMPAT, 0, 0),
    .driver_api_id = FWK_DT_PH_IDX_SCP_ID_API(SYSPWR_COMPAT, 0, 1),
    .initial_system_power_state = SYSP_PROP(0, init_pwr_state),
};

/* access data table from dynamic element callback */
#ifdef FWK_MODULE_GEN_DYNAMIC_SYSTEM_POWER
const struct fwk_element *_static_get_element_table_system_power() {
    return element_table;
}
#endif

/* config dt structure */
struct fwk_module_config config_dt_system_power = {
    .data = &sys_pwr_config_data,
#ifdef FWK_MODULE_GEN_DYNAMIC_SYSTEM_POWER
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_system_power),
#else
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(element_table),
#endif
};
