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

int amu_mmap_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_amu_mmap_init_zero_cores_fail);
    RUN_TEST(test_amu_mmap_init_success);
    RUN_TEST(test_amu_mmap_element_init_bad_params_fail);
    RUN_TEST(test_amu_mmap_element_init_success);
    RUN_TEST(test_amu_mmap_bind_request_amu_api_bad_params_fail);
    RUN_TEST(test_amu_mmap_bind_request_amu_api_success);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return amu_mmap_test_main();
}
#endif
