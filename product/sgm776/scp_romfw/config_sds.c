/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_devices.h"
#include "sgm776_mmap.h"
#include "sgm776_pik.h"
#include "sgm776_pik_scp.h"
#include "sgm776_sds.h"
#include "system_mmap.h"
#include "software_mmap.h"

#include <mod_sds.h>
#include <mod_sid.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

static const uint32_t version_packed = FWK_BUILD_VERSION;
static struct sgm776_sds_platid platid;

static const struct mod_sds_region_desc sds_module_regions[] = {
    [SGM776_SDS_REGION_SECURE] = {
        .base = (void*)SDS_SECURE_BASE,
        .size = SDS_SECURE_SIZE,
    },
#ifdef BUILD_MODE_DEBUG
    [SGM776_SDS_REGION_NONSECURE] = {
        .base = (void *)SDS_NONSECURE_BASE,
        .size = SDS_NONSECURE_SIZE,
    },
#endif
};

static_assert(FWK_ARRAY_SIZE(sds_module_regions) == SGM776_SDS_REGION_COUNT,
              "Mismatch between number of SDS regions and number of regions "
              "provided by the SDS configuration.");

static const struct mod_sds_config sds_module_config = {
    .regions = sds_module_regions,
    .region_count = SGM776_SDS_REGION_COUNT,
    .clock_id = FWK_ID_NONE_INIT,
};

static const struct fwk_element sds_element_table[] = {
    {
        .name = "CPU Info",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM776_SDS_CPU_INFO,
            .size = SGM776_SDS_CPU_INFO_SIZE,
            .region_id = SGM776_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "ROM Version",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM776_SDS_ROM_VERSION,
            .size = SGM776_SDS_ROM_VERSION_SIZE,
            .payload = &version_packed,
            .region_id = SGM776_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "Platform ID",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM776_SDS_PLATFORM_ID,
            .size = SGM776_SDS_PLATFORM_ID_SIZE,
            .payload = &platid,
            .region_id = SGM776_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "Reset Syndrome",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM776_SDS_RESET_SYNDROME,
            .size = SGM776_SDS_RESET_SYNDROME_SIZE,
            .payload = (const void *)&PIK_SCP->RESET_SYNDROME,
            .region_id = SGM776_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "Bootloader",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM776_SDS_BOOTLOADER,
            .size = SGM776_SDS_BOOTLOADER_SIZE,
            .region_id = SGM776_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
    {
        .name = "Features",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM776_SDS_FEATURE_AVAILABILITY,
            .size = SGM776_SDS_FEATURE_AVAILABILITY_SIZE,
            .region_id = SGM776_SDS_REGION_SECURE,
            .finalize = true,
        }),
    },
#ifdef BUILD_MODE_DEBUG
    {
        .name = "Boot Counters",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM776_SDS_CPU_BOOTCTR,
            .size = SGM776_SDS_CPU_BOOTCTR_SIZE,
            .region_id = SGM776_SDS_REGION_NONSECURE,
            .finalize = true,
        }),
    },
    {
        .name = "CPU Flags",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM776_SDS_CPU_FLAGS,
            .size = SGM776_SDS_CPU_FLAGS_SIZE,
            .region_id = SGM776_SDS_REGION_NONSECURE,
            .finalize = true,
        }),
    },
#endif
    { 0 }, /* Termination description. */
};

static_assert(SDS_SECURE_SIZE >
                    SGM776_SDS_CPU_INFO_SIZE +
                    SGM776_SDS_ROM_VERSION_SIZE +
                    SGM776_SDS_PLATFORM_ID_SIZE +
                    SGM776_SDS_RESET_SYNDROME_SIZE +
                    SGM776_SDS_BOOTLOADER_SIZE +
                    SGM776_SDS_CPU_BOOTCTR_SIZE +
                    SGM776_SDS_CPU_FLAGS_SIZE +
                    SGM776_SDS_FEATURE_AVAILABILITY_SIZE,
            "SDS structures too large for SDS S-RAM.\n");

#ifdef BUILD_MODE_DEBUG
    static_assert(SDS_NONSECURE_SIZE >
                        SGM776_SDS_CPU_BOOTCTR_SIZE +
                        SGM776_SDS_CPU_FLAGS_SIZE,
                "SDS structures too large for SDS NS-RAM.\n");
#endif

static const struct fwk_element *sds_get_element_table(fwk_id_t module_id)
{
    int status;
    const struct mod_sid_info *system_info;

    static_assert(BUILD_VERSION_MAJOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_MINOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_PATCH < UINT16_MAX, "Invalid version size");

    status = mod_sid_get_system_info(&system_info);
    fwk_assert(status == FWK_SUCCESS);
    platid.platform_identifier = system_info->system_part_number;
    platid.platform_type_identifier = *((uint32_t*)PLATFORM_ID);

    return sds_element_table;
}

struct fwk_module_config config_sds = {
    .data = &sds_module_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(sds_get_element_table),
};
