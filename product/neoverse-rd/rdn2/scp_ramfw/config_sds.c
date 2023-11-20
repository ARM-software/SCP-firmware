/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "platform_sds.h"
#include "scp_pik.h"
#include "scp_software_mmap.h"

#include <mod_sds.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

static const uint32_t version_packed = FWK_BUILD_VERSION;
static const uint32_t feature_flags = 0x00000000;

static const struct mod_sds_region_desc sds_module_regions[] = {
    [PLATFORM_SDS_REGION_SECURE] = {
        .base = (void*)SCP_SDS_SECURE_BASE,
        .size = SCP_SDS_SECURE_SIZE,
    },
};

static_assert(
    FWK_ARRAY_SIZE(sds_module_regions) == PLATFORM_SDS_REGION_COUNT,
    "Mismatch between number of SDS regions and number of regions "
    "provided by the SDS configuration.");

const struct mod_sds_config sds_module_config = {
    .regions = sds_module_regions,
    .region_count = PLATFORM_SDS_REGION_COUNT,
    .clock_id = FWK_ID_NONE_INIT,
};

static struct fwk_element sds_element_table[] = {
    [PLATFORM_SDS_CPU_INFO_IDX] = {
        .name = "CPU Info",
        .data = &((struct mod_sds_structure_desc){
            .id = PLATFORM_SDS_CPU_INFO,
            .size = PLATFORM_SDS_CPU_INFO_SIZE,
            .region_id = PLATFORM_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    [PLATFORM_SDS_ROM_VERSION_IDX] = {
        .name = "ROM firmware version",
        .data = &((struct mod_sds_structure_desc){
            .id = PLATFORM_SDS_ROM_VERSION,
            .size = PLATFORM_SDS_ROM_VERSION_SIZE,
            .payload = &version_packed,
            .region_id = PLATFORM_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    [PLATFORM_SDS_RAM_VERSION_IDX] = {
        .name = "RAM firmware version",
        .data = &((struct mod_sds_structure_desc){
            .id = PLATFORM_SDS_RAM_VERSION,
            .size = PLATFORM_SDS_RAM_VERSION_SIZE,
            .payload = &version_packed,
            .region_id = PLATFORM_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    [PLATFORM_SDS_RESET_SYNDROME_IDX] = {
        .name = "Reset Syndrome",
        .data = &((struct mod_sds_structure_desc){
            .id = PLATFORM_SDS_RESET_SYNDROME,
            .size = PLATFORM_SDS_RESET_SYNDROME_SIZE,
            .region_id = PLATFORM_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    [PLATFORM_SDS_FEATURE_AVAILABILITY_IDX] = {
        .name = "Feature Availability",
        .data = &((struct mod_sds_structure_desc){
            .id = PLATFORM_SDS_FEATURE_AVAILABILITY,
            .size = PLATFORM_SDS_FEATURE_AVAILABILITY_SIZE,
            .payload = &feature_flags,
            .region_id = PLATFORM_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    [PLATFORM_SDS_ISOLATED_CPU_MPID_LIST_IDX] = {
        .name = "Isolated CPU MPID List",
        .data = &((struct mod_sds_structure_desc){
                .id = PLATFORM_SDS_ISOLATED_CPU_MPID_LIST,
                .size = PLATFORM_SDS_ISOLATED_CPU_MPID_LIST_SIZE,
                .region_id = PLATFORM_SDS_REGION_SECURE,
                .finalize = true,
                }),
    },
    [PLATFORM_SDS_IDX_COUNT] = { 0 }, /* Termination description. */
};

static_assert(
    SCP_SDS_SECURE_SIZE > PLATFORM_SDS_CPU_INFO_SIZE +
            PLATFORM_SDS_ROM_VERSION_SIZE + PLATFORM_SDS_RAM_VERSION_SIZE +
            PLATFORM_SDS_RESET_SYNDROME_SIZE +
            PLATFORM_SDS_FEATURE_AVAILABILITY_SIZE,
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
