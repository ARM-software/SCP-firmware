/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <fwk_arch.h>

#ifdef TEST_ON_TARGET
#    include <fwk_id.h>
#    include <fwk_module.h>
#else
#    include <Mockfwk_id.h>
#    include <Mockfwk_mm.h>
#    include <Mockfwk_module.h>

#    include <internal/Mockfwk_core_internal.h>
#endif

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC

#include <config_amu_mmap.h>

struct mod_core_amu_counters core[CORE_COUNT];

void setUp(void)
{
    amu_mmap.core_count = CORE_COUNT;
    amu_mmap.core = core;

    for (unsigned int i = 0; i < CORE_COUNT; ++i) {
        core[i].core_config =
            (struct mod_core_element_config *)element_table[i].data;
        core[i].num_counters = element_table[i].sub_element_count;
    }
}

void tearDown(void)
{
    Mockfwk_id_Destroy();
}

void test_amu_mmap_init_zero_cores_fail(void)
{
    int status = FWK_E_PANIC;
    memset(&amu_mmap, 0, sizeof(amu_mmap));
    status = amu_mmap_init(FWK_ID_MODULE(FWK_MODULE_IDX_AMU_MMAP), 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_EQUAL(0, amu_mmap.core_count);
    TEST_ASSERT_NULL(amu_mmap.core);
}

void test_amu_mmap_init_success(void)
{
    int status = FWK_E_PANIC;
    memset(&amu_mmap, 0, sizeof(amu_mmap));
    fwk_mm_calloc_ExpectAndReturn(
        CORE_COUNT, sizeof(struct mod_core_amu_counters *), core);

    status =
        amu_mmap_init(FWK_ID_MODULE(FWK_MODULE_IDX_AMU_MMAP), CORE_COUNT, NULL);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(CORE_COUNT, amu_mmap.core_count);
    TEST_ASSERT_EQUAL_PTR(core, amu_mmap.core);
}

void test_amu_mmap_element_init_bad_params_fail(void)
{
    int status = FWK_E_PANIC;
    fwk_id_t element_id;

    fwk_module_is_valid_element_id_ExpectAnyArgsAndReturn(false);
    status = amu_mmap_element_init(element_id, 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* data is NULL*/
    fwk_module_is_valid_element_id_ExpectAnyArgsAndReturn(true);
    status = amu_mmap_element_init(element_id, 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_amu_mmap_element_init_success(void)
{
    int status = FWK_E_PANIC;
    fwk_id_t element_id;
    for (unsigned int i = 0; i < FWK_ARRAY_SIZE(element_table); ++i) {
        element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_AMU_MMAP, i);
        fwk_module_is_valid_element_id_ExpectAndReturn(element_id, true);
        fwk_id_get_element_idx_ExpectAndReturn(element_id, i);

        status = amu_mmap_element_init(
            element_id,
            element_table[i].sub_element_count,
            element_table[i].data);

        TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
        TEST_ASSERT_EQUAL_PTR(
            element_table[i].data, amu_mmap.core[i].core_config);
        TEST_ASSERT_EQUAL(
            element_table[i].sub_element_count, amu_mmap.core[i].num_counters);
    }
}

void test_amu_mmap_bind_request_amu_api_bad_params_fail(void)
{
    int status = FWK_E_PANIC;
    fwk_id_t source_id = FWK_ID_MODULE(FWK_MODULE_IDX_FAKE_CLIENT);
    fwk_id_t target_id = FWK_ID_MODULE(FWK_MODULE_IDX_AMU_MMAP);
    struct mod_amu_api *api = NULL;
    fwk_id_t api_id =
        FWK_ID_API(FWK_MODULE_IDX_AMU_MMAP, MOD_AMU_MMAP_API_IDX_AMU);

    /* Bad target ID */
    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);

    status = amu_mmap_process_bind_request(
        source_id, FWK_ID_NONE, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_EQUAL(NULL, api);

    /* Bad API ID*/
    fwk_id_is_equal_ExpectAndReturn(target_id, target_id, true);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);

    status = amu_mmap_process_bind_request(
        source_id, target_id, FWK_ID_NONE, (const void **)&api);

    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);
    TEST_ASSERT_EQUAL(NULL, api);
}

void test_amu_mmap_bind_request_amu_api_success(void)
{
    int status = FWK_E_PANIC;
    fwk_id_t source_id = FWK_ID_MODULE(FWK_MODULE_IDX_FAKE_CLIENT);
    fwk_id_t target_id = FWK_ID_MODULE(FWK_MODULE_IDX_AMU_MMAP);
    struct mod_amu_api *api = NULL;
    fwk_id_t api_id =
        FWK_ID_API(FWK_MODULE_IDX_AMU_MMAP, MOD_AMU_MMAP_API_IDX_AMU);

    fwk_id_is_equal_ExpectAndReturn(target_id, target_id, true);
    fwk_id_is_equal_ExpectAndReturn(api_id, api_id, true);

    status = amu_mmap_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&amu_api, api);
}

void test_amu_mmap_get_counters_bad_params_fail(void)
{
    int status = FWK_E_PANIC;
    fwk_id_t counters_id;
    struct amu_api *api = &amu_api;
    uint64_t amu_value[4] = { 0 };
    size_t amu_count = 4;

    /* Bad ID*/
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(false);
    status = api->get_counters(counters_id, amu_value, amu_count);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Bad buffer*/
    counters_id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_AMU_MMAP, CORE0_IDX, NUM_OF_COREA_COUNTERS);
    fwk_module_is_valid_sub_element_id_ExpectAndReturn(counters_id, true);
    status = api->get_counters(counters_id, NULL, amu_count);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_amu_mmap_get_counters_count_exceeds_available(void)
{
    fwk_id_t counters_id;
    int status = FWK_E_PANIC;
    struct amu_api *api = &amu_api;

    for (unsigned int i = 0; i < CORE_COUNT; ++i) {
        size_t amu_count = 2;
        uint64_t amu_value[amu_count];
        counters_id = FWK_ID_SUB_ELEMENT(
            FWK_MODULE_IDX_AMU_MMAP, i, element_table[i].sub_element_count - 1);
        fwk_module_is_valid_sub_element_id_ExpectAndReturn(counters_id, true);

        status = api->get_counters(counters_id, amu_value, amu_count);

        TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
    }
}

void test_amu_mmap_get_counters_success(void)
{
    int status = FWK_E_PANIC;
    struct amu_api *api = &amu_api;
    uint64_t test_value = 0xDEADBEEFC0FFEE00;

    /* Fill the amu test data */
    for (unsigned int i = 0; i < CORE_COUNT; ++i) {
        for (unsigned int j = 0; j < element_table[i].sub_element_count; ++j) {
            amu_counters[i][j] = test_value++;
        }
    }

    for (size_t core_idx = 0; core_idx < CORE_COUNT; ++core_idx) {
        size_t core_num_counters = element_table[core_idx].sub_element_count;
        for (size_t i = 0; i < core_num_counters; ++i) {
            for (size_t n = 1; n < core_num_counters - i; ++n) {
                uint64_t amu_value_buff[n];
                fwk_id_t start_counter_id =
                    FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_AMU_MMAP, core_idx, i);
                fwk_module_is_valid_sub_element_id_ExpectAndReturn(
                    start_counter_id, true);
                status = api->get_counters(start_counter_id, amu_value_buff, n);
                TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
                TEST_ASSERT_EQUAL_HEX64_ARRAY(
                    &amu_counters[core_idx][i], amu_value_buff, n);
            }
        }
    }
}

int amu_mmap_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_amu_mmap_init_zero_cores_fail);
    RUN_TEST(test_amu_mmap_init_success);
    RUN_TEST(test_amu_mmap_element_init_bad_params_fail);
    RUN_TEST(test_amu_mmap_element_init_success);
    RUN_TEST(test_amu_mmap_bind_request_amu_api_bad_params_fail);
    RUN_TEST(test_amu_mmap_bind_request_amu_api_success);
    RUN_TEST(test_amu_mmap_get_counters_bad_params_fail);
    RUN_TEST(test_amu_mmap_get_counters_count_exceeds_available);
    RUN_TEST(test_amu_mmap_get_counters_success);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return amu_mmap_test_main();
}
#endif
