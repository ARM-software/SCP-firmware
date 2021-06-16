/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_sds.h"
#include "software_mmap.h"

#include <mod_sds.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stdint.h>

static const uint32_t feature_flags = 0x00000000;
static const uint32_t version_packed = FWK_BUILD_VERSION;

static const struct mod_sds_region_desc sds_module_regions[] = {
    [JUNO_SDS_REGION_SECURE] = {
        .base = (void*)SCP_SDS_SECURE_BASE,
        .size = SCP_SDS_SECURE_SIZE,
    },
};

static_assert(FWK_ARRAY_SIZE(sds_module_regions) == JUNO_SDS_REGION_COUNT,
              "Mismatch between number of SDS regions and number of regions "
              "provided by the SDS configuration.");

static const struct mod_sds_config sds_module_config = {
    .regions = sds_module_regions,
    .region_count = (unsigned int)JUNO_SDS_REGION_COUNT,
    .clock_id = FWK_ID_NONE_INIT,
};

static const struct fwk_element element_table[] = {
    [JUNO_SDS_RAM_VERSION_IDX] = {
        .name = "",
        .data = &(struct mod_sds_structure_desc) {
            .id = (uint32_t) JUNO_SDS_RAM_VERSION,
            .size = JUNO_SDS_RAM_VERSION_SIZE,
            .region_id = (uint32_t) JUNO_SDS_REGION_SECURE,
            .payload = &version_packed,
            .finalize = true,
        },
    },
    [JUNO_SDS_RAM_FEATURES_IDX] = {
        .name = "",
        .data = &(struct mod_sds_structure_desc) {
            .id = (uint32_t) JUNO_SDS_FEATURE_AVAILABILITY,
            .size = sizeof(feature_flags),
            .region_id = (uint32_t) JUNO_SDS_REGION_SECURE,
            .payload = &feature_flags,
            .finalize = true,
        },
    },
    [JUNO_SDS_RAM_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    static_assert(BUILD_VERSION_MAJOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_MINOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_PATCH < UINT16_MAX, "Invalid version size");

    return element_table;
}

struct fwk_module_config config_sds = {
    .data = &sds_module_config,

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};
