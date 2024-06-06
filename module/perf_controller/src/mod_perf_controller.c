/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/perf_controller.h>

#include <fwk_assert.h>
#include <fwk_mm.h>
#include <fwk_module.h>

static struct mod_perf_controller_cluster_ctx *cluster_ctx_table;

static int set_performance_level(
    fwk_id_t cluster_id,
    uintptr_t cookie,
    uint32_t performance_level)
{
    struct mod_perf_controller_cluster_ctx *cluster_ctx;

    cluster_ctx = &cluster_ctx_table[fwk_id_get_element_idx(cluster_id)];

    return cluster_ctx->perf_driver_api->set_performance_level(
        cluster_ctx->config->driver_id, cookie, performance_level);
}

static struct mod_perf_controller_perf_api perf_api = {
    .set_performance_level = set_performance_level,
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
    cluster_ctx_table = fwk_mm_calloc(
        element_count, sizeof(struct mod_perf_controller_cluster_ctx));

    return FWK_SUCCESS;
}

static int mod_perf_controller_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_perf_controller_cluster_ctx *cluster_ctx;

    if (sub_element_count == 0U) {
        return FWK_E_PARAM;
    }

    cluster_ctx = &cluster_ctx_table[fwk_id_get_element_idx(element_id)];

    cluster_ctx->core_ctx_table = fwk_mm_calloc(
        sub_element_count, sizeof(struct mod_perf_controller_core_ctx));

    cluster_ctx->config =
        (const struct mod_perf_controller_cluster_config *)data;

    return FWK_SUCCESS;
}

static int mod_perf_controller_bind(fwk_id_t id, unsigned int round)
{
    if ((round != 0U) || (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))) {
        return FWK_SUCCESS;
    }

    struct mod_perf_controller_cluster_ctx *cluster_ctx;

    cluster_ctx = &cluster_ctx_table[fwk_id_get_element_idx(id)];

    return fwk_module_bind(
        cluster_ctx->config->driver_id,
        cluster_ctx->config->driver_api_id,
        &cluster_ctx->perf_driver_api);
}

static int mod_perf_controller_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    int status;
    enum mod_perf_controller_api_idx api_idx = fwk_id_get_api_idx(api_id);

    switch (api_idx) {
    case MOD_PERF_CONTROLLER_CLUSTER_PERF_API:
        *api = &perf_api;
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
