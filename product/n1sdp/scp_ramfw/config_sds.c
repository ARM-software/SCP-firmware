/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
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

const struct mod_sds_config sds_module_config = {
    .region_base_address = SCP_SDS_MEM_BASE,
    .region_size = SCP_SDS_MEM_SIZE,
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        CLOCK_IDX_INTERCONNECT)
};

static struct fwk_element sds_element_table[] = {
    {
        .name = "CPU Info",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_CPU_INFO,
            .size = N1SDP_SDS_CPU_INFO_SIZE,
            .finalize = true,
        }),
    },
    {
        .name = "Firmware version",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_FIRMWARE_VERSION,
            .size = N1SDP_SDS_FIRMWARE_VERSION_SIZE,
            .payload = &version_packed,
            .finalize = true,
        }),
    },
    {
        .name = "Reset Syndrome",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_RESET_SYNDROME,
            .size = N1SDP_SDS_RESET_SYNDROME_SIZE,
            .payload = (void *)(&PIK_SCP->RESET_SYNDROME),
            .finalize = true,
        }),
    },
    {
        .name = "Feature Availability",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_FEATURE_AVAILABILITY,
            .size = N1SDP_SDS_FEATURE_AVAILABILITY_SIZE,
            .payload = &feature_flags,
            .finalize = true,
        }),
    },
    {
        .name = "Platform Info",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_PLATFORM_INFO,
            .size = N1SDP_SDS_PLATFORM_INFO_SIZE,
            .finalize = true,
        }),
    },
    {
        .name = "BL33 Image Info",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_BL33_INFO,
            .size = N1SDP_SDS_BL33_INFO_SIZE,
            .finalize = true,
        }),
    },
#ifdef BUILD_HAS_MOD_TEST
    {
        .name = "Boot Counters",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_CPU_BOOTCTR,
            .size = N1SDP_SDS_CPU_BOOTCTR_SIZE,
            .finalize = true,
        }),
    },
    {
        .name = "CPU Flags",
        .data = &((struct mod_sds_structure_desc) {
            .id = N1SDP_SDS_CPU_FLAGS,
            .size = N1SDP_SDS_CPU_FLAGS_SIZE,
            .finalize = true,
        }),
    },
#endif
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
    .get_element_table = sds_get_element_table,
    .data = &sds_module_config,
};
