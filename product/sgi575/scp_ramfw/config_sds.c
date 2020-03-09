/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
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

const struct mod_sds_config sds_module_config = {
    .region_base_address = SCP_SDS_MEM_BASE,
    .region_size = SCP_SDS_MEM_SIZE,
    .clock_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK,
        CLOCK_IDX_INTERCONNECT)
};

static struct sgi575_sds_platid platid;

static struct fwk_element sds_element_table[] = {
    {
        .name = "CPU Info",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_CPU_INFO,
            .size = SGI575_SDS_CPU_INFO_SIZE,
            .finalize = true,
        }),
    },
    {
        .name = "Firmware version",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_FIRMWARE_VERSION,
            .size = SGI575_SDS_FIRMWARE_VERSION_SIZE,
            .payload = &version_packed,
            .finalize = true,
        }),
    },
    {
        .name = "Platform ID",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_PLATFORM_ID,
            .size = SGI575_SDS_PLATFORM_ID_SIZE,
            .payload = &platid,
            .finalize = true,
        }),
    },
    {
        .name = "Reset Syndrome",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_RESET_SYNDROME,
            .size = SGI575_SDS_RESET_SYNDROME_SIZE,
            .payload = (void *)(&PIK_SCP->RESET_SYNDROME),
            .finalize = true,
        }),
    },
    {
        .name = "Feature Availability",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_FEATURE_AVAILABILITY,
            .size = SGI575_SDS_FEATURE_AVAILABILITY_SIZE,
            .payload = &feature_flags,
            .finalize = true,
        }),
    },
#ifdef BUILD_HAS_MOD_TEST
    {
        .name = "Boot Counters",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_CPU_BOOTCTR,
            .size = SGI575_SDS_CPU_BOOTCTR_SIZE,
            .finalize = true,
        }),
    },
    {
        .name = "CPU Flags",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGI575_SDS_CPU_FLAGS,
            .size = SGI575_SDS_CPU_FLAGS_SIZE,
            .finalize = true,
        }),
    },
#endif
    { 0 }, /* Termination description. */
};

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
    .get_element_table = sds_get_element_table,
    .data = &sds_module_config,
};
