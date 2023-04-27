/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_soc.h"
#include "scp_mmap.h"
#include "scp_pik.h"
#include "tc_sds.h"

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

static const struct mod_sds_region_desc sds_module_regions[1] = {
    [TC_SDS_REGION_SECURE] = {
            .base = (void *)SCP_SDS_MEM_BASE,
            .size = SCP_SDS_MEM_SIZE,
    },
};

static_assert(
    FWK_ARRAY_SIZE(sds_module_regions) == TC_SDS_REGION_COUNT,
    "Mismatch between number of SDS regions and number of regions "
    "provided by the SDS configuration.");

const struct mod_sds_config sds_module_config = {
    .regions = sds_module_regions,
    .region_count = TC_SDS_REGION_COUNT,
    .clock_id = FWK_ID_ELEMENT_INIT(
        FWK_MODULE_IDX_CLOCK,
        CLOCK_IDX_CPU_GROUP_CORTEX_A510)
};

static struct fwk_element sds_element_table[4] = {
    {
        .name = "CPU Info",
        .data = &((struct mod_sds_structure_desc){
            .id = TC_SDS_CPU_INFO,
            .size = TC_SDS_CPU_INFO_SIZE,
            .region_id = TC_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "Feature Availability",
        .data = &((struct mod_sds_structure_desc){
            .id = TC_SDS_FEATURE_AVAILABILITY,
            .size = TC_SDS_FEATURE_AVAILABILITY_SIZE,
            .payload = &feature_flags,
            .region_id = TC_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "Bootloader",
        .data = &((struct mod_sds_structure_desc){
            .id = TC_SDS_BOOTLOADER,
            .size = TC_SDS_BOOTLOADER_SIZE,
            .region_id = TC_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    { 0 }, /* Termination description. */
};

static_assert(
    SCP_SDS_MEM_SIZE > TC_SDS_CPU_INFO_SIZE + TC_SDS_FEATURE_AVAILABILITY_SIZE +
            TC_SDS_BOOTLOADER_SIZE,
    "SDS structures too large for SDS SRAM.\n");

static const struct fwk_element *sds_get_element_table(fwk_id_t module_id)
{
    static_assert(BUILD_VERSION_MAJOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_MINOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_PATCH < UINT16_MAX, "Invalid version size");

    return sds_element_table;
}

struct fwk_module_config config_sds = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(sds_get_element_table),
    .data = &sds_module_config,
};
