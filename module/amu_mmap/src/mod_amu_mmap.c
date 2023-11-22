/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <interface_amu.h>

#include <mod_amu_mmap.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

struct mod_core_amu_counters {
    /* Core configuration */
    struct mod_core_element_config *core_config;
    /* Number of counters */
    size_t num_counters;
};

static struct mod_amu_mmap {
    /* Number of cores */
    uint32_t core_count;
    /* AMU counters per core*/
    struct mod_core_amu_counters *core;
} amu_mmap;

static inline uint64_t *amu_calc_counter_address(
    uint64_t *counters_base_addr,
    uint32_t offset)
{
    return (uint64_t *)((uintptr_t)counters_base_addr + (uintptr_t)offset);
}

/* Module APIs */

static int amu_mmap_get_counters(
    fwk_id_t start_counter_id,
    uint64_t *counter_buff,
    size_t num_counter)
{
    uint32_t core_idx;
    uint32_t start_counter_idx;
    uint64_t *counter_address = NULL;
    uint32_t *offsets = NULL;

    if (!fwk_module_is_valid_sub_element_id(start_counter_id) ||
        counter_buff == NULL) {
        return FWK_E_PARAM;
    }

    core_idx = start_counter_id.sub_element.element_idx;
    start_counter_idx = start_counter_id.sub_element.sub_element_idx;
    offsets = amu_mmap.core[core_idx].core_config->counters_offsets;

    if (start_counter_idx + num_counter >
        amu_mmap.core[core_idx].num_counters) {
        return FWK_E_RANGE;
    }

    for (size_t i = 0; i < num_counter; ++i) {
        counter_address = amu_calc_counter_address(
            amu_mmap.core[core_idx].core_config->counters_base_addr,
            offsets[start_counter_idx + i]);
        counter_buff[i] = *counter_address;
    }

    return FWK_SUCCESS;
}

struct amu_api amu_api = {
    .get_counters = amu_mmap_get_counters,
};

/*
 * Framework handlers
 */
static int amu_mmap_init(
    fwk_id_t module_id,
    unsigned int core_count,
    const void *unused)
{
    if (core_count == 0) {
        return FWK_E_PARAM;
    }

    amu_mmap.core =
        fwk_mm_calloc(core_count, sizeof(struct mod_core_amu_counters));
    amu_mmap.core_count = core_count;

    return FWK_SUCCESS;
}

static int amu_mmap_element_init(
    fwk_id_t core_id,
    unsigned int sub_element_count,
    const void *data)
{
    unsigned int core_idx;

    if (!fwk_module_is_valid_element_id(core_id) || data == NULL) {
        return FWK_E_PARAM;
    }

    core_idx = fwk_id_get_element_idx(core_id);
    amu_mmap.core[core_idx].core_config =
        (struct mod_core_element_config *)data;
    amu_mmap.core[core_idx].num_counters = sub_element_count;

    if (amu_mmap.core[core_idx].core_config->counters_base_addr == NULL ||
        amu_mmap.core[core_idx].core_config->counters_offsets == NULL) {
        return FWK_E_ACCESS;
    }

    return FWK_SUCCESS;
}

static int amu_mmap_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

static int amu_mmap_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only allow binding to the module */
    if (!fwk_id_is_equal(target_id, FWK_ID_MODULE(FWK_MODULE_IDX_AMU_MMAP))) {
        return FWK_E_PARAM;
    }

    if (!fwk_id_is_equal(api_id, mod_amu_mmap_api_id_amu)) {
        return FWK_E_ACCESS;
    }

    *api = &amu_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_amu_mmap = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = amu_mmap_init,
    .element_init = amu_mmap_element_init,
    .start = amu_mmap_start,
    .process_bind_request = amu_mmap_process_bind_request,
};
