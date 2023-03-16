/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* SCP framework ID macros */
#include <fwk_id.h>

/* Base set of DT generator macros - from Zephyr project */
#include <fwk_devicetree.h>

/*
 * Generated macros from DT processing - generally reproduced in
 * source files using the generator macros
 */
#include <fwk_dt_config.h>

/* Generated API calls for modules using dynamic table type */
#include <fwk_dt_config_dyn.h>

/**
 * @brief fwk_dt_config_common.h API
 *
 * Set of common SCP specific DT generator macros for use within fwk modules
 * and product components.
 *
 * Common usage patterns built from the base set in fwk_devicetree.h
 *
 * Generator macros in the C source are used to reproduce macros generated in the DT processing
 * thus extracting the value in the device tree.
 *
 * @defgroup devicetree-scp-fwk Devicetree extensions for SCP firmware
 * @ingroup devicetree-scp-fwk
 * @{
 */

/**
 * @brief Extract a single property for a compatible instance
 * @param compat Compatible name
 * @param inst instance number
 * @param prop property name
 */
#define FWK_DT_INST_PROP(compat, inst, prop) DT_PROP(DT_INST(inst, compat), prop)

/**
 * @brief Extract a single property if it exists, or set default
 * @param compat Compatible name
 * @param inst instance number
 * @param prop property name
 * @param default default value
 *
 * Checks that 'prop' exists, if not the supplied default value will be used.
 * Typical use is to populate structures where optional device tree attibutes are
 * used. 
 *
 * Example usage:
 *
 *     static const struct mod_bootloader_config bootloader_module_config = {
 *          ...
 *          .sds_struct_id = FWK_DT_INST_PROP_DEFAULT(BOOTLOADER_COMPAT, 0, sds_id, 0),
 *     };
 */
#define FWK_DT_INST_PROP_DEFAULT(compat, inst, prop, default) \
    COND_CODE_1(DT_NODE_HAS_PROP(DT_INST(inst ,compat), prop), \
    (FWK_DT_INST_PROP(compat, inst, prop)), \
    (default) \
    )

/**
 * @brief Extract an array indexed property for a compatible instance
 * @param compat Compatible name
 * @param inst instance number
 * @param prop property name
 * @param idx array index
 */
#define FWK_DT_INST_IDX_PROP(compat, inst, prop, idx) \
    DT_PROP_BY_IDX(DT_INST(inst, compat), prop, idx)

/**
 * @defgroup devicetree-scp-fwk-scp-ids Framework IDs API
 *
 * The SCP framework uses 32 bit ID values as selectors / references between modules and
 * framework elements. There are base scp_id elements defined to allow structured definition
 * of the IDs. Specific ID numbers in module devicetree blocks use phandle refs to the base 
 * definitions, with the correct index numbers to define the SCP ID.
 *
 * The framework API begins with .yaml bindings:
 *
 *     description: SCP software framework module element ID 
 *
 *     compatible: "arm,scp-elem-id"
 *
 *     include: base.yaml
 *
 *     scp-id-cells:
 *        - mod-idx       # module index
 *        - elem-idx      # element index
 * 
 * These are then used in the baseline .dtsi include scp-fwk.dtsi
 *
 * e.g.:
 *
 *     scp-fwk {
 *
 *         elem_id: fwk-id@0 {
 *             compatible = "arm,scp-elem-id";
 *             #scp-id-cells = <2>;        
 *         };
 *     };
 *
 * These are then referenced in module objects where a SCP ID is required.
 *
 * e.g:
 *
 *     scp-fwk {
 *      
 *         scp-clk-dev@0 {
 *             compatible = "arm,scp-clk-dev";
 *             name = "I2SCLK";
 *             scp-ids = <&elem_id FWK_MODULE_IDX_JUNO_CDCEL937 JUNO_CLOCK_CDCEL937_IDX_I2SCLK>,
 *                       <&elem_id FWK_MODULE_IDX_JUNO_CDCEL937 MOD_JUNO_CDCEL937_API_IDX_CLOCK_DRIVER>;
 *          
 *         };
 *     };
 *
 * This mechanism allows the hybrid build containing both devicetree configured and
 * earlier directly written standard C configuration
 *
 * @ingroup devicetree-scp-fwk
 * @{
 */

/**
 * @brief Generate SCP element ID from phandle sds_ids reference.
 * @param compat Compatible name
 * @param inst Instance number
 * @param array_idx
 *
 *  Use the Phandle references to the SCP ID compatibles to create Element ID types.
 */
#define FWK_DT_PH_IDX_SCP_ID_ELEM(compat, inst, array_idx) FWK_ID_ELEMENT_INIT( \
        DT_PHA_BY_IDX(DT_INST(inst, compat), scp_ids, array_idx, mod_idx), \
        DT_PHA_BY_IDX(DT_INST(inst, compat), scp_ids, array_idx, elem_idx))

/**
 * @brief Shortcut to get the first SCP element ID in an array
 * @param compat Compatible name
 * @param inst Instance number
 */
#define FWK_DT_PH_SCP_ID_ELEM(compat, inst) FWK_DT_PH_IDX_SCP_ID_ELEM(compat, inst, 0)

/**
 * @brief Generate SCP API ID from phandle sds_ids reference.
 * @param compat Compatible name
 * @param inst Instance number
 * @param array_idx
 *
 * Use the Phandle references to the SCP ID compatibles to create API ID types.
 */
#define FWK_DT_PH_IDX_SCP_ID_API(compat, inst, array_idx) FWK_ID_API_INIT( \
        DT_PHA_BY_IDX(DT_INST(inst, compat), scp_ids, array_idx, mod_idx), \
        DT_PHA_BY_IDX(DT_INST(inst, compat), scp_ids, array_idx, elem_idx))

/**
 * @brief Shortcut to get the first SCP API ID in an array
 * @param compat Compatible name
 * @param inst Instance number
 */
#define FWK_DT_PH_SCP_ID_API(compat, inst) FWK_DT_PH_IDX_SCP_ID_API(compat, inst, 0)

/**
 * @brief Generate SCP Module ID from phandle sds_ids reference.
 * @param compat Compatible name
 * @param inst Instance number
 * 
 * Use the Phandle references to the SCP ID compatibles to create Module ID types
 */
#define FWK_DT_PH_IDX_SCP_ID_MOD(compat, inst, array_idx) FWK_ID_MODULE_INIT( \
        DT_PHA_BY_IDX(DT_INST(inst, compat), scp_ids, array_idx, mod_idx))

/**
 * @brief Generate SCP Notification ID from phandle sds_ids reference.
 * @param compat Compatible name
 * @param inst Instance number
 * @param array_idx
 *
 * Use the Phandle references to the SCP ID compatibles to create Notification ID types.
 */
#define FWK_DT_PH_IDX_SCP_ID_NTFY(compat, inst, array_idx) FWK_ID_NOTIFICATION_INIT( \
        DT_PHA_BY_IDX(DT_INST(inst, compat), scp_ids, array_idx, mod_idx), \
        DT_PHA_BY_IDX(DT_INST(inst, compat), scp_ids, array_idx, elem_idx))        

/**
 * @brief Create an Element ID if a phandle present at array index
 * @param compat Compatible name
 * @param inst Instance number
 * @param array_idx
 *
 * use the phandle elements to create an SCP Framework Element ID if index is present,
 * set to FWK_ID_NONE_INIT if not present.
 */
#define FWK_DT_PH_OPT_IDX_SCP_ID_ELEM(compat, inst, array_idx) \
    COND_CODE_1(DT_PROP_HAS_IDX(DT_INST(inst, compat), scp_ids, array_idx),    \
        (FWK_DT_PH_IDX_SCP_ID_ELEM(compat, inst, array_idx)), \
        (FWK_ID_NONE_INIT) \
    )

/**
 * @brief Create an Element or a Module ID
 * @param compat Compatible name
 * @param inst Instance number
 * @param array_idx
 *
 * Create a element ID if the phandle refers to an object with an elem_idx cell,
 * otherwise treat as module ID
 */
#define FWK_DT_PH_IDX_SCP_ID_MOD_OR_ELEM(compat, inst, array_idx) \
    COND_CODE_1(DT_PHA_HAS_CELL_AT_IDX(DT_INST(inst, compat), scp_ids, array_idx, elem_idx), \
        (FWK_DT_PH_IDX_SCP_ID_ELEM(compat, inst, array_idx)), \
        (FWK_DT_PH_IDX_SCP_ID_MOD(compat, inst, array_idx)) \
    )

/**
 * @}
 */

/**
 * @brief Create a framework element id from array
 * @param compat Compatible name
 * @param inst Instance number
 * @param prop Array property 
 *
 * Intialise a framework element id from a device tree 2 element property array - without a phandle reference.
 * Only use if the phandle reference cannot be used. e.g. extra ID required as a build time optional
 * addition.
 */
#define FWK_DT_FWK_ELEM_ID(compat, inst, prop) \
    FWK_ID_ELEMENT_INIT( \
        FWK_DT_INST_IDX_PROP(compat, inst, prop, 0), \
        FWK_DT_INST_IDX_PROP(compat, inst, prop, 1) \
        )
/** 
 * @brief Optionally Create a framework element id from array
 * @param compat Compatible name
 * @param inst Instance number
 * @param prop Array property 
 *
 * Intialise a framework id from an optional device tree 2 element property array - without a phandle reference.
 * Uses a default FWK_ID_NONE if the property is not present on the node instance.
 * Only use if the phandle reference cannot be used. e.g. extra ID required as a build time optional
 * addition
 */
#define FWK_DT_OPT_FWK_ELEM_ID(compat, inst, prop) \
    COND_CODE_1(DT_NODE_HAS_PROP(DT_INST(inst ,compat), prop), \
        (FWK_DT_FWK_ELEM_ID(compat, inst, prop)), \
        (FWK_ID_NONE_INIT) \
    )

/**
 * @brief Check that a table is of expected size
 * @param table_array Table array source element
 * @param count Expected number of elements in the table
 *
 * check that the table array matches the count of elements from DT
 */
#define FWK_DT_TABLE_CHECK(table_array, element_count) \
    ((sizeof(table_array) / sizeof(table_array[0])) == (element_count))

/**
 * @brief Compile time check for condition
 * @param condition The condition to validate
 *
 * Force a compile time error if 'condition' is false.
 * results in no generated code if 'condition' is true.
 *
 * Use to validate some generated elements that cannot be checked by C
 * pre-processor.
 *
 * example usage:
 *
 *     FWT_DT_BUILD_BUG_ON(!FWK_DT_TABLE_CHECK(my_table, has_4_elements))
 */
#define FWK_DT_BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

/**
 * @}
 */
