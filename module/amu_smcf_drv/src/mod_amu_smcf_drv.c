/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_amu_smcf_drv.h>
#include <mod_smcf.h>

#include <interface_amu.h>

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#define MOD_NAME "[amu_smcf_drv]"

/*!
 * \brief amu_smcf_drv module context.
 */
struct mod_amu_smcf_drv_ctx {
    /*! Number of cores */
    uint32_t num_of_cores;

    /*! Per core context table for amu_smcf_drv*/
    struct amu_smcf_drv_element_config *element_config_table;

    /*! SMCF data buffer for amu data for each core */
    struct mod_smcf_buffer amu_smcf_data_buf;

    /*! SMCF data sampling API */
    const struct smcf_data_api *data_api;
} amu_smcf_drv_ctx;

static int amu_smcf_drv_get_counter_value(
    fwk_id_t start_counter_id,
    uint64_t *counter_buff,
    size_t num_counter)
{
    int status = FWK_SUCCESS;
    uint32_t *amu_smcf_data;
    uint64_t *counter_addr;
    uintptr_t counter_offset;
    unsigned int i, core_idx, amu_counter_idx;
    struct amu_smcf_drv_element_config *core_counters_cfg;
    struct mod_smcf_buffer smcf_tag_buf;
    static uint32_t tag_buf[AMU_TAG_BUFFER_SIZE];

    /* arguments check */
    if (!fwk_module_is_valid_sub_element_id(start_counter_id) ||
        counter_buff == NULL || num_counter == 0) {
        return FWK_E_PARAM;
    }

    core_idx = fwk_id_get_element_idx(start_counter_id);
    amu_counter_idx = fwk_id_get_sub_element_idx(start_counter_id);
    core_counters_cfg = &(amu_smcf_drv_ctx.element_config_table[core_idx]);
    if (amu_counter_idx + num_counter >
        core_counters_cfg->total_num_of_counters) {
        return FWK_E_RANGE;
    }

    /* Prepare smcf get_data arguments */
    memset(tag_buf, 0, sizeof(uint32_t) * AMU_TAG_BUFFER_SIZE);
    memset(
        amu_smcf_drv_ctx.amu_smcf_data_buf.ptr,
        0,
        amu_smcf_drv_ctx.amu_smcf_data_buf.size);
    smcf_tag_buf.ptr = tag_buf;
    smcf_tag_buf.size = AMU_TAG_BUFFER_SIZE;
    amu_smcf_data = amu_smcf_drv_ctx.amu_smcf_data_buf.ptr;

    status = amu_smcf_drv_ctx.data_api->get_data(
        core_counters_cfg->smcf_mli_id,
        amu_smcf_drv_ctx.amu_smcf_data_buf,
        smcf_tag_buf);

    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Copy individual counter */
    for (i = 0; i < num_counter; ++i) {
        counter_offset =
            (uintptr_t)core_counters_cfg->counter_offsets[amu_counter_idx + i];
        counter_addr = (uint64_t *)((uintptr_t)amu_smcf_data + counter_offset);
        counter_buff[i] = *counter_addr;
    }

    return FWK_SUCCESS;
}

static const struct amu_api amu_smcf_drv_api = {
    .get_counters = amu_smcf_drv_get_counter_value,
};

/*
 * Framework handlers
 */
static int amu_smcf_drv_mod_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    if (element_count == 0) {
        return FWK_E_PARAM;
    }

    amu_smcf_drv_ctx.num_of_cores = element_count;
    amu_smcf_drv_ctx.element_config_table = fwk_mm_calloc(
        element_count, sizeof(struct amu_smcf_drv_element_config));

    return FWK_SUCCESS;
}

static int amu_smcf_drv_element_init(
    fwk_id_t element_id,
    unsigned int counters_count,
    const void *data)
{
    unsigned int i;
    unsigned int core_idx;
    uint64_t min_possible_offset;
    uint32_t *amu_counter_offsets;
    struct amu_smcf_drv_element_config *core_counters_cfg;

    if (!fwk_module_is_valid_element_id(element_id) || data == NULL) {
        return FWK_E_PARAM;
    }

    core_idx = fwk_id_get_element_idx(element_id);
    core_counters_cfg = (struct amu_smcf_drv_element_config *)data;
    amu_counter_offsets = core_counters_cfg->counter_offsets;
    /* Verify all offsets in element config so that counters data won't overlap
     * --------------------------------------------------------------
     * |       |       |       |                   |        |       |
     * | CNTR1 | CNTR2 | CNTR3 |        GAP        | CNTR4  | CNTR5 |
     * |       |       |       |                   |        |       |
     * --------------------------------------------------------------
     * |       |       |                           |        |
     * .-------.-------.---------------------------.--------.--------
     *                      Offset(s)
     * Mandatory Condition for configs so that counter data won't overlap:
     * COUNTER_OFFSET(I) >= COUNTER_OFFSET(I-1) + COUNTER_DATA_SZ
     */
    for (i = 1; i < counters_count; ++i) {
        min_possible_offset = amu_counter_offsets[i - 1] + sizeof(uint64_t);
        if (amu_counter_offsets[i] < min_possible_offset) {
            return FWK_E_ALIGN;
        }
    }

    amu_smcf_drv_ctx.element_config_table[core_idx] = *core_counters_cfg;
    amu_smcf_drv_ctx.element_config_table[core_idx].total_num_of_counters =
        counters_count;

    return FWK_SUCCESS;
}

static int amu_smcf_drv_start(fwk_id_t id)
{
    unsigned int i, num_counters;
    uint32_t per_core_amu_data_max_size, total_counter_data_sz;
    struct amu_smcf_drv_element_config *core_counters_cfg;

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        /* elements has already been started with module start */
        return FWK_SUCCESS;
    }

    if (!fwk_module_is_valid_module_id(id)) {
        return FWK_E_PARAM;
    }

    /* Calculate the max size for the smcf counter data buffer to accomodate
     * counter data for each element.
     * SMCF_COUNTER_DATA_SZ = MAX(COUNTER_DATA_SZ_FOR_EACH_CORE)
     */
    per_core_amu_data_max_size = 0;
    for (i = 0; i < amu_smcf_drv_ctx.num_of_cores; ++i) {
        core_counters_cfg = &(amu_smcf_drv_ctx.element_config_table[i]);
        num_counters = core_counters_cfg->total_num_of_counters;
        total_counter_data_sz =
            core_counters_cfg->counter_offsets[num_counters - 1] -
            core_counters_cfg->counter_offsets[0] + sizeof(uint64_t);
        per_core_amu_data_max_size =
            FWK_MAX(per_core_amu_data_max_size, total_counter_data_sz);
    }

    amu_smcf_drv_ctx.amu_smcf_data_buf.size =
        (per_core_amu_data_max_size / sizeof(uint32_t));
    amu_smcf_drv_ctx.amu_smcf_data_buf.ptr = fwk_mm_calloc(
        amu_smcf_drv_ctx.amu_smcf_data_buf.size, sizeof(uint32_t));

    return FWK_SUCCESS;
}

static int amu_smcf_drv_bind(fwk_id_t id, unsigned int round)
{
    /* Bind to SMCF Data API */
    return fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SMCF),
        FWK_ID_API(FWK_MODULE_IDX_SMCF, MOD_SMCF_API_IDX_DATA),
        &amu_smcf_drv_ctx.data_api);
}

static int amu_smcf_drv_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t pd_id,
    fwk_id_t api_id,
    const void **api)
{
    if (fwk_id_is_equal(
            requester_id, FWK_ID_MODULE(FWK_MODULE_IDX_AMU_SMCF_DRV))) {
        return FWK_E_PARAM;
    }

    *api = &amu_smcf_drv_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_amu_smcf_drv = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_AMU_SMCF_DRV_API_IDX_COUNT,
    .init = amu_smcf_drv_mod_init,
    .element_init = amu_smcf_drv_element_init,
    .start = amu_smcf_drv_start,
    .bind = amu_smcf_drv_bind,
    .process_bind_request = amu_smcf_drv_process_bind_request,
};
