/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "scp_sgi575_mmap.h"
#include "scp_sgi575_pik.h"
#include "scp_software_mmap.h"
#include "sgi575_pik_scp.h"
#include "sgi575_sds.h"
#include "sgi575_ssc.h"

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
    [SGI575_SDS_REGION_SECURE] = {
        .base = (void*)SCP_SDS_SECURE_BASE,
        .size = SCP_SDS_SECURE_SIZE,
    },
#ifdef BUILD_MODE_DEBUG
    [SGI575_SDS_REGION_NONSECURE] = {
        .base = (void *)SCP_SDS_NONSECURE_BASE,
        .size = SCP_SDS_NONSECURE_SIZE,
    },
#endif
};

static_assert(FWK_ARRAY_SIZE(sds_module_regions) == SGI575_SDS_REGION_COUNT,
              "Mismatch between number of SDS regions and number of regions "
              "provided by the SDS configuration.");

static const struct mod_sds_config sds_module_config = {
    .regions = sds_module_regions,
    .region_count = SGI575_SDS_REGION_COUNT,
    .clock_id = FWK_ID_NONE_INIT,
};

static struct sgi575_sds_platid platid;

static struct fwk_element sds_element_table[] = {
    {
        .name = "CPU Info",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_CPU_INFO,
            .size = SGI575_SDS_CPU_INFO_SIZE,
            .region_id = SGI575_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "Firmware version",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_FIRMWARE_VERSION,
            .size = SGI575_SDS_FIRMWARE_VERSION_SIZE,
            .region_id = SGI575_SDS_REGION_SECURE,
            .payload = &version_packed,
            .finalize = true,
        }),
    },
    {
        .name = "Platform ID",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_PLATFORM_ID,
            .size = SGI575_SDS_PLATFORM_ID_SIZE,
            .region_id = SGI575_SDS_REGION_SECURE,
            .payload = &platid,
            .finalize = true,
        }),
    },
    {
        .name = "Reset Syndrome",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_RESET_SYNDROME,
            .size = SGI575_SDS_RESET_SYNDROME_SIZE,
            .region_id = SGI575_SDS_REGION_SECURE,
            .payload = (void *)(&PIK_SCP->RESET_SYNDROME),
            .finalize = true,
        }),
    },
    {
        .name = "Feature Availability",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_FEATURE_AVAILABILITY,
            .size = SGI575_SDS_FEATURE_AVAILABILITY_SIZE,
            .region_id = SGI575_SDS_REGION_SECURE,
            .payload = &feature_flags,
            .finalize = true,
        }),
    },
#ifdef BUILD_MODE_DEBUG
    {
        .name = "Boot Counters",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_CPU_BOOTCTR,
            .size = SGI575_SDS_CPU_BOOTCTR_SIZE,
            .region_id = SGI575_SDS_REGION_NONSECURE,
            .finalize = true,
        }),
    },
    {
        .name = "CPU Flags",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_CPU_FLAGS,
            .size = SGI575_SDS_CPU_FLAGS_SIZE,
            .region_id = SGI575_SDS_REGION_NONSECURE,
            .finalize = true,
        }),
    },
#endif
    { 0 }, /* Termination description. */
};

static_assert(SCP_SDS_SECURE_SIZE >
                    SGI575_SDS_CPU_INFO_SIZE +
                    SGI575_SDS_FIRMWARE_VERSION_SIZE +
                    SGI575_SDS_PLATFORM_ID_SIZE +
                    SGI575_SDS_RESET_SYNDROME_SIZE +
                    SGI575_SDS_FEATURE_AVAILABILITY_SIZE,
            "SDS structures too large for SDS S-RAM.\n");

#ifdef BUILD_MODE_DEBUG
    static_assert(SCP_SDS_NONSECURE_SIZE >
                        SGI575_SDS_CPU_BOOTCTR_SIZE +
                        SGI575_SDS_CPU_FLAGS_SIZE,
                "SDS structures too large for SDS NS-RAM.\n");
#endif

static const struct fwk_element *sds_get_element_table(fwk_id_t module_id)
{
    struct ssc_reg *ssc_regs = ((struct ssc_reg *)(SSC_BASE));

    static_assert(BUILD_VERSION_MAJOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_MINOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_PATCH < UINT16_MAX, "Invalid version size");

    platid.platform_identifier = ssc_regs->SSC_VERSION;
    platid.platform_type_identifier = platid.platform_identifier &
                                      SGI575_SDS_PLATID_PARTNO_MASK;

    return sds_element_table;
}

struct fwk_module_config config_sds = {
    .data = &sds_module_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(sds_get_element_table),
};
