/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Mockmod_metrics_analyzer_extra.h"
#include "config_metrics_analyzer.h"
#include "fwk_module_idx.h"
#include "fwk_status.h"
#include "interface_power_management.h"
#include "mod_metrics_analyzer_extra.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>

#include UNIT_TEST_SRC

#define MOD_METRICS_ANALYZER_ID FWK_ID_MODULE(FWK_MODULE_IDX_METRICS_ANALYZER)
#define MOD_METRICS_ANALYZER_ANALYZE_API_ID \
    FWK_ID_API( \
        FWK_MODULE_IDX_METRICS_ANALYZER, MOD_METRICS_ANALYZER_API_IDX_ANALYZE)

/* Runtime allocated variables */
static struct mod_domain_ctx domains[METRICS_ANALYZER_DOMAIN_IDX_COUNT];
static struct mod_metric_ctx metrics[METRICS_ANALYZER_DOMAIN_IDX_COUNT]
                                    [METRICS_ANALYZER_METRIC_IDX_COUNT];
struct interface_power_management_api limit_api = {
    .get_limit = &get_limit,
    .set_limit = &set_limit,
};

void setUp(void)
{
}

void tearDown(void)
{
    Mockmod_metrics_analyzer_extra_Verify();
    Mockmod_metrics_analyzer_extra_Destroy();
    Mockfwk_module_Destroy();
}

void initialize_module_ctx(void)
{
    metrics_analyzer_ctx.domain = domains;
    metrics_analyzer_ctx.domain_count = FWK_ARRAY_SIZE(domains);
}

void initialize_module_domains(void)
{
    for (size_t i = 0; i < metrics_analyzer_ctx.domain_count; ++i) {
        metrics_analyzer_ctx.domain[i].config = metrics_analyzer_domain[i].data;
        metrics_analyzer_ctx.domain[i].metrics_count =
            metrics_analyzer_domain[i].sub_element_count;
        metrics_analyzer_ctx.domain[i].aggregate_limit = UINT32_MAX;
        metrics_analyzer_ctx.domain[i].metrics = metrics[i];
        for (size_t j = 0; j < metrics_analyzer_ctx.domain[i].metrics_count;
             ++i) {
            metrics_analyzer_ctx.domain[i].metrics[j].limit = UINT32_MAX;
            metrics_analyzer_ctx.domain[i].metrics[j].limit_provider_config =
                &metrics_analyzer_ctx.domain[i].config->limit_providers[j];
        }
    }
}

void initialize_module_domains_apis(void)
{
    for (size_t i = 0; i < metrics_analyzer_ctx.domain_count; ++i) {
        metrics_analyzer_ctx.domain[i].limit_consumer_api = &limit_api;

        for (size_t j = 0; j < metrics_analyzer_ctx.domain[i].metrics_count;
             ++i) {
            metrics_analyzer_ctx.domain[i].metrics[i].limit_provider_api =
                &limit_api;
        }
    }
}

void test_init_invalid_params(void)
{
    size_t domain_count = FWK_ARRAY_SIZE(metrics_analyzer_domain);
    int status = FWK_E_INIT;
    status =
        metrics_analyzer_init(MOD_METRICS_ANALYZER_ID, 0, module_config.data);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    status =
        metrics_analyzer_init(FWK_ID_NONE, domain_count, module_config.data);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_init_intialize_mod_ctx_correctly(void)
{
    size_t domain_count = FWK_ARRAY_SIZE(metrics_analyzer_domain);
    int status = FWK_E_INIT;
    fwk_mm_calloc_ExpectAndReturn(
        domain_count, sizeof(struct mod_domain_ctx), domains);
    status = metrics_analyzer_init(
        MOD_METRICS_ANALYZER_ID, domain_count, module_config.data);

    TEST_ASSERT_EQUAL(domain_count, metrics_analyzer_ctx.domain_count);
    TEST_ASSERT_EQUAL(domains, metrics_analyzer_ctx.domain);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_element_init_invalid_params(void)
{
    int status = FWK_E_INIT;
    initialize_module_ctx();
    /* Impossible value sub element count */
    status = metrics_analyzer_element_init(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_METRICS_ANALYZER, 0),
        0,
        metrics_analyzer_domain[0].data);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    /* Invalid config data */
    status = metrics_analyzer_element_init(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_METRICS_ANALYZER, 0),
        metrics_analyzer_domain[0].sub_element_count,
        NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_element_init_intialize_domains_correctly(void)
{
    int status = FWK_E_INIT;
    size_t element_idx = 0;
    const struct fwk_element *element = &metrics_analyzer_domain[element_idx];
    struct mod_metric_ctx *domain_metrics = metrics[element_idx];
    const struct mod_metrics_analyzer_domain_config *config = element->data;

    initialize_module_ctx();
    fwk_mm_calloc_ExpectAndReturn(
        element->sub_element_count,
        sizeof(struct mod_metric_ctx),
        domain_metrics);
    status = metrics_analyzer_element_init(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_METRICS_ANALYZER, element_idx),
        element->sub_element_count,
        element->data);
    TEST_ASSERT_EQUAL(
        UINT32_MAX, metrics_analyzer_ctx.domain[element_idx].aggregate_limit);
    TEST_ASSERT_EQUAL(
        element->sub_element_count,
        metrics_analyzer_ctx.domain[element_idx].metrics_count);
    TEST_ASSERT_EQUAL(
        element->data, metrics_analyzer_ctx.domain[element_idx].config);
    TEST_ASSERT_EQUAL(
        domain_metrics, metrics_analyzer_ctx.domain[element_idx].metrics);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* Sub elements (metrics) */
    for (size_t i = 0; i < element->sub_element_count; ++i) {
        TEST_ASSERT_EQUAL(UINT32_MAX, domain_metrics[i].limit);
        TEST_ASSERT_EQUAL(
            &config->limit_providers[i],
            domain_metrics[i].limit_provider_config);
    }
}

void test_bind_no_binding(void)
{
    int status = FWK_E_INIT;

    status = metrics_analyzer_bind(MOD_METRICS_ANALYZER_ID, 0);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    status = metrics_analyzer_bind(MOD_METRICS_ANALYZER_ID, 1);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_bind_element_providers_and_consumer(void)
{
    int status = FWK_E_INIT;
    initialize_module_ctx();
    initialize_module_domains();
    size_t domain_idx = 0;
    struct mod_domain_ctx *domain_ctx =
        &metrics_analyzer_ctx.domain[domain_idx];
    for (size_t i = 0; i < domain_ctx->metrics_count; ++i) {
        fwk_module_bind_ExpectAndReturn(
            domain_ctx->metrics[i].limit_provider_config->domain_id,
            domain_ctx->metrics[i].limit_provider_config->api_id,
            &domain_ctx->metrics[i].limit_provider_api,
            FWK_SUCCESS);
    }
    fwk_module_bind_ExpectAndReturn(
        domain_ctx->config->limit_consumer.domain_id,
        domain_ctx->config->limit_consumer.api_id,
        &domain_ctx->limit_consumer_api,
        FWK_SUCCESS);

    status = metrics_analyzer_bind(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_METRICS_ANALYZER, domain_idx), 1);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_process_bind_request_invalid_params(void)
{
    int status = FWK_E_INIT;
    initialize_module_ctx();
    initialize_module_domains();
    const struct mod_metrics_analyzer_analyze_api *api = NULL;
    status = metrics_analyzer_process_bind_request(
        FWK_ID_NONE,
        MOD_METRICS_ANALYZER_ID,
        MOD_METRICS_ANALYZER_ANALYZE_API_ID,
        NULL);

    TEST_ASSERT_EQUAL(NULL, api);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    status = metrics_analyzer_process_bind_request(
        FWK_ID_NONE,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_METRICS_ANALYZER, 0),
        MOD_METRICS_ANALYZER_ANALYZE_API_ID,
        (const void **)&api);

    TEST_ASSERT_EQUAL(NULL, api);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    status = metrics_analyzer_process_bind_request(
        FWK_ID_NONE,
        MOD_METRICS_ANALYZER_ID,
        FWK_ID_ELEMENT(FWK_MODULE_IDX_METRICS_ANALYZER, 0),
        (const void **)&api);

    TEST_ASSERT_EQUAL(NULL, api);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_process_bind_request_correct_api(void)
{
    int status = FWK_E_INIT;
    initialize_module_ctx();
    initialize_module_domains();
    const struct mod_metrics_analyzer_analyze_api *api = NULL;
    status = metrics_analyzer_process_bind_request(
        FWK_ID_NONE,
        MOD_METRICS_ANALYZER_ID,
        MOD_METRICS_ANALYZER_ANALYZE_API_ID,
        (const void **)&api);

    TEST_ASSERT_EQUAL(&analyze_api, api);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_process_start_success(void)
{
    TEST_ASSERT_EQUAL(
        FWK_SUCCESS, metrics_analyzer_start(MOD_METRICS_ANALYZER_ID));
}

void test_collect_domain_limits_invalid_params(void)
{
    int status = FWK_E_INIT;
    status = collect_domain_limits(NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_collect_domain_limits(void)
{
    int status = FWK_E_INIT;
    const struct mod_metrics_analyzer_interactor limit_provider_config[] = {
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(10, 0) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(11, 1) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(12, 2) },
    };
    struct mod_metric_ctx metrics_ctx[] = {
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[0],
            .limit = UINT32_MAX,
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[1],
            .limit = UINT32_MAX,
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[2],
            .limit = UINT32_MAX,
        },
    };
    struct mod_domain_ctx domain_ctx = {
        .metrics_count = 3,
        .metrics = metrics_ctx,
    };

    unsigned int limits[] = { 100, 150, 200 };

    /* Expected calls */
    for (size_t i = 0; i < domain_ctx.metrics_count; ++i) {
        get_limit_ExpectAndReturn(
            limit_provider_config[i].domain_id, NULL, FWK_SUCCESS);
        get_limit_IgnoreArg_power_limit();
        get_limit_ReturnMemThruPtr_power_limit(&limits[i], sizeof(limits[i]));
    }
    get_limit_StopIgnore();

    /* Test */
    status = collect_domain_limits(&domain_ctx);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* Validate */
    for (size_t i = 0; i < domain_ctx.metrics_count; ++i) {
        TEST_ASSERT_EQUAL(limits[i], metrics_ctx[i].limit);
    }
}

void test_collect_domain_limits_no_change_domain(void)
{
    int status = FWK_E_INIT;
    const struct mod_metrics_analyzer_interactor limit_provider_config[] = {
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(10, 0) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(11, 1) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(12, 2) },
    };

    unsigned int limits[] = { 100, 150, 200 };
    size_t min_limit_idx = 0;

    struct mod_metric_ctx metrics_ctx[] = {
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[0],
            .limit = limits[0],
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[1],
            .limit = limits[1],
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[2],
            .limit = limits[2],
        },
    };
    struct mod_domain_ctx domain_ctx = {
        .metrics_count = 3,
        .metrics = metrics_ctx,
        .aggregate_limit = limits[min_limit_idx],
    };

    /* Expected calls */
    for (size_t i = 0; i < domain_ctx.metrics_count; ++i) {
        get_limit_ExpectAndReturn(
            limit_provider_config[i].domain_id, NULL, FWK_SUCCESS);
        get_limit_IgnoreArg_power_limit();
        get_limit_ReturnMemThruPtr_power_limit(&limits[i], sizeof(limits[i]));
    }
    get_limit_StopIgnore();

    /* Test */
    status = collect_domain_limits(&domain_ctx);

    /* Validate */
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    for (size_t i = 0; i < domain_ctx.metrics_count; ++i) {
        TEST_ASSERT_EQUAL(limits[i], metrics_ctx[i].limit);
    }
    /* Domain evaluation still valid */
    TEST_ASSERT_EQUAL(limits[min_limit_idx], domain_ctx.aggregate_limit);
}

void test_evaluate_domain_invalid_params(void)
{
    int status = FWK_E_INIT;
    status = evaluate_domain_aggregate_limit(NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_evaluate_domain_not_required(void)
{
    int status = FWK_E_INIT;
    const struct mod_metrics_analyzer_interactor limit_provider_config[] = {
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(10, 0) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(11, 1) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(12, 2) },
    };

    unsigned int limits[] = { 100, 150, 200 };
    size_t min_limit_idx = 0;

    struct mod_metric_ctx metrics_ctx[] = {
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[0],
            .limit = limits[0],
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[1],
            .limit = limits[1],
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[2],
            .limit = limits[2],
        },
    };
    struct mod_domain_ctx domain_ctx = {
        .metrics_count = 3,
        .metrics = metrics_ctx,
        .aggregate_limit = limits[min_limit_idx],
    };

    /* Test */
    status = evaluate_domain_aggregate_limit(&domain_ctx);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    /* Domain evaluation still valid */
    TEST_ASSERT_EQUAL(limits[min_limit_idx], domain_ctx.aggregate_limit);
}

void test_evaluate_domain_limit_less_than_aggregate_limit(void)
{
    int status = FWK_E_INIT;
    const struct mod_metrics_analyzer_interactor limit_provider_config[] = {
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(10, 0) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(11, 1) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(12, 2) },
    };

    unsigned int limits[] = { 100, 150, 200 };
    size_t min_limit_idx = 0;
    unsigned int lower_limit = 10;

    struct mod_metric_ctx metrics_ctx[] = {
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[0],
            .limit = limits[0],
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[1],
            .limit = limits[1],
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[2],
            .limit = lower_limit,
        },
    };
    struct mod_domain_ctx domain_ctx = {
        .metrics_count = 3,
        .metrics = metrics_ctx,
        .aggregate_limit = limits[min_limit_idx],
    };

    /* Test */
    status = evaluate_domain_aggregate_limit(&domain_ctx);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(lower_limit, domain_ctx.aggregate_limit);
}

void test_evaluate_domain_limit_more_than_aggregate_limit(void)
{
    int status = FWK_E_INIT;
    const struct mod_metrics_analyzer_interactor limit_provider_config[] = {
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(10, 0) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(11, 1) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(12, 2) },
    };

    unsigned int limits[] = { 100, 150, 200 };
    unsigned int greater_limit = 1000;
    size_t min_limit_idx = 0;

    struct mod_metric_ctx metrics_ctx[] = {
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[0],
            .limit = limits[0],
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[1],
            .limit = limits[1],
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[2],
            .limit = greater_limit,
        },
    };
    struct mod_domain_ctx domain_ctx = {
        .metrics_count = 3,
        .metrics = metrics_ctx,
        .aggregate_limit = limits[min_limit_idx],
    };

    /* Test */
    status = evaluate_domain_aggregate_limit(&domain_ctx);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(limits[min_limit_idx], domain_ctx.aggregate_limit);
}

void test_evaluate_domain_limit_lowest_level_changed(void)
{
    int status = FWK_E_INIT;
    const struct mod_metrics_analyzer_interactor limit_provider_config[] = {
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(10, 0) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(11, 1) },
        { .api_id = FWK_ID_NONE, .domain_id = FWK_ID_ELEMENT(12, 2) },
    };

    unsigned int limits[] = { 100, 150, 200 };
    unsigned int new_limit = 1000;
    size_t min_limit_idx = 0;
    size_t new_min_limit_idx = 1;

    struct mod_metric_ctx metrics_ctx[] = {
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[0],
            .limit = new_limit,
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[1],
            .limit = limits[1],
        },
        {
            .limit_provider_api = &limit_api,
            .limit_provider_config = &limit_provider_config[2],
            .limit = limits[2],
        },
    };
    struct mod_domain_ctx domain_ctx = {
        .metrics_count = 3,
        .metrics = metrics_ctx,
        .aggregate_limit = limits[min_limit_idx],
    };

    /* Test */
    status = evaluate_domain_aggregate_limit(&domain_ctx);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(limits[new_min_limit_idx], domain_ctx.aggregate_limit);
}

void test_report_domain_aggregate_limit_api_is_not_set(void)
{
    int status = FWK_E_INIT;

    struct mod_domain_ctx domain_ctx = { 0 };

    /* Test */
    status = report_domain_aggregate_limit(&domain_ctx);
    TEST_ASSERT_EQUAL(FWK_E_PANIC, status);
}

void test_report_domain_aggregate_limit(void)
{
    int status = FWK_E_INIT;

    struct mod_metrics_analyzer_domain_config
        config = { .limit_consumer = {
                       .api_id = FWK_ID_NONE,
                       .domain_id = FWK_ID_ELEMENT(10, 0),
                   }, };
    unsigned int consumer_limit = 100;
    struct mod_domain_ctx domain_ctx = {
        .aggregate_limit = consumer_limit,
        .config = &config,
        .limit_consumer_api = &limit_api,
    };

    /* Expected calls */
    set_limit_ExpectAndReturn(
        domain_ctx.config->limit_consumer.domain_id,
        domain_ctx.aggregate_limit,
        FWK_SUCCESS);

    /* Test */
    status = report_domain_aggregate_limit(&domain_ctx);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_collect_domains_limits_invalid_params(void)
{
    int status = FWK_E_INIT;
    status = collect_domains_limits(NULL, 5);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_evaluate_domains_aggregate_limit_invalid_params(void)
{
    int status = FWK_E_INIT;
    status = evaluate_domains_aggregate_limit(NULL, 5);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_report_domains_aggregate_limit_invalid_params(void)
{
    int status = FWK_E_INIT;
    status = report_domains_aggregate_limit(NULL, 5);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_collect_domains_limits_zero_domains(void)
{
    int status = FWK_E_INIT;
    status = collect_domains_limits(NULL, 0);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_evaluate_domains_aggregate_limit_zero_domains(void)
{
    int status = FWK_E_INIT;
    status = evaluate_domains_aggregate_limit(NULL, 0);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_report_domains_aggregate_limit_zero_domains(void)
{
    int status = FWK_E_INIT;
    status = report_domains_aggregate_limit(NULL, 0);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

int metrics_analyzer_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init_invalid_params);
    RUN_TEST(test_init_intialize_mod_ctx_correctly);
    RUN_TEST(test_element_init_invalid_params);
    RUN_TEST(test_element_init_intialize_domains_correctly);
    RUN_TEST(test_bind_no_binding);
    RUN_TEST(test_bind_element_providers_and_consumer);
    RUN_TEST(test_process_bind_request_invalid_params);
    RUN_TEST(test_process_bind_request_correct_api);
    RUN_TEST(test_process_start_success);
    RUN_TEST(test_collect_domain_limits_invalid_params);
    RUN_TEST(test_collect_domain_limits);
    RUN_TEST(test_collect_domain_limits_no_change_domain);
    RUN_TEST(test_evaluate_domain_invalid_params);
    RUN_TEST(test_evaluate_domain_not_required);
    RUN_TEST(test_evaluate_domain_limit_less_than_aggregate_limit);
    RUN_TEST(test_evaluate_domain_limit_more_than_aggregate_limit);
    RUN_TEST(test_evaluate_domain_limit_lowest_level_changed);
    RUN_TEST(test_report_domain_aggregate_limit_api_is_not_set);
    RUN_TEST(test_report_domain_aggregate_limit);
    RUN_TEST(test_collect_domains_limits_invalid_params);
    RUN_TEST(test_evaluate_domains_aggregate_limit_invalid_params);
    RUN_TEST(test_report_domains_aggregate_limit_invalid_params);
    RUN_TEST(test_collect_domains_limits_zero_domains);
    RUN_TEST(test_evaluate_domains_aggregate_limit_zero_domains);
    RUN_TEST(test_report_domains_aggregate_limit_zero_domains);

    return UNITY_END();
}

int main(void)
{
    return metrics_analyzer_test_main();
}
