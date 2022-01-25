/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* 
 * Set of common DT macros for use within fwk modules and product
 * components.
 * Common usage patterns built from the base set in fwk_devicetree.h
 * 
 */

#include <fwk_devicetree.h>
#include <fwk_id.h>

/* Extract a single property for a compatible instance */
#define FWK_DT_INST_PROP(compat, inst, prop) DT_PROP(DT_INST(inst, compat), prop)

/* extract a single property if it exists, or set default */
#define FWK_DT_INST_PROP_DEFAULT(compat, inst, prop, default) \
    COND_CODE_1(DT_NODE_HAS_PROP(DT_INST(inst ,compat), prop), \
    (FWK_DT_INST_PROP(compat, inst, prop)), \
    (default) \
    )

/* Extract an array  indexed property for a compatible instance */
#define FWK_DT_INST_IDX_PROP(compat, inst, prop, idx) \
    DT_PROP_BY_IDX(DT_INST(inst, compat), prop, idx)

/* intialise a framework element id from a device tree property array */
#define FWK_DT_FWK_ID(compat, inst, prop) \
    FWK_ID_ELEMENT_INIT( \
        FWK_DT_INST_IDX_PROP(compat, inst, prop, 0), \
        FWK_DT_INST_IDX_PROP(compat, inst, prop, 1) \
        )

/* initialise a framework api ID from a device tree property array */
#define FWK_DT_FWK_API_ID(compat, inst, prop) \
    FWK_ID_API_INIT( \
        FWK_DT_INST_IDX_PROP(compat, inst, prop, 0), \
        FWK_DT_INST_IDX_PROP(compat, inst, prop, 1) \
        )

/* initialise a framework module if from a device tree property array */
#define FWK_DT_FWK_MOD_ID(compat, inst, prop) \
    FWK_ID_MODULE_INIT( \
        FWK_DT_INST_IDX_PROP(compat, inst, prop, 0) \
        )
        
/* Init either module if array len 1, or element if array len 2 */
#define FWT_DT_FWK_ID_MOD_OR_ELEM(compat, inst, prop) \
    COND_CODE_1(DT_PROP_LEN(DT_INST(inst, compat), prop), \
        (FWK_DT_FWK_MOD_ID(compat, inst, prop)), \
        (FWK_DT_FWK_ID(compat, inst, prop)) \
    )

/* 
 * Intialise a framework id from an optional device tree property array 
 * Uses a default FWK_ID_NONE if the property is not present on the node instance.
 */
#define FWK_DT_OPT_FWK_ID(compat, inst, prop) \
    COND_CODE_1(DT_NODE_HAS_PROP(DT_INST(inst ,compat), prop), \
        (FWK_DT_FWK_ID(compat, inst, prop)), \
        (FWK_ID_NONE_INIT) \
    )

/*
 * check that the table array matches the count of elements from DT
 */
#define FWK_DT_TABLE_CHECK(table_array, element_count) \
    ((sizeof(table_array) / sizeof(table_array[0])) == (element_count))

/* 
 * Force a compile time error if 'condition' is false.
 * results in no generated code if 'condition' is true.
 * Use to validate some generated elements that cannot be checked by C
 * pre-processor.
 * 
 * e.g. 
 * FWT_DT_BUILD_BUG_ON(!FWK_DT_TABLE_CHECK(my_table, has_4_elements))
 */
#define FWK_DT_BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
