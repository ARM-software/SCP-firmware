/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_stats.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_string.h>
#include <fwk_time.h>

/* 'PERF' = 0x50455246 in SCP little-endian */
#define STATS_SIGN_PERF 0x50455246
/* 'POWR' = 0x504F5752 in SCP little-endian */
#define STATS_SIGN_POWR 0x504F5752

#define STATS_UPDATE_PERIOD_MS  100

struct mod_stats_ctx {
    /* Platform specific memory configuration data */
    const struct mod_stats_config_info *config;

    /* Offset of the available memory in the statistics region */
    uint32_t avail_mem_offset;

    /* Context of performance statistics */
    struct mod_stats_info *perf_stats;

    /* Context of power domain statistics */
    struct mod_stats_info *power_stats;

    /* Alarm API for periodic shared memory updates */
    const struct mod_timer_alarm_api *alarm_api;
};

static struct mod_stats_ctx stats_ctx;

static struct mod_stats_info *get_module_stats_info(fwk_id_t module_id)
{
    if (fwk_id_get_module_idx(module_id) ==
        fwk_id_get_module_idx(fwk_module_id_scmi_perf)) {
        return stats_ctx.perf_stats;
    }

    if (fwk_id_get_module_idx(module_id) ==
        fwk_id_get_module_idx(fwk_module_id_scmi_power_domain)) {
        return stats_ctx.power_stats;
    }

    return NULL;
}

static int set_module_stats_info(fwk_id_t module_id,
    struct mod_stats_info *stats)
{
    int ret = FWK_E_PARAM;

    if (fwk_id_get_module_idx(module_id) ==
        fwk_id_get_module_idx(fwk_module_id_scmi_perf)) {
        stats_ctx.perf_stats = stats;
        stats->type_signature = STATS_SIGN_PERF;
        ret = FWK_SUCCESS;
    }

    if (fwk_id_get_module_idx(module_id) ==
        fwk_id_get_module_idx(fwk_module_id_scmi_power_domain)) {
        stats_ctx.power_stats = stats;
        stats->type_signature = STATS_SIGN_POWR;
        ret = FWK_SUCCESS;
    }

    return ret;
}

static int allocate_domain_stats(fwk_id_t module_id,
    fwk_id_t domain_id,
    int level_count)
{
    struct mod_stats_desc_header *desc_header;
    struct mod_stats_info *stats;
    struct mod_stats_map *se_map;
    uintptr_t scp_stats_addr;
    uint32_t stats_offset;
    uint32_t stats_size;
    int stats_id;
    uint32_t idx;

    stats = get_module_stats_info(module_id);
    if (stats == NULL) {
        return FWK_E_PARAM;
    }

    idx = fwk_id_get_element_idx(domain_id);
    desc_header = stats->desc_header;

    stats_id = stats->context->last_stats_id++;
    fwk_assert(stats_id < stats->context->se_used_num);

    stats->context->se_index_map[idx] = stats_id;

    /* Calculate needed memory for variable length array inside
     * domain_stats_data structure. That structure is directly
     * mapped into shared memory area containing statistics for this
     * domain. The size depends on number of levels */
    stats_size = sizeof(struct mod_stats_level_stats) * level_count;
    stats_size += sizeof(struct mod_stats_domain_stats_data);

    if (stats_size > (stats_ctx.config->stats_region_size -
        stats_ctx.avail_mem_offset)) {
        FWK_LOG_ERR("[STATS]: Error, size of statistics region too small\n");
        stats->mode = STATS_INTERNAL_ERROR;
        return FWK_E_NOMEM;
    }

    se_map = stats->context->se_stats_map;
    se_map->se_level_count[idx] = level_count;
    se_map->se_curr_level[idx] = 0;

    /* Offset from the beginning of statistics header used by AP */
    stats_offset = stats_ctx.avail_mem_offset - stats->desc_header_offset;
    desc_header->domain_offset[idx] = stats_offset;

    /* Address used in SCP to get domain statistics in the shared region */
    scp_stats_addr = stats_ctx.config->scp_stats_addr +
                     stats_ctx.avail_mem_offset;
    se_map->se_stats[idx] = (struct mod_stats_domain_stats_data *)
                            scp_stats_addr;

    /* Shrink the free space in the shared region */
    stats_ctx.avail_mem_offset += stats_size;

    stats->used_mem_size += stats_size;

    FWK_LOG_DEBUG(
        "[STATS]: stats addr %lx, stats_size=%luB\n",
        (uint32_t)scp_stats_addr,
        stats_size);

    return FWK_SUCCESS;
}

static int _allocate_header(struct mod_stats_info *stats, int domain_count)
{
    stats->desc_header_size = sizeof(struct mod_stats_desc_header);
    stats->desc_header_size += domain_count * sizeof(uint32_t);

    if (stats->desc_header_size > (stats_ctx.config->stats_region_size -
        stats_ctx.avail_mem_offset)) {
        FWK_LOG_ERR("[STATS]: Error, size of statistics region too small\n");
        stats->mode = STATS_INTERNAL_ERROR;
        return FWK_E_NOMEM;
    }

    stats->desc_header_offset = stats_ctx.avail_mem_offset;
    stats->desc_header = (struct mod_stats_desc_header *)
                         (stats_ctx.config->scp_stats_addr +
                          stats_ctx.avail_mem_offset);

    stats->used_mem_size += stats->desc_header_size;
    stats_ctx.avail_mem_offset += stats->desc_header_size;

    return FWK_SUCCESS;
}

static struct mod_stats_info *
_allocate_stats_context(int domain_count, int used_domains)
{
    struct mod_stats_info *stats;
    struct mod_stats_map *se_map;
    int se_map_size = 0;
    int i;

    stats = fwk_mm_calloc(1, sizeof(struct mod_stats_info));
    stats->context = fwk_mm_calloc(1, sizeof(struct mod_stats_context));
    stats->context->se_index_map =
        (int *)fwk_mm_calloc((size_t)domain_count, sizeof(int));

    stats->mode = STATS_SETUP;

    /* Set default values indicating that the domain is not used in
     * statistics collection */
    for (i = 0; i < domain_count; i++) {
        stats->context->se_index_map[i] = FWK_E_SUPPORT;
    }

    stats->context->se_total_num = domain_count;
    stats->context->se_used_num = used_domains;
    stats->context->last_stats_id = 0;

    /* Calculate needed memory for variable length array inside
     * stats_map structure */
    se_map_size = used_domains;
    se_map_size *= sizeof(struct mod_stats_domain_stats_data *);
    se_map_size += sizeof(struct mod_stats_map);

    stats->context->se_stats_map =
        (struct mod_stats_map *)fwk_mm_calloc(1, (size_t)se_map_size);
    se_map = stats->context->se_stats_map;

    se_map->se_level_count =
        (int *)fwk_mm_calloc((size_t)used_domains, sizeof(int));
    se_map->se_curr_level =
        (uint32_t *)fwk_mm_calloc((size_t)used_domains, sizeof(uint32_t));

    return stats;
}

static int stats_init_module(fwk_id_t module_id,
    int domain_count,
    int used_domains)
{
    struct mod_stats_info *stats;
    int ret;

    FWK_LOG_INFO("[STATS]: init module, total_domains=%d used=%d\n",
                 domain_count, used_domains);

    stats = _allocate_stats_context(domain_count, used_domains);
    ret = set_module_stats_info(module_id, stats);

    fwk_assert(ret == FWK_SUCCESS);

    ret = _allocate_header(stats, domain_count);
    if (ret != FWK_SUCCESS) {
        return ret;
    }

    stats->desc_header->signature = stats->type_signature;
    stats->desc_header->domain_count = (uint16_t)domain_count;

    return FWK_SUCCESS;
}

static int stats_start_module(fwk_id_t module_id)
{
    struct mod_stats_info *stats;

    stats = get_module_stats_info(module_id);
    if (stats == NULL) {
        return FWK_E_PARAM;
    }

    if (stats->mode == STATS_SETUP) {
        if (stats->context->last_stats_id == stats->context->se_used_num) {
            stats->mode = STATS_INITIALIZED;
            return FWK_SUCCESS;
        } else {
            stats->mode = STATS_NOT_SUPPORTED;
            return FWK_E_SUPPORT;
        }
    }

    return FWK_E_SUPPORT;
}

static struct mod_stats_domain_stats_data *
get_domain_section_data(fwk_id_t module_id, fwk_id_t domain_id)
{
    struct mod_stats_domain_stats_data *domain_stats = NULL;
    struct mod_stats_info *stats;
    int stats_id;
    uint32_t idx;

    idx = fwk_id_get_element_idx(domain_id);
    stats = get_module_stats_info(module_id);
    if (stats == NULL) {
        return NULL;
    }

    fwk_assert((int)idx < stats->context->se_total_num);

    stats_id = stats->context->se_index_map[idx];
    if (stats_id != FWK_E_SUPPORT) {
        domain_stats = stats->context->se_stats_map->se_stats[stats_id];
    }

    return domain_stats;
}

static int stats_add_domain(fwk_id_t module_id,
    fwk_id_t domain_id,
    int level_count)
{
    struct mod_stats_domain_stats_data *domain_stats;
    struct mod_stats_level_stats *level_stats;
    struct mod_stats_info *stats;
    uint32_t idx;
    int i, ret;

    idx = fwk_id_get_element_idx(domain_id);
    stats = get_module_stats_info(module_id);
    if (stats == NULL) {
        return FWK_E_PARAM;
    }

    fwk_assert((int)idx < stats->context->se_total_num);

    ret = allocate_domain_stats(module_id, domain_id, level_count);
    if (ret != FWK_SUCCESS) {
        return ret;
    }

    domain_stats = get_domain_section_data(module_id, domain_id);
    if (domain_stats == NULL) {
        return FWK_E_PARAM;
    }

    domain_stats->level_count = (uint16_t)level_count;

    for (i = 0; i < level_count; i++) {
        level_stats = &domain_stats->level[i];
        level_stats->level_id = (uint32_t)i;
    }

    return FWK_SUCCESS;
}

/* This is only temporary, it will be swapped with system ts func */
static uint64_t _get_curret_ts_us(void)
{
    fwk_timestamp_t timestamp = 0;
    fwk_duration_ns_t duration = 0;
    uint64_t ts_us;

    timestamp = fwk_time_current();
    duration = fwk_time_stamp_duration(timestamp);
    ts_us = fwk_time_duration_us(duration);

    return ts_us;
}

static int
stats_update_domain(fwk_id_t module_id, fwk_id_t domain_id, uint32_t level_id)
{
    struct mod_stats_domain_stats_data *domain_stats;
    struct mod_stats_level_stats *level_stats;
    struct mod_stats_info *stats;
    struct mod_stats_map *se_map;
    uint64_t ts_now_us;
    uint32_t old_level_id, idx;
    int stats_id;
    unsigned int flags;

    stats = get_module_stats_info(module_id);
    if (stats == NULL) {
        return FWK_E_PARAM;
    }

    if (stats->mode != STATS_INITIALIZED) {
        return FWK_E_SUPPORT;
    }

    domain_stats = get_domain_section_data(module_id, domain_id);
    if (domain_stats == NULL) {
        return FWK_E_PARAM;
    }

    idx = fwk_id_get_element_idx(domain_id);

    se_map = stats->context->se_stats_map;
    stats_id = stats->context->se_index_map[idx];

    if ((int)level_id >= se_map->se_level_count[stats_id]) {
        return FWK_E_PARAM;
    }

    ts_now_us = _get_curret_ts_us();

    flags = fwk_interrupt_global_disable();

    /* Update old performance level statistics */
    old_level_id = se_map->se_curr_level[stats_id];
    level_stats = &domain_stats->level[old_level_id];
    level_stats->total_residency_us += ts_now_us;
    level_stats->total_residency_us -= domain_stats->ts_last_change_us;

    /* Update new performance level statistics */
    level_stats = &domain_stats->level[level_id];
    level_stats->usage_count++;
    domain_stats->ts_last_change_us = ts_now_us;
    domain_stats->curr_level_id = (uint16_t)level_id;
    se_map->se_curr_level[stats_id] = level_id;

    fwk_interrupt_global_enable(flags);

    return FWK_SUCCESS;
}

static int
get_statistics_desc(fwk_id_t module_id,
    uint32_t *addr_low,
    uint32_t *addr_high,
    uint32_t *len)
{
    struct mod_stats_info *stats;
    uint64_t ap_stats_addr;

    stats = get_module_stats_info(module_id);
    if (stats == NULL) {
        return FWK_E_PARAM;
    }

    if (stats->mode != STATS_INITIALIZED) {
        return FWK_E_SUPPORT;
    }

    ap_stats_addr = stats_ctx.config->ap_stats_addr;
    ap_stats_addr += stats->desc_header_offset;

    *addr_low = (uint32_t)(ap_stats_addr & ~0UL);
    *addr_high = (uint32_t)(ap_stats_addr >> 32);
    *len = stats->used_mem_size;

    return FWK_SUCCESS;
}

static const struct mod_stats_api mod_statistics_api = {
    .init_stats = stats_init_module,
    .start_stats = stats_start_module,
    .add_domain = stats_add_domain,
    .update_domain = stats_update_domain,
    .get_statistics_desc = get_statistics_desc,
};

static void update_all_domains_current_level(fwk_id_t module_id)
{
    struct mod_stats_domain_stats_data *domain_stats;
    struct mod_stats_level_stats *level_stats;
    struct mod_stats_info *stats;
    struct mod_stats_map *se_map;
    uint64_t ts_now_us, delta_t;
    uint32_t curr_level_id;
    fwk_id_t domain_id;
    int stats_id, i;
    unsigned int flags;

    stats = get_module_stats_info(module_id);
    if (stats == NULL) {
        return;
    }

    if (stats->mode != STATS_INITIALIZED) {
        return;
    }

    for (i = 0; i < stats->context->se_total_num; i++) {
        domain_id = FWK_ID_ELEMENT(fwk_id_get_module_idx(module_id), i);
        domain_stats = get_domain_section_data(module_id, domain_id);
        if (domain_stats == NULL) {
            continue;
        }

        se_map = stats->context->se_stats_map;
        stats_id = stats->context->se_index_map[i];

        flags = fwk_interrupt_global_disable();

        ts_now_us = _get_curret_ts_us();

        /* Update current operation level statistics */
        delta_t = ts_now_us - domain_stats->ts_last_change_us;

        curr_level_id = se_map->se_curr_level[stats_id];
        level_stats = &domain_stats->level[curr_level_id];
        level_stats->total_residency_us += delta_t;
        domain_stats->ts_last_change_us = ts_now_us;

        fwk_interrupt_global_enable(flags);
    }

}

static void periodic_update_callback(uintptr_t param)
{
    /* Update current level stats in all tracked domains in the perf module */
    update_all_domains_current_level(fwk_module_id_scmi_perf);

    /* Update current level stats in all tracked domains in the power module */
    update_all_domains_current_level(fwk_module_id_scmi_power_domain);
}

static int register_module_stats(fwk_id_t module_id)
{
    return FWK_SUCCESS;
}

static int stats_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    const struct mod_stats_config_info *config = data;

    if (config == NULL || config->stats_region_size == 0) {
        FWK_LOG_INFO("STATS: statistics are not configured\n");
        return FWK_E_SUPPORT;
    }

    /* Force platform to populate all addresses with valid data */
    fwk_assert(config->scp_stats_addr != 0);
    /* It's unlikely that AP would have shared SRAM at address 0x0 */
    fwk_assert(config->ap_stats_addr != 0);

    /* Make sure that there is no stale data */
    fwk_str_memset(
        (void *)config->scp_stats_addr, 0, config->stats_region_size);

    stats_ctx.config = config;
    stats_ctx.avail_mem_offset = 0;

    return FWK_SUCCESS;
}

static int stats_element_init(fwk_id_t domain_id,
    unsigned int sub_element_count,
    const void *data)
{
    return FWK_SUCCESS;
}

static int stats_post_init(fwk_id_t module_id)
{
    return FWK_SUCCESS;
}

static int stats_start(fwk_id_t id)
{
    int status;

    if (!fwk_id_is_equal(stats_ctx.config->alarm_id, FWK_ID_NONE)) {
        status = stats_ctx.alarm_api->start(stats_ctx.config->alarm_id,
            STATS_UPDATE_PERIOD_MS, MOD_TIMER_ALARM_TYPE_PERIODIC,
            periodic_update_callback, (uintptr_t)0);

        if (status != FWK_SUCCESS) {
            return status;
        }
    } else {
        FWK_LOG_ERR("[STATS]: failed to start period updates\n");
        return FWK_E_SUPPORT;
    }

    return FWK_SUCCESS;
}

static int stats_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round >= 1) {
        return FWK_SUCCESS;
    }

    if (!fwk_id_is_equal(stats_ctx.config->alarm_id, FWK_ID_NONE)) {
        status = fwk_module_bind(stats_ctx.config->alarm_id,
            MOD_TIMER_API_ID_ALARM, &stats_ctx.alarm_api);
        if (status != FWK_SUCCESS) {
            return FWK_E_PANIC;
        }
    }

    return FWK_SUCCESS;
}

static int process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only allow binding to the module */
    if (!fwk_id_is_equal(target_id, fwk_module_id_statistics)) {
        return FWK_E_PARAM;
    }

    *api = &mod_statistics_api;

    /* Request from SCMI Performance domain statistics */
    if (fwk_id_get_module_idx(source_id) ==
        fwk_id_get_module_idx(fwk_module_id_scmi_perf)) {
        return register_module_stats(fwk_module_id_scmi_perf);
    }

    /* Request from SCMI Power domain statistics*/
    if (fwk_id_get_module_idx(source_id) ==
        fwk_id_get_module_idx(fwk_module_id_scmi_power_domain)) {
        return register_module_stats(fwk_module_id_scmi_power_domain);
    }

    return FWK_E_PARAM;
}

const struct fwk_module module_statistics = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = stats_init,
    .element_init = stats_element_init,
    .post_init = stats_post_init,
    .start = stats_start,
    .bind = stats_bind,
    .process_bind_request = process_bind_request,
    .api_count = (unsigned int)MOD_STATS_API_IDX_COUNT,
};
