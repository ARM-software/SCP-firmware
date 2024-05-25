/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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

    return UNITY_END();
}

int main(void)
{
    return metrics_analyzer_test_main();
}
