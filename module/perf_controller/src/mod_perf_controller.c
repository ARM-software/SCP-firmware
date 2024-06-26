/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/perf_controller.h>

#include <interface_power_management.h>

#include <fwk_assert.h>
#include <fwk_mm.h>
#include <fwk_module.h>

static struct mod_perf_controller_ctx perf_controller_ctx;

static struct mod_perf_controller_internal_api internal_api;

static uint32_t get_cores_min_power_limit(
    struct mod_perf_controller_cluster_ctx *cluster_ctx)
{
    unsigned int core_idx;
    struct mod_perf_controller_core_ctx *core_ctx;
    uint32_t min_power_limit = UINT32_MAX;

    for (core_idx = 0U; core_idx < cluster_ctx->core_count; core_idx++) {
        core_ctx = &(cluster_ctx->core_ctx_table[core_idx]);
        min_power_limit = FWK_MIN(min_power_limit, core_ctx->power_limit);
    }

    return min_power_limit;
}

static int cluster_apply_performance_granted(
    struct mod_perf_controller_cluster_ctx *cluster_ctx)
{
    uint32_t min_power_limit;
    uint32_t performance_limit;
    uint32_t requested_performance;
    uintptr_t cookie;
    int status;

    min_power_limit = internal_api.get_cores_min_power_limit(cluster_ctx);
    status = cluster_ctx->power_model_api->power_to_performance(
        cluster_ctx->config->power_model_id,
        min_power_limit,
        &performance_limit);

    if (status != FWK_SUCCESS) {
        return status;
    }

    if (cluster_ctx->performance_request_details.level <= performance_limit) {
        cookie = cluster_ctx->performance_request_details.cookie;
        requested_performance = cluster_ctx->performance_request_details.level;
    } else {
        cookie = 0U;
        requested_performance = performance_limit;
    }

    return cluster_ctx->perf_driver_api->set_performance_level(
        cluster_ctx->config->performance_driver_id,
        cookie,
        requested_performance);
}

static struct mod_perf_controller_internal_api internal_api = {
    .get_cores_min_power_limit = get_cores_min_power_limit,
    .cluster_apply_performance_granted = cluster_apply_performance_granted,
};

static int mod_perf_controller_set_performance_level(
    fwk_id_t cluster_id,
    uintptr_t cookie,
    uint32_t performance_level)
{
    int status;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;

    cluster_ctx = &perf_controller_ctx
                       .cluster_ctx_table[fwk_id_get_element_idx(cluster_id)];

    cluster_ctx->performance_request_details.level = performance_level;

    if (performance_level <= cluster_ctx->performance_limit) {
        status = cluster_ctx->perf_driver_api->set_performance_level(
            cluster_ctx->config->performance_driver_id,
            cookie,
            performance_level);
    } else {
        status = FWK_PENDING;
        cluster_ctx->performance_request_details.cookie = cookie;
    }

    return status;
}

static struct mod_perf_controller_perf_api perf_api = {
    .set_performance_level = mod_perf_controller_set_performance_level,
};

static int mod_perf_controller_set_limit(fwk_id_t core_id, uint32_t power_limit)
{
    unsigned int core_idx;
    unsigned int cluster_idx;
    struct mod_perf_controller_core_ctx *core_ctx;

    cluster_idx = fwk_id_get_element_idx(core_id);
    core_idx = fwk_id_get_sub_element_idx(core_id);

    core_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx]
                    .core_ctx_table[core_idx];

    core_ctx->power_limit = power_limit;

    return FWK_SUCCESS;
}

static struct interface_power_management_api power_api = {
    .set_limit = mod_perf_controller_set_limit,
};

static int mod_perf_controller_apply_performance_granted(void)
{
    uint32_t cluster_idx;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;
    int status = FWK_SUCCESS;

    for (cluster_idx = 0U; (cluster_idx < perf_controller_ctx.cluster_count) &&
         (status == FWK_SUCCESS);
         cluster_idx++) {
        cluster_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx];
        status = internal_api.cluster_apply_performance_granted(cluster_ctx);
    }
    return status;
}

static struct mod_perf_controller_apply_performance_granted_api
    apply_performance_granted_api = {
        .apply_performance_granted =
            mod_perf_controller_apply_performance_granted,
    };

/*
 * Framework handlers
 */
static int mod_perf_controller_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0U) {
        return FWK_E_PARAM;
    }

    perf_controller_ctx.cluster_count = element_count;

    perf_controller_ctx.cluster_ctx_table = fwk_mm_calloc(
        element_count, sizeof(struct mod_perf_controller_cluster_ctx));

    return FWK_SUCCESS;
}

static int mod_perf_controller_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_perf_controller_cluster_ctx *cluster_ctx;
    const struct mod_perf_controller_cluster_config *cluster_config;

    if ((sub_element_count == 0U) || (data == NULL)) {
        return FWK_E_PARAM;
    }

    cluster_ctx = &perf_controller_ctx
                       .cluster_ctx_table[fwk_id_get_element_idx(element_id)];

    cluster_ctx->core_ctx_table = fwk_mm_calloc(
        sub_element_count, sizeof(struct mod_perf_controller_core_ctx));

    cluster_config = (const struct mod_perf_controller_cluster_config *)data;

    cluster_ctx->config = cluster_config;

    cluster_ctx->core_count = sub_element_count;

    cluster_ctx->performance_limit = cluster_config->initial_performance_limit;

    return FWK_SUCCESS;
}

static int mod_perf_controller_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;

    if ((round != 0U) || (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))) {
        return FWK_SUCCESS;
    }

    cluster_ctx =
        &perf_controller_ctx.cluster_ctx_table[fwk_id_get_element_idx(id)];

    status = fwk_module_bind(
        cluster_ctx->config->performance_driver_id,
        cluster_ctx->config->performance_driver_api_id,
        &cluster_ctx->perf_driver_api);

    if (status != FWK_SUCCESS) {
        return status;
    }

    return fwk_module_bind(
        cluster_ctx->config->power_model_id,
        cluster_ctx->config->power_model_api_id,
        &cluster_ctx->power_model_api);
}

static int mod_perf_controller_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    int status;
    enum mod_perf_controller_api_idx api_idx;

    if (fwk_id_get_module_idx(api_id) != FWK_MODULE_IDX_PERF_CONTROLLER) {
        return FWK_E_PARAM;
    }

    api_idx = (enum mod_perf_controller_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_idx) {
    case MOD_PERF_CONTROLLER_CLUSTER_PERF_API:
        *api = &perf_api;
        status = FWK_SUCCESS;
        break;
    case MOD_PERF_CONTROLLER_CORE_POWER_API:
        *api = &power_api;
        status = FWK_SUCCESS;
        break;
    case MOD_PERF_CONTROLLER_APPLY_PERFORMANCE_GRANTED_API:
        *api = &apply_performance_granted_api;
        status = FWK_SUCCESS;
        break;
    default:
        status = FWK_E_PARAM;
        break;
    }

    return status;
}

const struct fwk_module module_perf_controller = {
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = (uint32_t)MOD_PERF_CONTROLLER_API_COUNT,
    .init = mod_perf_controller_init,
    .element_init = mod_perf_controller_element_init,
    .bind = mod_perf_controller_bind,
    .process_bind_request = mod_perf_controller_process_bind_request,
};
