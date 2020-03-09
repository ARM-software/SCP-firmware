/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_sds.h"
#include "software_mmap.h"

#include <mod_sds.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stdint.h>

static const uint32_t feature_flags = 0x00000000;
static const uint32_t version_packed = FWK_BUILD_VERSION;

static const struct fwk_element element_table[] = {
    [JUNO_SDS_RAM_VERSION_IDX] = {
        .name = "",
        .data = &(struct mod_sds_structure_desc) {
            .id = JUNO_SDS_RAM_VERSION,
            .size = JUNO_SDS_RAM_VERSION_SIZE,
            .payload = &version_packed,
            .finalize = true,
        },
    },
    [JUNO_SDS_RAM_FEATURES_IDX] = {
        .name = "",
        .data = &(struct mod_sds_structure_desc) {
            .id = JUNO_SDS_FEATURE_AVAILABILITY,
            .size = sizeof(feature_flags),
            .payload = &feature_flags,
            .finalize = true,
        },
    },
    [JUNO_SDS_RAM_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    static_assert(BUILD_VERSION_MAJOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_MINOR < UINT8_MAX, "Invalid version size");
    static_assert(BUILD_VERSION_PATCH < UINT16_MAX, "Invalid version size");

    return element_table;
}

struct fwk_module_config config_sds = {
    .get_element_table = get_element_table,
    .data = &(struct mod_sds_config) {
        .region_base_address = SDS_SHARED_MEM_BASE,
        .region_size = SDS_SHARED_MEM_SIZE,
        .clock_id = FWK_ID_NONE_INIT,
    },
};
