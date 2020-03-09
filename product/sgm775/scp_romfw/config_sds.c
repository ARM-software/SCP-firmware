/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_devices.h"
#include "sgm775_mmap.h"
#include "sgm775_pik.h"
#include "sgm775_pik_scp.h"
#include "sgm775_sds.h"
#include "sgm775_ssc.h"
#include "system_mmap.h"

#include <mod_sds.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

static const struct mod_sds_config sds_module_config = {
    .region_base_address = TRUSTED_RAM_BASE,
    .region_size = 3520,
    .clock_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CLOCK,
                    CLOCK_DEV_IDX_SYS_FCMCLK),
};
static const uint32_t version_packed = FWK_BUILD_VERSION;
static struct sgm775_sds_platid platid;

static const struct fwk_element sds_element_table[] = {
    {
        .name = "CPU Info",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_CPU_INFO,
            .size = SGM775_SDS_CPU_INFO_SIZE,
            .finalize = true,
        }),
    },
    {
        .name = "ROM Version",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_ROM_VERSION,
            .size = SGM775_SDS_ROM_VERSION_SIZE,
            .payload = &version_packed,
            .finalize = true,
        }),
    },
    {
        .name = "Platform ID",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_PLATFORM_ID,
            .size = SGM775_SDS_PLATFORM_ID_SIZE,
            .payload = &platid,
            .finalize = true,
        }),
    },
    {
        .name = "Reset Syndrome",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_RESET_SYNDROME,
            .size = SGM775_SDS_RESET_SYNDROME_SIZE,
            .payload = (void *)(&PIK_SCP->RESET_SYNDROME),
            .finalize = true,
        }),
    },
    {
        .name = "Bootloader",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_BOOTLOADER,
            .size = SGM775_SDS_BOOTLOADER_SIZE,
            .finalize = true,
        }),
    },
    {
        .name = "Features",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_FEATURE_AVAILABILITY,
            .size = SGM775_SDS_FEATURE_AVAILABILITY_SIZE,
            .finalize = true,
        }),
    },
#ifdef BUILD_HAS_MOD_TEST
    {
        .name = "Boot Counters",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_CPU_BOOTCTR,
            .size = SGM775_SDS_CPU_BOOTCTR_SIZE,
            .finalize = true,
        }),
    },
    {
        .name = "CPU Flags",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_CPU_FLAGS,
            .size = SGM775_SDS_CPU_FLAGS_SIZE,
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
    platid.platform_type_identifier = *((uint32_t*)PLATFORM_ID);

    return sds_element_table;
}

struct fwk_module_config config_sds = {
    .get_element_table = sds_get_element_table,
    .data = &sds_module_config,
};
