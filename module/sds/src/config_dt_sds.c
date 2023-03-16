/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mod_sds.h>

#include <fwk_macros.h>
#include <fwk_module.h>

#include <fwk_dt_config_common.h>

#define SDS_R_COMPAT arm_scp_sds_region
#define SDS_R_NODE_ID(n) DT_INST(n, SDS_R_COMPAT)
#define SDS_R_PROP(n, prop) FWK_DT_INST_PROP(SDS_R_COMPAT, n, prop)

/* sds region table single entry */
#define SDS_R_TABLE_ENT(n) \
    [SDS_R_PROP(n, region_idx)] = { \
        .base = (void *)DT_REG_ADDR_BY_IDX(SDS_R_NODE_ID(n), 0), \
        .size = DT_REG_SIZE_BY_IDX(SDS_R_NODE_ID(n), 0), \
    },

/* sds region table generator */
static const struct mod_sds_region_desc sds_module_regions[] = {
    DT_FOREACH_OKAY_INST_arm_scp_sds_region(SDS_R_TABLE_ENT)
};

/* sds config data */
static const struct mod_sds_config sds_module_config = {
    .regions = sds_module_regions,
    .region_count = (unsigned int)DT_N_INST_arm_scp_sds_region_NUM_OKAY,
    .clock_id = FWK_ID_NONE_INIT,
};

/* SDS element table */
#define SDS_COMPAT arm_scp_sds
#define SDS_PROP(n, prop) FWK_DT_INST_PROP(SDS_COMPAT, n, prop)
#define SDS_NODE_ID(n) DT_INST(n, SDS_COMPAT)

#define SDS_ELEM_INIT(n) \
    [SDS_PROP(n, table_idx)] = { \
        .name = SDS_PROP(n ,label), \
        .data = &(struct mod_sds_structure_desc) { \
            .id = (uint32_t)DT_PROP_BY_IDX(SDS_NODE_ID(n), data_id_size, 0), \
            .size = DT_PROP_BY_IDX(SDS_NODE_ID(n), data_id_size, 1), \
            .region_id = SDS_PROP(n, region_id), \
            .payload = NULL, \
            .finalize = SDS_PROP(n, finalize), \
        }, \
    },

static struct fwk_element element_table[] = {
    /* macro for array elements */
    DT_FOREACH_OKAY_INST_arm_scp_sds(SDS_ELEM_INIT)
    /* last NULL element */
    [DT_N_INST_arm_scp_sds_NUM_OKAY] = { 0 },
};

/* access data table from dynamic element callback */
#ifdef FWK_MODULE_GEN_DYNAMIC_SDS
const struct fwk_element *_static_get_element_table_sds() {
    return element_table;
}
#endif

/* config structure */
struct fwk_module_config config_dt_sds = {
    .data = &sds_module_config,
#ifdef FWK_MODULE_GEN_DYNAMIC_SDS
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_sds),
#else
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(element_table),
#endif
};
