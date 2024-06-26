/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Unit test for mod_perf_controller.c
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockmod_perf_controller_extra.h>
#include <config_mod_perf_controller.h>
#include <internal/perf_controller.h>

#include <mod_perf_controller.h>

#include <fwk_module_idx.h>

#include <stdlib.h>

#include UNIT_TEST_SRC

static struct mod_perf_controller_cluster_ctx
    test_cluster_ctx_table[TEST_CLUSTER_COUNT];
static struct mod_perf_controller_core_ctx
    test_core_ctx_table[TEST_CLUSTER_COUNT][MAX_CORE_PER_CLUSTER];

static struct mod_perf_controller_drv_api perf_driver_api = {
    .set_performance_level = driver_set_performance_level,
};

static struct mod_perf_controller_power_model_api power_model_api = {
    .power_to_performance = power_to_performance,
};

void setUp(void)
{
    unsigned int cluster_idx;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;

    perf_controller_ctx.cluster_ctx_table = test_cluster_ctx_table;
    perf_controller_ctx.cluster_count = TEST_CLUSTER_COUNT;

    for (cluster_idx = 0U; cluster_idx < TEST_CLUSTER_COUNT; cluster_idx++) {
        cluster_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx];
        cluster_ctx->core_ctx_table = test_core_ctx_table[cluster_idx];
        cluster_ctx->perf_driver_api = &perf_driver_api;
        cluster_ctx->power_model_api = &power_model_api;
        cluster_ctx->config = (struct mod_perf_controller_cluster_config *)
                                  cluster_config[cluster_idx]
                                      .data;
        cluster_ctx->core_count = cluster_config[cluster_idx].sub_element_count;
    }

    internal_api.get_cores_min_power_limit = get_cores_min_power_limit_stub;
    internal_api.cluster_apply_performance_granted =
        cluster_apply_performance_granted_stub;
}

void tearDown(void)
{
    Mockmod_perf_controller_extra_Verify();
    Mockmod_perf_controller_extra_Destroy();
}

/*!
 * \brief Helper function to compare two values.
 *
 * \details returns -1 when a < b,
 *          returns 1 when a >b,
 *          return 0 when a = b.
 */
int helper_comp(const void *a, const void *b)
{
    return (*(int *)a > *(int *)b) - (*(int *)a < *(int *)b);
}

void test_set_performance_level_within_limits(void)
{
    int status;
    unsigned int cluster_idx;
    fwk_id_t cluster_id;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;
    uintptr_t cookie;
    uint32_t performance_level;

    for (cluster_idx = 0U; cluster_idx < TEST_CLUSTER_COUNT; cluster_idx++) {
        cluster_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PERF_CONTROLLER, cluster_idx);
        cluster_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx];

        performance_level = 10U;
        cluster_ctx->performance_limit = performance_level;

        cookie = 15U;
        driver_set_performance_level_ExpectAndReturn(
            cluster_ctx->config->performance_driver_id,
            cookie,
            performance_level,
            FWK_SUCCESS);

        status = mod_perf_controller_set_performance_level(
            cluster_id, cookie, performance_level);

        TEST_ASSERT_EQUAL(
            cluster_ctx->performance_request_details.level, performance_level);
        TEST_ASSERT_EQUAL(cluster_ctx->performance_request_details.cookie, 0U);
        TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    }
}

void test_set_performance_level_out_of_limits(void)
{
    int status;
    unsigned int cluster_idx;
    fwk_id_t cluster_id;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;
    uintptr_t cookie;
    uint32_t performance_level;

    for (cluster_idx = 0U; cluster_idx < TEST_CLUSTER_COUNT; cluster_idx++) {
        cluster_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PERF_CONTROLLER, cluster_idx);
        cluster_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx];

        cluster_ctx->performance_limit = 10U;
        performance_level = cluster_ctx->performance_limit + 1U;

        cookie = 1U;
        status = mod_perf_controller_set_performance_level(
            cluster_id, cookie, performance_level);

        TEST_ASSERT_EQUAL(
            cluster_ctx->performance_request_details.level, performance_level);
        TEST_ASSERT_EQUAL(
            cluster_ctx->performance_request_details.cookie, cookie);
        TEST_ASSERT_EQUAL(status, FWK_PENDING);
    }
}

void test_set_limit_success(void)
{
    int status;
    fwk_id_t core_id;
    struct mod_perf_controller_core_ctx *core_ctx;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;
    unsigned int cluster_idx = TEST_CLUSTER_COUNT - 1U;
    unsigned int core_idx = MAX_CORE_PER_CLUSTER - 1U;
    uint32_t power_limit;

    for (cluster_idx = 0U; cluster_idx < TEST_CLUSTER_COUNT - 1U;
         cluster_idx++) {
        cluster_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx];
        for (core_idx = 0U; core_idx < cluster_ctx->core_count; core_idx++) {
            core_id = FWK_ID_SUB_ELEMENT(
                FWK_MODULE_IDX_PERF_CONTROLLER, cluster_idx, core_idx);
            core_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx]
                            .core_ctx_table[core_idx];

            power_limit = 20U;
            status = mod_perf_controller_set_limit(core_id, power_limit);

            TEST_ASSERT_EQUAL(core_ctx->power_limit, power_limit);
            TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
        }
    }
}

void test_get_cores_min_power_limit(void)
{
    uint32_t min_power_limit;
    unsigned int core_idx;
    unsigned int cluster_idx;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;
    uint32_t core_power_limit_test_values[MAX_CORE_PER_CLUSTER] = {
        100U, 300U, 200U, 10U
    };

    for (cluster_idx = 0U; cluster_idx < TEST_CLUSTER_COUNT - 1U;
         cluster_idx++) {
        cluster_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx];

        for (core_idx = 0u; core_idx < cluster_ctx->core_count; core_idx++) {
            cluster_ctx->core_ctx_table[core_idx].power_limit =
                core_power_limit_test_values[core_idx];
        }

        min_power_limit = get_cores_min_power_limit(cluster_ctx);

        /*
            Using sorting to determine the minimum. qsort is used as it is a
            standard function that would make the test easier. The heavy lifting
            still needs to be done on the implementation side.
        */

        qsort(
            core_power_limit_test_values,
            cluster_ctx->core_count,
            sizeof(core_power_limit_test_values[0]),
            helper_comp);

        TEST_ASSERT_EQUAL(core_power_limit_test_values[0], min_power_limit);
    }
}

void test_controller_apply_performance_granted_within_limits(void)
{
    int status;
    unsigned int cluster_idx;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;
    uint32_t min_power_limit;
    uint32_t performance_limit;
    uint32_t *requested_performance;
    uintptr_t *cookie;

    for (cluster_idx = 0U; cluster_idx < TEST_CLUSTER_COUNT - 1U;
         cluster_idx++) {
        cluster_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx];
        requested_performance = &cluster_ctx->performance_request_details.level;
        cookie = &cluster_ctx->performance_request_details.cookie;

        min_power_limit = 500U;
        performance_limit = 700U;

        get_cores_min_power_limit_stub_ExpectAndReturn(
            cluster_ctx, min_power_limit);

        power_to_performance_ExpectAndReturn(
            cluster_ctx->config->power_model_id,
            min_power_limit,
            NULL,
            FWK_SUCCESS);

        power_to_performance_IgnoreArg_performance_level();

        power_to_performance_ReturnThruPtr_performance_level(
            &performance_limit);

        *requested_performance = performance_limit;
        *cookie = 2U;

        driver_set_performance_level_ExpectAndReturn(
            cluster_ctx->config->performance_driver_id,
            *cookie,
            *requested_performance,
            FWK_SUCCESS);

        status = cluster_apply_performance_granted(cluster_ctx);

        TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    }
}

void test_controller_apply_performance_granted_out_of_limits(void)
{
    int status;
    unsigned int cluster_idx;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;
    uint32_t min_power_limit;
    uint32_t performance_limit;
    uint32_t *requested_performance;
    uintptr_t *cookie;

    for (cluster_idx = 0U; cluster_idx < TEST_CLUSTER_COUNT - 1U;
         cluster_idx++) {
        cluster_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx];
        requested_performance = &cluster_ctx->performance_request_details.level;
        cookie = &cluster_ctx->performance_request_details.cookie;

        min_power_limit = 800U;
        performance_limit = 991U;

        get_cores_min_power_limit_stub_ExpectAndReturn(
            cluster_ctx, min_power_limit);

        power_to_performance_ExpectAndReturn(
            cluster_ctx->config->power_model_id,
            min_power_limit,
            NULL,
            FWK_SUCCESS);

        power_to_performance_IgnoreArg_performance_level();

        power_to_performance_ReturnThruPtr_performance_level(
            &performance_limit);

        *requested_performance = performance_limit + 1U;
        *cookie = 3U;

        driver_set_performance_level_ExpectAndReturn(
            cluster_ctx->config->performance_driver_id,
            0U, /* No cookie */
            performance_limit,
            FWK_SUCCESS);

        status = cluster_apply_performance_granted(cluster_ctx);

        TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    }
}

void test_controller_apply_performance_granted_success(void)
{
    int status;
    unsigned int cluster_idx;
    struct mod_perf_controller_cluster_ctx *cluster_ctx;

    for (cluster_idx = 0U; cluster_idx < TEST_CLUSTER_COUNT; cluster_idx++) {
        cluster_ctx = &perf_controller_ctx.cluster_ctx_table[cluster_idx];
        cluster_apply_performance_granted_stub_ExpectAndReturn(
            cluster_ctx, FWK_SUCCESS);
    }

    status = mod_perf_controller_apply_performance_granted();

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int perf_controller_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_set_performance_level_within_limits);
    RUN_TEST(test_set_performance_level_out_of_limits);
    RUN_TEST(test_set_limit_success);
    RUN_TEST(test_get_cores_min_power_limit);
    RUN_TEST(test_controller_apply_performance_granted_within_limits);
    RUN_TEST(test_controller_apply_performance_granted_out_of_limits);
    RUN_TEST(test_controller_apply_performance_granted_success);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return perf_controller_test_main();
}
#endif
