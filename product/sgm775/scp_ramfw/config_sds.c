/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_devices.h"
#include "sgm775_mmap.h"
#include "sgm775_sds.h"

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

const struct mod_sds_config sds_module_config = {
    .region_base_address = TRUSTED_RAM_BASE,
    .region_size = 3520,
    .clock_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_CLOCK,
                    CLOCK_DEV_IDX_FCMCLK),
};

static const struct fwk_element sds_element_table[] = {
    {
        .name = "RAM Version",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_RAM_VERSION,
            .size = SGM775_SDS_RAM_VERSION_SIZE,
            .payload = &version_packed,
            .finalize = true,
        }),
    },
    {
        .name = "Feature Availability",
        .data = &((struct mod_sds_structure_desc) {
            .id = SGM775_SDS_FEATURE_AVAILABILITY,
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
    .get_element_table = sds_get_element_table,
    .data = &sds_module_config,
};
