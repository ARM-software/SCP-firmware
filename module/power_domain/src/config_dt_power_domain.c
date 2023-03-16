/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mod_power_domain.h>

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fwk_dt_config_common.h>

#ifndef POWER_DOMAIN_IDX_NONE
#define POWER_DOMAIN_IDX_NONE UINT32_MAX
#endif


/* Generates Power Domain configuation tables, and power domain state mask 
   tables from the device tree */

/* generate PD mask tables . 
 * Masks of the allowed states for the cluster power domain depending on the
 * PD state level

e.g. creates:

    static const uint32_t pd_cluster_masks[] = {
        [MOD_PD_STATE_OFF] = (uint32_t)MOD_PD_STATE_OFF_MASK,
        [MOD_PD_STATE_ON] = (uint32_t)JUNO_CLUSTER_VALID_STATE_MASK,
        [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = (uint32_t)MOD_PD_STATE_OFF_MASK,
};

from 

        pd_masks_cluster: pd-mask-table@2 {
            compatible = "arm,scp-pd-mask-table";
            label = "pd_cluster_masks";
            pd-states = <MOD_PD_STATE_OFF MOD_PD_STATE_ON MOD_SYSTEM_POWER_POWER_STATE_SLEEP0>;
            mask-values = <MOD_PD_STATE_OFF_MASK JUNO_CLUSTER_VALID_STATE_MASK MOD_PD_STATE_OFF_MASK>;            
        };

Note: this creates a 4 element array, with indexs 0, 1, 3 filled in.

Mask state tables are then referenced from the power domain config table entries.

*/

#define MT_COMPAT arm_scp_pd_mask_table
#define MT_NODE_ID(n) DT_INST(n, MT_COMPAT)

#define MT_TABLE_NAME(n) DT_STRING_TOKEN(MT_NODE_ID(n), label)

/* generate a single table entry */
#define MT_TABLE_ENTRY(node_id, prop, idx) \
    [DT_PROP_BY_IDX(node_id, pd_states, idx)] = (uint32_t)DT_PROP_BY_IDX(node_id, prop, idx),

/* generator for a single table */
#define MT_TABLE_INIT(n) \
static const uint32_t MT_TABLE_NAME(n)[] = { \
    DT_FOREACH_PROP_ELEM(MT_NODE_ID(n), mask_values, MT_TABLE_ENTRY) \
};

/* generate all arm_scp_pd_mask_table table instances */
DT_FOREACH_OKAY_INST_arm_scp_pd_mask_table(MT_TABLE_INIT)

/* generate PD config table */

/* create power config table entries:-
e.g.

   [POWER_DOMAIN_IDX_BIG_CPU1] = {
        .name = "BIG_CPU1",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .parent_idx = (uint32_t) POWER_DOMAIN_IDX_BIG_SSTOP,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_BIG_CPU1),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
            .disable_state_transition_notifications = true,
        },

from
        power-domain@1 {
            compatible = "arm,scp-power-domain";
            label = "BIG_CPU1";
            pd-type = <MOD_PD_TYPE_CORE>;
            pd-idx = <POWER_DOMAIN_IDX_BIG_CPU1>;
            pd-parent = <&pd_big_sstop>;
            scp-ids =  <&elem_id FWK_MODULE_IDX_JUNO_PPU JUNO_PPU_DEV_IDX_BIG_CPU1>,
                       <&elem_id FWK_MODULE_IDX_JUNO_PPU MOD_JUNO_PPU_API_IDX_PD>;
            allowed-mask-table = <&pd_masks_core>;
            disable-state-trans-notifications;
        };
*/

#define PD_COMPAT arm_scp_power_domain
#define PD_PROP(n, prop) FWK_DT_INST_PROP(PD_COMPAT, n ,prop)

/* parent pd_idx if parent phandle present, otherwise default to NONE */
#define PD_PARENT_IDX(n) \
    COND_CODE_1(DT_NODE_HAS_PROP(DT_INST(n, PD_COMPAT), pd_parent), \
        (DT_PROP(PD_PROP(n, pd_parent), pd_idx)), \
        (POWER_DOMAIN_IDX_NONE) \
    )

/* single element .data structure */
#define PD_ELEM_DATA(n) \
        .attributes.pd_type = PD_PROP(n, pd_type), \
        .parent_idx = (uint32_t)PD_PARENT_IDX(n), \
        .driver_id =  FWK_DT_PH_IDX_SCP_ID_MOD_OR_ELEM(PD_COMPAT, n, 0), \
        .api_id = FWK_DT_PH_IDX_SCP_ID_API(PD_COMPAT, n, 1), \
        .allowed_state_mask_table = DT_STRING_TOKEN(PD_PROP(n, allowed_mask_table), label), \
        .allowed_state_mask_table_size = DT_PROP_LEN(PD_PROP(n, allowed_mask_table), mask_values), \
        .disable_state_transition_notifications = PD_PROP(n, disable_state_trans_notifications),

/* single element table entry */
#define PD_ELEM_INIT(n) \
    [PD_PROP(n, pd_idx)] = {  \
        .name = PD_PROP(n, label), \
        .data = &(struct mod_power_domain_element_config) { \
            PD_ELEM_DATA(n) \
        } \
    },

/* table generator */
static struct fwk_element element_table[] = {
    /* Macro for array elements */
    DT_FOREACH_OKAY_INST_arm_scp_power_domain(PD_ELEM_INIT)
    /* last null element */
    [DT_N_INST_arm_scp_power_domain_NUM_OKAY] = { 0 },
};

/* access data table from dynamic element callback */
#ifdef FWK_MODULE_GEN_DYNAMIC_POWER_DOMAIN
const struct fwk_element *_static_get_element_table_power_domain() {
    return element_table;
}
#endif

/* config structure */
struct fwk_module_config config_dt_power_domain = {
#ifdef FWK_MODULE_GEN_DYNAMIC_POWER_DOMAIN
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_power_domain),
#else
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(element_table),
#endif
};
