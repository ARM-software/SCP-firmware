/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_sds.h"
#include "software_mmap.h"

#include <mod_sds.h>

#include <fwk_assert.h>
#include <fwk_dt_config_common.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stdint.h>

static const uint32_t feature_flags = 0x00000000;
static const uint32_t version_packed = FWK_BUILD_VERSION;

#ifdef FWK_MODULE_GEN_DYNAMIC_SDS
const struct fwk_element *dyn_get_element_table_sds(fwk_id_t module_id)
{
    const struct fwk_element *sds_elem_table = _static_get_element_table_sds();
    struct mod_sds_structure_desc *sds_struct_desc;

    static_assert(BUILD_VERSION_MAJOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_MINOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_PATCH < UINT16_MAX, "Invalid version size");
    
    /* ensure that the dts file contains enough elements */
    static_assert(DT_N_INST_arm_scp_sds_NUM_OKAY == JUNO_SDS_RAM_IDX_COUNT,
                "Too few configured sds elements in dts file");
    static_assert(DT_N_INST_arm_scp_sds_region_NUM_OKAY == JUNO_SDS_REGION_COUNT,
                "Mismatch between number of SDS regions and number of regions "
                "provided by the SDS configuration.");

    /* override payload elements needed for Juno */
    sds_struct_desc = (struct mod_sds_structure_desc *)sds_elem_table[JUNO_SDS_RAM_VERSION_IDX].data;
    sds_struct_desc->payload = &version_packed;
    sds_struct_desc = (struct mod_sds_structure_desc *)sds_elem_table[JUNO_SDS_RAM_FEATURES_IDX].data;
    sds_struct_desc->payload = &feature_flags;
    sds_struct_desc->size = sizeof(feature_flags);

    return sds_elem_table;
}
#else
#error("Error: Juno scp_ramfw must use dynamic dts module access to override values");
#endif
