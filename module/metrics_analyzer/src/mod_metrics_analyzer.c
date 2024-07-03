/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Metrics Analyzer
 */

#include "mod_metrics_analyzer.h"

#include <interface_power_management.h>

#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

struct mod_metric_ctx {
    const struct mod_metrics_analyzer_interactor *limit_provider_config;
    struct interface_power_management_api *limit_provider_api;
    uint32_t limit;
};

struct mod_domain_ctx {
    struct mod_metric_ctx *metrics;
    size_t metrics_count;
    uint32_t aggregate_limit;
    const struct mod_metrics_analyzer_domain_config *config;
    struct interface_power_management_api *limit_consumer_api;
};

struct mod_metrics_analyzer_ctx {
    /* list of system domains context */
    struct mod_domain_ctx *domain;
    /* Number of domains */
    size_t domain_count;
    /* Metrics analyzer configuration (on the module scope) */
    const struct mod_metrics_analyzer_config *config;
};

static struct mod_metrics_analyzer_ctx metrics_analyzer_ctx;

static int collect_domain_limits(struct mod_domain_ctx *domain_ctx)
{
    int status = FWK_SUCCESS;
    if (domain_ctx == NULL) {
        return FWK_E_PARAM;
    }

    domain_ctx->aggregate_limit = UINT32_MAX;
    for (size_t i = 0; i < domain_ctx->metrics_count; ++i) {
        unsigned int power_limit;
        struct mod_metric_ctx *metric_ctx = &domain_ctx->metrics[i];
        status = metric_ctx->limit_provider_api->get_limit(
            metric_ctx->limit_provider_config->domain_id, &power_limit);
        if (status == FWK_SUCCESS) {
            metric_ctx->limit = power_limit;
            domain_ctx->aggregate_limit =
                FWK_MIN(domain_ctx->aggregate_limit, metric_ctx->limit);
        } else {
            metric_ctx->limit = UINT32_MAX;
        }
    }

    return FWK_SUCCESS;
}

static int report_domain_aggregate_limit(struct mod_domain_ctx *domain_ctx)
{
    if (domain_ctx == NULL) {
        return FWK_E_PARAM;
    }
    if (domain_ctx->limit_consumer_api == NULL ||
        domain_ctx->limit_consumer_api->set_limit == NULL) {
        return FWK_E_PANIC;
    }

    return domain_ctx->limit_consumer_api->set_limit(
        domain_ctx->config->limit_consumer.domain_id,
        domain_ctx->aggregate_limit);
}

static int collect_domains_limits(
    struct mod_domain_ctx *domains_ctx,
    size_t domain_count)
{
    if (domain_count && domains_ctx == NULL) {
        return FWK_E_PARAM;
    }

    for (size_t i = 0; i < domain_count; ++i) {
        (void)collect_domain_limits(&domains_ctx[i]);
    }

    return FWK_SUCCESS;
}

static int report_domains_aggregate_limit(
    struct mod_domain_ctx *domains_ctx,
    size_t domain_count)
{
    if (domain_count && domains_ctx == NULL) {
        return FWK_E_PARAM;
    }

    for (size_t i = 0; i < domain_count; ++i) {
        (void)report_domain_aggregate_limit(&domains_ctx[i]);
    }

    return FWK_SUCCESS;
}

static int analyze(void)
{
    int status = FWK_E_INIT;

    status = collect_domains_limits(
        metrics_analyzer_ctx.domain, metrics_analyzer_ctx.domain_count);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = report_domains_aggregate_limit(
        metrics_analyzer_ctx.domain, metrics_analyzer_ctx.domain_count);

    return status;
}

static struct mod_metrics_analyzer_analyze_api analyze_api = {
    .analyze = analyze,
};

/*
 * Framework handlers
 */

static int metrics_analyzer_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *config_data)
{
    if (element_count == 0U ||
        !fwk_id_is_equal(
            module_id, FWK_ID_MODULE(FWK_MODULE_IDX_METRICS_ANALYZER))) {
        return FWK_E_PARAM;
    }

    metrics_analyzer_ctx.domain_count = element_count;
    metrics_analyzer_ctx.domain =
        fwk_mm_calloc(element_count, sizeof(metrics_analyzer_ctx.domain[0]));
    return FWK_SUCCESS;
}

static int metrics_analyzer_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    if ((sub_element_count == 0U) || (sub_element_count >= (1U << 8)) ||
        (data == NULL)) {
        return FWK_E_PARAM;
    }

    size_t domain_idx = fwk_id_get_element_idx(element_id);
    metrics_analyzer_ctx.domain[domain_idx].config = data;
    metrics_analyzer_ctx.domain[domain_idx].aggregate_limit = UINT32_MAX;
    metrics_analyzer_ctx.domain[domain_idx].metrics_count = sub_element_count;
    metrics_analyzer_ctx.domain[domain_idx].metrics = fwk_mm_calloc(
        sub_element_count,
        sizeof(metrics_analyzer_ctx.domain[domain_idx].metrics[0]));

    for (size_t i = 0; i < sub_element_count; ++i) {
        metrics_analyzer_ctx.domain[domain_idx].metrics[i].limit = UINT32_MAX;
        metrics_analyzer_ctx.domain[domain_idx]
            .metrics[i]
            .limit_provider_config =
            &metrics_analyzer_ctx.domain[domain_idx].config->limit_providers[i];
    }

    return FWK_SUCCESS;
}

static int metrics_analyzer_bind_element(fwk_id_t id)
{
    int status = FWK_E_INIT;
    size_t domain_idx = fwk_id_get_element_idx(id);
    struct mod_domain_ctx *domain_ctx =
        &metrics_analyzer_ctx.domain[domain_idx];
    for (size_t i = 0; i < domain_ctx->metrics_count; ++i) {
        struct mod_metric_ctx *metrics_ctx = &domain_ctx->metrics[i];
        status = fwk_module_bind(
            metrics_ctx->limit_provider_config->domain_id,
            metrics_ctx->limit_provider_config->api_id,
            &metrics_ctx->limit_provider_api);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    status = fwk_module_bind(
        domain_ctx->config->limit_consumer.domain_id,
        domain_ctx->config->limit_consumer.api_id,
        &domain_ctx->limit_consumer_api);

    return status;
}

static int metrics_analyzer_bind(fwk_id_t id, unsigned int round)
{
    /* Bind our elements in round 0 */
    if (round == 0 && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return metrics_analyzer_bind_element(id);
    }

    return FWK_SUCCESS;
}

static int metrics_analyzer_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only allow binding to the module */
    if (api == NULL ||
        !fwk_id_is_equal(
            target_id, FWK_ID_MODULE(FWK_MODULE_IDX_METRICS_ANALYZER))) {
        return FWK_E_PARAM;
    }

    if (!fwk_id_is_equal(
            api_id,
            FWK_ID_API(
                FWK_MODULE_IDX_METRICS_ANALYZER,
                MOD_METRICS_ANALYZER_API_IDX_ANALYZE))) {
        return FWK_E_PARAM;
    }

    *api = &analyze_api;
    return FWK_SUCCESS;
}

static int metrics_analyzer_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

/* metrics_analyzer module definition */
const struct fwk_module module_metrics_analyzer = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MOD_METRICS_ANALYZER_API_IDX_COUNT,
    .init = metrics_analyzer_init,
    .element_init = metrics_analyzer_element_init,
    .bind = metrics_analyzer_bind,
    .start = metrics_analyzer_start,
    .process_bind_request = metrics_analyzer_process_bind_request,
};
