/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_sds.h>
#include <rcar_mmap.h>
#include <rcar_sds.h>
#include <clock_devices.h>

static const uint32_t feature_flags = RCAR_SDS_FEATURE_FIRMWARE_INIT;
static const uint32_t version_packed = FWK_BUILD_VERSION;

static const struct mod_sds_region_desc sds_module_regions[] = {
    [RCAR_SDS_REGION_SECURE] = {
        .base = (void*)(TRUSTED_RAM_BASE + RCAR_SDS_SCP_RAM_BASE_OFFSET),
        .size = RCAR_SDS_SCP_RAM_SIZE,
    },
};

static_assert(FWK_ARRAY_SIZE(sds_module_regions) == RCAR_SDS_REGION_COUNT,
              "Mismatch between number of SDS regions and number of regions "
              "provided by the SDS configuration.");

const struct mod_sds_config sds_module_config = {
    .regions = sds_module_regions,
    .region_count = RCAR_SDS_REGION_COUNT,
    .clock_id = FWK_ID_NONE,
};

static const struct fwk_element sds_element_table[] = {
    {
        .name = "RAM Version",
        .data = &((struct mod_sds_structure_desc) {
            .id = RCAR_SDS_RAM_VERSION,
            .size = RCAR_SDS_RAM_VERSION_SIZE,
            .payload = &version_packed,
            .finalize = true,
        }),
    },
    {
        .name = "Feature Availability",
        .data = &((struct mod_sds_structure_desc) {
            .id = RCAR_SDS_FEATURE_AVAILABILITY,
            .size = sizeof(feature_flags),
            .payload = &feature_flags,
            .finalize = false,
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *sds_get_element_table(fwk_id_t module_id)
{
    static_assert(BUILD_VERSION_MAJOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_MINOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_PATCH < UINT16_MAX, "Invalid version size");

    return sds_element_table;
}

struct fwk_module_config config_sds = {
    .data = &sds_module_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(sds_get_element_table),
};
