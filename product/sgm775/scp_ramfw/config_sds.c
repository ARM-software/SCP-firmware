/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_devices.h"
#include "sgm775_mmap.h"
#include "sgm775_sds.h"
#include "software_mmap.h"

#include <mod_sds.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

static const uint32_t feature_flags = 0x00000000;
static const uint32_t version_packed = FWK_BUILD_VERSION;

static const struct mod_sds_region_desc sds_module_regions[] = {
    [SGM775_SDS_REGION_SECURE] = {
        .base = (void*)SDS_SECURE_BASE,
        .size = SDS_SECURE_SIZE,
    },
#ifdef BUILD_MODE_DEBUG
    [SGM775_SDS_REGION_NONSECURE] = {
        .base = (void *)SDS_NONSECURE_BASE,
        .size = SDS_NONSECURE_SIZE,
    },
#endif
};

static_assert(FWK_ARRAY_SIZE(sds_module_regions) == SGM775_SDS_REGION_COUNT,
              "Mismatch between number of SDS regions and number of regions "
              "provided by the SDS configuration.");

static const struct mod_sds_config sds_module_config = {
    .regions = sds_module_regions,
    .region_count = SGM775_SDS_REGION_COUNT,
    .clock_id = FWK_ID_NONE_INIT,
};

static const struct fwk_element sds_element_table[] = {
    {
        .name = "RAM Version",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_RAM_VERSION,
            .size = SGM775_SDS_RAM_VERSION_SIZE,
            .payload = &version_packed,
            .region_id = SGM775_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "Feature Availability",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_FEATURE_AVAILABILITY,
            .size = sizeof(feature_flags),
            .payload = &feature_flags,
            .region_id = SGM775_SDS_REGION_SECURE,
            .finalize = false,
        }),
    },
    { 0 }, /* Termination description. */
};

static_assert(SDS_SECURE_SIZE >
                    SGM775_SDS_RAM_VERSION_SIZE +
                    sizeof(feature_flags),
            "SDS structures too large for SDS S-RAM.\n");

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
