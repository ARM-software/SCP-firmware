/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_amu_smcf_drv.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mocksmcf_data.h>

#include UNIT_TEST_SRC

#include <stdio.h>

#define AMU_COUNTER_DATA_SIZE_BYTES 8

struct amu_smcf_drv_element_config core_configs[NUM_OF_CORES];
uint32_t core_amu_data_buf[AMU_DATA_MAX_SZ];

/*!
 * \brief Test smcf get_data api
 *
 * \param monitor_id  Monitor ID
 * \param[out] data_buffer Pointer to the buffer for AMU counter data
 *             writes amu_counter data in the following testable format.
 *             Every single counter is 64 bit and following is the value
 *             for each counter depending on its index.
 *             counter_i.lsb = i + 1
 *             counter_i.msb = i + 1
 * \param[out] tag_buffer Pointer to the buffer for TAG data for the given MLI
 *             sample
 *
 * \retval ::FWK_SUCCESS The request was successfully completed.
 */
static int test_smcf_get_data_api(
    fwk_id_t monitor_id,
    struct mod_smcf_buffer data_buffer,
    struct mod_smcf_buffer tag_buffer)
{
    uintptr_t base_offset;
    unsigned int i, core_idx, smcf_counter_offset_idx;
    uint32_t *data;
    const struct amu_smcf_drv_element_config *core_counters_cfg;

    core_idx = fwk_id_get_element_idx(monitor_id);
    if (data_buffer.ptr == NULL || data_buffer.size == 0 ||
        core_idx >= NUM_OF_CORES)
        return FWK_E_PARAM;

    data = data_buffer.ptr;

    core_counters_cfg = &(amu_smcf_drv_ctx.element_config_table[core_idx]);
    base_offset = core_counters_cfg->counter_offsets[0];
    for (i = 0; i < core_counters_cfg->total_num_of_counters; ++i) {
        smcf_counter_offset_idx =
            (core_counters_cfg->counter_offsets[i] - base_offset) /
            sizeof(uint32_t);
        data[smcf_counter_offset_idx] = i + 1;
        data[smcf_counter_offset_idx + 1] = i + 1;
    }

    return FWK_SUCCESS;
}

static void validate_counter_values(
    const uint64_t *const counter_values,
    int count,
    int start_cntr_idx)
{
    int i;
    uint64_t expected_counter_value;

    for (i = 0; i < count; ++i) {
        uint64_t counter_lsb = start_cntr_idx + i + 1;
        uint64_t counter_msb = start_cntr_idx + i + 1;
        expected_counter_value = (counter_msb << 32) + counter_lsb;
        TEST_ASSERT_EQUAL_UINT64(expected_counter_value, counter_values[i]);
    }
}

void setUp(void)
{
    amu_smcf_drv_ctx.num_of_cores = NUM_OF_CORES;
    amu_smcf_drv_ctx.element_config_table = core_configs;
}

void tearDown(void)
{
}

void test_amu_smcf_drv_init_success(void)
{
    int status;

    memset(&amu_smcf_drv_ctx, 0, sizeof(amu_smcf_drv_ctx));
    fwk_mm_calloc_ExpectAndReturn(
        NUM_OF_CORES,
        sizeof(struct amu_smcf_drv_element_config),
        (void *)core_config_table);
    status = amu_smcf_drv_mod_init(
        FWK_ID_MODULE(FWK_MODULE_IDX_AMU_SMCF_DRV), NUM_OF_CORES, NULL);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_amu_smcf_drv_init_fail_zero_cores(void)
{
    int status;

    memset(&amu_smcf_drv_ctx, 0, sizeof(amu_smcf_drv_ctx));
    status = amu_smcf_drv_mod_init(
        FWK_ID_MODULE(FWK_MODULE_IDX_AMU_SMCF_DRV), 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_EQUAL(amu_smcf_drv_ctx.num_of_cores, 0);
    TEST_ASSERT_NULL(amu_smcf_drv_ctx.element_config_table);
}

void test_amu_smcf_drv_element_init_success(void)
{
    int status;
    unsigned int i;
    fwk_id_t element_id;

    for (i = 0; i < NUM_OF_CORES; ++i) {
        element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_AMU_SMCF_DRV, i);
        fwk_module_is_valid_element_id_ExpectAndReturn(element_id, true);
        fwk_id_get_element_idx_ExpectAndReturn(element_id, i);
        status = amu_smcf_drv_element_init(
            element_id,
            test_element_table[i].sub_element_count,
            test_element_table[i].data);
        TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

        TEST_ASSERT_EQUAL(
            amu_smcf_drv_ctx.element_config_table[i].total_num_of_counters,
            test_element_table[i].sub_element_count);
    }
}

void test_amu_smcf_drv_element_init_fail_null_data(void)
{
    int status;
    unsigned int i;
    fwk_id_t element_id;

    for (i = 0; i < NUM_OF_CORES; ++i) {
        element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_AMU_SMCF_DRV, i);
        fwk_module_is_valid_element_id_ExpectAndReturn(element_id, true);
        status = amu_smcf_drv_element_init(element_id, 0, NULL);
        TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    }
}

void test_amu_smcf_drv_element_init_fail_unaligned_aux_offset(void)
{
    int status;
    fwk_id_t element_id;
    struct amu_smcf_drv_element_config element_config_invalid_aux_offset = {
        .smcf_mli_id =
            FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_SMCF, AMU_COUNTERS_CORE0, 0),
        .counter_offsets =
            (uint32_t[]){
                [COREA_CORE] = 0,
                [COREA_CONST] = sizeof(uint64_t) * COREA_CONST,
                [COREA_INST_RET] = sizeof(uint64_t) * COREA_INST_RET,
                [COREA_MEM_STALL] = sizeof(uint64_t) * COREA_MEM_STALL,
                [COREA_AUX0] =
                    sizeof(uint64_t) * COREA_INST_RET, // wrong offset
                [COREA_AUX1] = sizeof(uint64_t) * COREA_AUX1,
                [COREA_AUX2] = sizeof(uint64_t) * COREA_AUX2,
                [COREA_AUX3] = sizeof(uint64_t) * COREA_AUX3,
                [COREA_AUX4] = sizeof(uint64_t) * COREA_AUX4,
                [COREA_AUX5] = sizeof(uint64_t) * COREA_AUX5,
            },
    };

    element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_AMU_SMCF_DRV, 0);
    fwk_module_is_valid_element_id_ExpectAndReturn(element_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    status = amu_smcf_drv_element_init(
        element_id, NUM_OF_COREA_COUNTERS, &element_config_invalid_aux_offset);
    TEST_ASSERT_EQUAL(FWK_E_ALIGN, status);
}

void test_amu_smcf_drv_start_success(void)
{
    int status;

    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_AMU_SMCF_DRV);
    fwk_module_is_valid_module_id_ExpectAndReturn(module_id, true);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, false);
    fwk_mm_calloc_ExpectAndReturn(
        AMU_DATA_MAX_SZ, sizeof(uint32_t), core_amu_data_buf);
    status = amu_smcf_drv_start(module_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(amu_smcf_drv_ctx.amu_smcf_data_buf.size, AMU_DATA_MAX_SZ);
}

void test_amu_smcf_drv_start_fail_invalid_params(void)
{
    int status;
    fwk_id_t wrong_module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_COUNT);
    fwk_id_is_type_ExpectAndReturn(wrong_module_id, FWK_ID_TYPE_ELEMENT, false);
    fwk_module_is_valid_module_id_ExpectAndReturn(wrong_module_id, false);
    status = amu_smcf_drv_start(wrong_module_id);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_amu_smcf_drv_bind_request_amu_data_api_success(void)
{
    int status;
    fwk_id_t test_client_id = FWK_ID_MODULE(FWK_MODULE_IDX_TEST_CLIENT);
    fwk_id_t api_id =
        FWK_ID_API(FWK_MODULE_IDX_AMU_SMCF_DRV, MOD_AMU_SMCF_DRV_API_IDX_DATA);
    struct amu_api *counter_api = NULL;

    fwk_id_is_equal_ExpectAndReturn(
        test_client_id, FWK_ID_MODULE(FWK_MODULE_IDX_AMU_SMCF_DRV), false);

    status = amu_smcf_drv_process_bind_request(
        test_client_id,
        FWK_ID_MODULE(FWK_MODULE_IDX_AMU_SMCF_DRV),
        api_id,
        (const void **)(&counter_api));
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&amu_smcf_drv_api, counter_api);
}

void test_amu_smcf_drv_bind_request_amu_data_api_fail_invalid_params(void)
{
    int status;
    fwk_id_t test_client_id = FWK_ID_MODULE(FWK_MODULE_IDX_AMU_SMCF_DRV);
    fwk_id_t api_id =
        FWK_ID_API(FWK_MODULE_IDX_AMU_SMCF_DRV, MOD_AMU_SMCF_DRV_API_IDX_DATA);
    struct amu_api *counter_api = NULL;

    fwk_id_is_equal_ExpectAndReturn(
        test_client_id, FWK_ID_MODULE(FWK_MODULE_IDX_AMU_SMCF_DRV), true);

    status = amu_smcf_drv_process_bind_request(
        test_client_id,
        FWK_ID_MODULE(FWK_MODULE_IDX_AMU_SMCF_DRV),
        api_id,
        (const void **)(&counter_api));
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_NULL(counter_api);
}

void test_amu_smcf_drv_get_amu_counter_success(void)
{
    int status;
    unsigned int i, j, counter_set_size;
    struct amu_smcf_drv_element_config *core_ctx;

    struct smcf_data_api data_api = {
        .get_data = test_smcf_get_data_api,
    };
    amu_smcf_drv_ctx.data_api = &data_api;
    uint64_t amu_counter_values[10];

    for (i = 1; i < NUM_OF_CORES; ++i) {
        core_ctx = &(amu_smcf_drv_ctx.element_config_table[i]);
        for (j = 0; j < core_ctx->total_num_of_counters; ++j) {
            unsigned int max_counter_set_sz =
                core_ctx->total_num_of_counters - j;
            for (counter_set_size = 1; counter_set_size < max_counter_set_sz;
                 ++counter_set_size) {
                memset(amu_counter_values, 0, 10 * sizeof(uint64_t));
                fwk_id_t drv_counter_id =
                    FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_AMU_SMCF_DRV, i, j);
                fwk_id_t counter_smcf_mli_id = core_ctx->smcf_mli_id;
                fwk_module_is_valid_element_id_ExpectAndReturn(
                    drv_counter_id, true);
                fwk_module_is_valid_sub_element_id_ExpectAndReturn(
                    drv_counter_id, true);
                fwk_id_get_element_idx_ExpectAndReturn(drv_counter_id, i);
                fwk_id_get_element_idx_ExpectAndReturn(counter_smcf_mli_id, i);
                fwk_id_get_sub_element_idx_ExpectAndReturn(drv_counter_id, j);

                status = amu_smcf_drv_get_counter_value(
                    drv_counter_id, amu_counter_values, counter_set_size);
                TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
                /*
                 * 64 bit test counter value for ith counter: [0xi][0xi]
                 */
                validate_counter_values(
                    amu_counter_values, counter_set_size, j);
            }
        }
    }
}

void test_amu_smcf_drv_get_amu_counter_fail_invalid_param(void)
{
    int status;
    struct smcf_data_api data_api = {
        .get_data = test_smcf_get_data_api,
    };

    amu_smcf_drv_ctx.data_api = &data_api;
    uint64_t amu_counter_values[10];

    /* invalid counter id */
    fwk_id_t invalid_counter_id =
        FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_AMU_SMCF_DRV, 0, 100);
    fwk_module_is_valid_element_id_ExpectAndReturn(invalid_counter_id, true);
    fwk_module_is_valid_sub_element_id_ExpectAndReturn(
        invalid_counter_id, false);

    status = amu_smcf_drv_get_counter_value(
        invalid_counter_id, amu_counter_values, 1);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

int amu_smcf_drv_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_amu_smcf_drv_init_success);
    RUN_TEST(test_amu_smcf_drv_init_fail_zero_cores);
    RUN_TEST(test_amu_smcf_drv_element_init_success);
    RUN_TEST(test_amu_smcf_drv_element_init_fail_null_data);
    RUN_TEST(test_amu_smcf_drv_element_init_fail_unaligned_aux_offset);
    RUN_TEST(test_amu_smcf_drv_start_success);
    RUN_TEST(test_amu_smcf_drv_start_fail_invalid_params);
    RUN_TEST(test_amu_smcf_drv_bind_request_amu_data_api_success);
    RUN_TEST(test_amu_smcf_drv_bind_request_amu_data_api_fail_invalid_params);
    RUN_TEST(test_amu_smcf_drv_get_amu_counter_success);
    RUN_TEST(test_amu_smcf_drv_get_amu_counter_fail_invalid_param);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return amu_smcf_drv_test_main();
}
#endif
