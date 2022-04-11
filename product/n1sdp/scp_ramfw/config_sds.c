/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "n1sdp_pik_scp.h"
#include "n1sdp_scp_pik.h"
#include "n1sdp_scp_software_mmap.h"
#include "n1sdp_sds.h"

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
    [N1SDP_SDS_REGION_SECURE] = {
        .base = (void*)SCP_SDS_SECURE_BASE,
        .size = SCP_SDS_SECURE_SIZE,
    },
#ifdef BUILD_MODE_DEBUG
    [N1SDP_SDS_REGION_NONSECURE] = {
        .base = (void *)SCP_SDS_NONSECURE_BASE,
        .size = SCP_SDS_NONSECURE_SIZE,
    },
#endif
};

static_assert(FWK_ARRAY_SIZE(sds_module_regions) == N1SDP_SDS_REGION_COUNT,
              "Mismatch between number of SDS regions and number of regions "
              "provided by the SDS configuration.");

static const struct mod_sds_config sds_module_config = {
    .regions = sds_module_regions,
    .region_count = N1SDP_SDS_REGION_COUNT,
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
                                    CLOCK_IDX_INTERCONNECT)
};

static struct fwk_element sds_element_table[] = {
    {
        .name = "CPU Info",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_CPU_INFO,
            .size = N1SDP_SDS_CPU_INFO_SIZE,
            .region_id = N1SDP_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "Firmware version",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_FIRMWARE_VERSION,
            .size = N1SDP_SDS_FIRMWARE_VERSION_SIZE,
            .region_id = N1SDP_SDS_REGION_SECURE,
            .payload = &version_packed,
            .finalize = true,
        }),
    },
    {
        .name = "Reset Syndrome",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_RESET_SYNDROME,
            .size = N1SDP_SDS_RESET_SYNDROME_SIZE,
            .region_id = N1SDP_SDS_REGION_SECURE,
            .payload = (void *)(&PIK_SCP->RESET_SYNDROME),
            .finalize = true,
        }),
    },
    {
        .name = "Feature Availability",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_FEATURE_AVAILABILITY,
            .size = N1SDP_SDS_FEATURE_AVAILABILITY_SIZE,
            .region_id = N1SDP_SDS_REGION_SECURE,
            .payload = &feature_flags,
            .finalize = true,
        }),
    },
    {
        .name = "Platform Info",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_PLATFORM_INFO,
            .size = N1SDP_SDS_PLATFORM_INFO_SIZE,
            .region_id = N1SDP_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
#ifdef BUILD_MODE_DEBUG
    {
        .name = "Boot Counters",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_CPU_BOOTCTR,
            .size = N1SDP_SDS_CPU_BOOTCTR_SIZE,
            .region_id = N1SDP_SDS_REGION_NONSECURE,
            .finalize = true,
        }),
    },
    {
        .name = "CPU Flags",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_CPU_FLAGS,
            .size = N1SDP_SDS_CPU_FLAGS_SIZE,
            .region_id = N1SDP_SDS_REGION_NONSECURE,
            .finalize = true,
        }),
    },
#endif
    { 0 }, /* Termination description. */
};

static_assert(SCP_SDS_SECURE_SIZE >
                    N1SDP_SDS_CPU_INFO_SIZE +
                    N1SDP_SDS_FIRMWARE_VERSION_SIZE +
                    N1SDP_SDS_RESET_SYNDROME_SIZE +
                    N1SDP_SDS_FEATURE_AVAILABILITY_SIZE,
            "SDS structures too large for SDS S-RAM.\n");

#ifdef BUILD_MODE_DEBUG
    static_assert(SCP_SDS_NONSECURE_SIZE >
                        N1SDP_SDS_CPU_BOOTCTR_SIZE +
                        N1SDP_SDS_CPU_FLAGS_SIZE,
                "SDS structures too large for SDS NS-RAM.\n");
#endif

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
