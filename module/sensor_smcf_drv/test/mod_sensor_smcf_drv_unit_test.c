/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_sensor_smcf_drv.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>

#include UNIT_TEST_SRC

#define SENSOR_SMCF_DRV_TEST_TEMP_VALUE_IN_CELCIUS 40

struct sensor_smcf_drv_element_config configs[SENSOR_DEVICE_CNT];
uint32_t device_sensor_data_buf[SENSOR_DATA_MAX_SZ_IN_WORDS];

/*!
 * \brief Test smcf get_data api
 *
 * \param device_idx device index
 * \param[out] buff Pointer to the buffer for generated sensor sample value(s)
 *             in the following testable format
 *             (SENSOR_SMCF_DRV_TEST_TEMP_VALUE_IN_CELCIUS + (device_idx << 1) +
 *              sample_idx)
 * \param buff_sz size of the buffer
 */

static void generate_sensor_samples(
    unsigned int device_idx,
    uint32_t *const buff,
    size_t buff_sz)
{
    unsigned int i;
    for (i = 0; i < buff_sz; ++i) {
        buff[i] =
            SENSOR_SMCF_DRV_TEST_TEMP_VALUE_IN_CELCIUS + i + (device_idx << 1);
    }
}

/*!
 * \brief Test smcf get_data api
 *
 * \param monitor_id  Monitor ID
 * \param[out] data_buffer Pointer to the buffer for sensor data
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
    unsigned int device_idx;

    device_idx =
        fwk_id_get_element_idx(monitor_id) % sensor_smcf_drv_ctx.num_of_devices;
    if (data_buffer.ptr == NULL || data_buffer.size == 0 ||
        device_idx >= SENSOR_DEVICE_CNT)
        return FWK_E_PARAM;

    generate_sensor_samples(device_idx, data_buffer.ptr, data_buffer.size);

    return FWK_SUCCESS;
}

static void validate_multiple_sensor_data_samples(
    const uint32_t *const sensor_sample_values,
    size_t sample_size,
    unsigned int device_idx)
{
    size_t sample_count = sample_size / sizeof(uint32_t);
    uint32_t expected_sample_buff[sample_count];
    generate_sensor_samples(device_idx, expected_sample_buff, sample_count);
    TEST_ASSERT_EQUAL_UINT32_ARRAY(
        expected_sample_buff, sensor_sample_values, sample_count);
}

void setUp(void)
{
    sensor_smcf_drv_ctx.num_of_devices = SENSOR_DEVICE_CNT;
    sensor_smcf_drv_ctx.element_config_table = configs;
    memcpy(
        configs,
        device_config_table,
        SENSOR_DEVICE_CNT * sizeof(struct sensor_smcf_drv_element_config));
}

void tearDown(void)
{
}

void test_sensor_smcf_drv_init_success(void)
{
    int status;

    memset(&sensor_smcf_drv_ctx, 0, sizeof(struct mod_sensor_smcf_drv_ctx));
    fwk_mm_calloc_ExpectAndReturn(
        SENSOR_DEVICE_CNT,
        sizeof(struct sensor_smcf_drv_element_config),
        (void *)configs);
    status = sensor_smcf_drv_mod_init(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV), SENSOR_DEVICE_CNT, NULL);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(sensor_smcf_drv_ctx.num_of_devices, SENSOR_DEVICE_CNT);
    TEST_ASSERT_EQUAL_PTR(configs, sensor_smcf_drv_ctx.element_config_table);
}

void test_sensor_smcf_drv_init_fail_zero_devices(void)
{
    int status;
    memset(&sensor_smcf_drv_ctx, 0, sizeof(struct mod_sensor_smcf_drv_ctx));
    status = sensor_smcf_drv_mod_init(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV), 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_EQUAL(sensor_smcf_drv_ctx.num_of_devices, 0);
    TEST_ASSERT_NULL(sensor_smcf_drv_ctx.element_config_table);
}

void test_sensor_smcf_drv_element_init_success(void)
{
    int status;
    unsigned int i;
    fwk_id_t element_id;

    for (i = 0; i < SENSOR_DEVICE_CNT; ++i) {
        element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR_SMCF_DRV, i);
        fwk_module_is_valid_element_id_ExpectAndReturn(element_id, true);
        fwk_id_get_element_idx_ExpectAndReturn(element_id, i);
        status = sensor_smcf_drv_element_init(
            element_id, 0, &device_config_table[i]);
        TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    }
}

void test_sensor_smcf_drv_element_init_fail_null_data(void)
{
    int status;
    unsigned int i;
    fwk_id_t element_id;

    for (i = 0; i < SENSOR_DEVICE_CNT; ++i) {
        element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR_SMCF_DRV, i);
        fwk_module_is_valid_element_id_ExpectAndReturn(element_id, true);
        status = sensor_smcf_drv_element_init(element_id, 0, NULL);
        TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    }
}

void test_sensor_smcf_drv_bind_request_sensor_data_api(void)
{
    int status;
    fwk_id_t test_client_id = FWK_ID_MODULE(FWK_MODULE_IDX_TEST_CLIENT);
    fwk_id_t api_id = FWK_ID_API(
        FWK_MODULE_IDX_SENSOR_SMCF_DRV, MOD_SENSOR_SMCF_DRV_API_IDX_GET_VALUE);
    struct mod_sensor_driver_api *sensor_data_api = NULL;

    fwk_id_is_equal_ExpectAndReturn(
        test_client_id, FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV), false);
    fwk_id_get_api_idx_ExpectAnyArgsAndReturn(
        MOD_SENSOR_SMCF_DRV_API_IDX_GET_VALUE);
    status = sensor_smcf_drv_process_bind_request(
        test_client_id,
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV),
        api_id,
        (const void **)(&sensor_data_api));
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&sensor_smcf_drv_get_value_api, sensor_data_api);
}

void test_sensor_smcf_drv_bind_request_sensor_sample_data_api(void)
{
    int status;
    fwk_id_t test_client_id = FWK_ID_MODULE(FWK_MODULE_IDX_TEST_CLIENT);
    fwk_id_t api_id = FWK_ID_API(
        FWK_MODULE_IDX_SENSOR_SMCF_DRV,
        MOD_SENSOR_SMCF_DRV_API_IDX_GET_MULTIPLE_SAMPLES);
    struct mod_sensor_smcf_drv_multiple_samples_api *sensor_data_samples_api =
        NULL;

    fwk_id_is_equal_ExpectAndReturn(
        test_client_id, FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV), false);
    fwk_id_get_api_idx_ExpectAnyArgsAndReturn(
        MOD_SENSOR_SMCF_DRV_API_IDX_GET_MULTIPLE_SAMPLES);
    status = sensor_smcf_drv_process_bind_request(
        test_client_id,
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV),
        api_id,
        (const void **)(&sensor_data_samples_api));
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        &sensor_smcf_drv_get_samples_api, sensor_data_samples_api);
}

void test_sensor_smcf_drv_bind_request_sensor_data_api_fail_invalid_params(void)
{
    int status;
    fwk_id_t test_client_id = FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV);
    fwk_id_t api_id = FWK_ID_API(
        FWK_MODULE_IDX_SENSOR_SMCF_DRV, MOD_SENSOR_SMCF_DRV_API_IDX_GET_VALUE);
    struct mod_sensor_driver_api *sensor_data_api = NULL;

    fwk_id_is_equal_ExpectAndReturn(
        test_client_id, FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV), true);

    status = sensor_smcf_drv_process_bind_request(
        test_client_id,
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV),
        api_id,
        (const void **)(&sensor_data_api));
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
    TEST_ASSERT_EQUAL(NULL, sensor_data_api);
}

void test_sensor_smcf_drv_start_success(void)
{
    int status;

    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SENSOR_SMCF_DRV);
    fwk_module_is_valid_module_id_ExpectAndReturn(module_id, true);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, false);
    fwk_mm_calloc_ExpectAndReturn(
        SENSOR_DATA_MAX_SZ_IN_WORDS, sizeof(uint32_t), device_sensor_data_buf);
    status = sensor_smcf_drv_start(module_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(
        sensor_smcf_drv_ctx.sensor_smcf_data_buf.size,
        SENSOR_DATA_MAX_SZ_IN_WORDS);
}

void test_sensor_smcf_drv_start_fail_invalid_params(void)
{
    int status;
    fwk_id_t wrong_module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_COUNT);
    fwk_id_is_type_ExpectAndReturn(wrong_module_id, FWK_ID_TYPE_ELEMENT, false);
    fwk_module_is_valid_module_id_ExpectAndReturn(wrong_module_id, false);
    status = sensor_smcf_drv_start(wrong_module_id);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_sensor_smcf_drv_get_sensor_single_value_success(void)
{
    int status;
    unsigned int i;
    uint64_t sensor_mli_index, sensor_value, expected_sensor_value;
    struct sensor_smcf_drv_element_config *device_ctx;

    struct smcf_data_api data_api = {
        .get_data = test_smcf_get_data_api,
    };
    sensor_smcf_drv_ctx.data_api = &data_api;

    /*
     * Assumption: Sensor MLI indices start with num_of_devices + <sensor_idx>
     * F.E MLI_IDX(sensor_0(device0)) = num_of_devices
     *     MLI_IDX(sensor_1(device1)) = num_of_devices + 1
     */
    for (i = 0; i < SENSOR_DEVICE_CNT; ++i) {
        device_ctx = &(sensor_smcf_drv_ctx.element_config_table[i]);
        fwk_id_t drv_sensor_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR_SMCF_DRV, i);
        sensor_mli_index = i + sensor_smcf_drv_ctx.num_of_devices;
        fwk_id_t sensor_smcf_id = device_ctx->smcf_mli_id;
        fwk_module_is_valid_element_id_ExpectAndReturn(drv_sensor_id, true);
        fwk_id_get_element_idx_ExpectAndReturn(drv_sensor_id, i);
        fwk_id_get_element_idx_ExpectAndReturn(
            sensor_smcf_id, sensor_mli_index);

        status = sensor_smcf_drv_get_sensor_single_value(
            drv_sensor_id, &sensor_value);
        TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

        /*
         * 64 bit test sensor value for device(i):
         * 32 bit msb: SENSOR_SMCF_DRV_TEST_TEMP_VALUE_IN_CELCIUS + (2 * i)
         * 32 bit lsb: SENSOR_SMCF_DRV_TEST_TEMP_VALUE_IN_CELCIUS + (2 * i) + 1
         */
        uint64_t sensor_value_lsb =
            SENSOR_SMCF_DRV_TEST_TEMP_VALUE_IN_CELCIUS + (i << 1);
        uint64_t sensor_value_msb = sensor_value_lsb + 1;
        expected_sensor_value = (sensor_value_msb << 32) + sensor_value_lsb;
        TEST_ASSERT_EQUAL_UINT64(sensor_value, expected_sensor_value);
    }
}

void test_sensor_smcf_drv_get_sensor_single_value_fail_invalid_param(void)
{
    int status;
    uint64_t sensor_value;

    struct smcf_data_api data_api = {
        .get_data = test_smcf_get_data_api,
    };

    sensor_smcf_drv_ctx.data_api = &data_api;

    fwk_id_t valid_sensor_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR_SMCF_DRV, 0);
    fwk_id_t invalid_sensor_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR_SMCF_DRV, 5);

    /* invalid sensor id */
    fwk_module_is_valid_element_id_ExpectAndReturn(invalid_sensor_id, false);
    status = sensor_smcf_drv_get_sensor_single_value(
        invalid_sensor_id, &sensor_value);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* NULL sensor value */
    status = FWK_SUCCESS;
    fwk_module_is_valid_element_id_ExpectAndReturn(valid_sensor_id, true);
    status = sensor_smcf_drv_get_sensor_single_value(valid_sensor_id, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_sensor_smcf_drv_get_sensor_data_sample_success(void)
{
    int status;
    unsigned int i, sensor_set_size;
    struct sensor_smcf_drv_element_config *device_ctx;

    struct smcf_data_api data_api = {
        .get_data = test_smcf_get_data_api,
    };
    sensor_smcf_drv_ctx.data_api = &data_api;
    size_t sensor_data_values_sz =
        sensor_smcf_drv_ctx.sensor_smcf_data_buf.size;
    uint32_t sensor_sample_values[sensor_data_values_sz];

    for (i = 0; i < SENSOR_DEVICE_CNT; ++i) {
        device_ctx = &(sensor_smcf_drv_ctx.element_config_table[i]);
        unsigned int max_sensor_set_sz = (device_ctx->max_samples_size);
        for (sensor_set_size = sizeof(uint32_t);
             sensor_set_size < max_sensor_set_sz;
             sensor_set_size += sizeof(uint32_t)) {
            fwk_id_t drv_sensor_id =
                FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR_SMCF_DRV, i);
            fwk_id_t sensor_smcf_id = device_ctx->smcf_mli_id;
            fwk_module_is_valid_element_id_ExpectAndReturn(drv_sensor_id, true);
            fwk_id_get_element_idx_ExpectAndReturn(drv_sensor_id, i);
            fwk_id_get_element_idx_ExpectAndReturn(sensor_smcf_id, i);

            memset(sensor_sample_values, 0, sensor_data_values_sz);
            status = sensor_smcf_drv_get_sensor_multiple_samples(
                drv_sensor_id, sensor_sample_values, sensor_set_size);
            TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
            validate_multiple_sensor_data_samples(
                sensor_sample_values, sensor_set_size, i);
        }
    }
}

void test_sensor_smcf_drv_get_sensor_data_sample_fail_invalid_param(void)
{
    int status;

    struct smcf_data_api data_api = {
        .get_data = test_smcf_get_data_api,
    };

    sensor_smcf_drv_ctx.data_api = &data_api;
    uint32_t sensor_sample_values[8];
    fwk_id_t valid_sensor_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR_SMCF_DRV, 0);
    fwk_id_t invalid_sensor_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR_SMCF_DRV, 5);

    /* invalid sensor id */
    status = FWK_SUCCESS;
    fwk_module_is_valid_element_id_ExpectAndReturn(invalid_sensor_id, false);

    status = sensor_smcf_drv_get_sensor_multiple_samples(
        invalid_sensor_id, sensor_sample_values, 2);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* NULL sample data buffer */
    status = FWK_SUCCESS;
    fwk_module_is_valid_element_id_ExpectAndReturn(valid_sensor_id, true);

    status =
        sensor_smcf_drv_get_sensor_multiple_samples(valid_sensor_id, NULL, 2);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Sample Size 0 */
    status = FWK_SUCCESS;
    fwk_module_is_valid_element_id_ExpectAndReturn(valid_sensor_id, true);

    status = sensor_smcf_drv_get_sensor_multiple_samples(
        valid_sensor_id, sensor_sample_values, 0);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void test_sensor_smcf_drv_get_sensor_data_sample_fail_invalid_sample_size(void)
{
    int status;

    struct smcf_data_api data_api = {
        .get_data = test_smcf_get_data_api,
    };

    sensor_smcf_drv_ctx.data_api = &data_api;
    uint32_t sensor_sample_values[20];
    fwk_id_t drv_sensor_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR_SMCF_DRV, 0);

    fwk_module_is_valid_element_id_ExpectAndReturn(drv_sensor_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(drv_sensor_id, 0);

    /* For device 0, max sample size is SENSOR_DATA_MAX_SZ_IN_BYTES */
    status = sensor_smcf_drv_get_sensor_multiple_samples(
        drv_sensor_id, sensor_sample_values, SENSOR_DATA_MAX_SZ_IN_BYTES + 1);
    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

int sensor_smcf_drv_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_sensor_smcf_drv_init_success);
    RUN_TEST(test_sensor_smcf_drv_init_fail_zero_devices);
    RUN_TEST(test_sensor_smcf_drv_element_init_success);
    RUN_TEST(test_sensor_smcf_drv_element_init_fail_null_data);
    RUN_TEST(test_sensor_smcf_drv_bind_request_sensor_data_api);
    RUN_TEST(test_sensor_smcf_drv_bind_request_sensor_sample_data_api);
    RUN_TEST(
        test_sensor_smcf_drv_bind_request_sensor_data_api_fail_invalid_params);
    RUN_TEST(test_sensor_smcf_drv_start_success);
    RUN_TEST(test_sensor_smcf_drv_start_fail_invalid_params);
    RUN_TEST(test_sensor_smcf_drv_get_sensor_single_value_success);
    RUN_TEST(test_sensor_smcf_drv_get_sensor_single_value_fail_invalid_param);
    RUN_TEST(test_sensor_smcf_drv_get_sensor_data_sample_success);
    RUN_TEST(test_sensor_smcf_drv_get_sensor_data_sample_fail_invalid_param);
    RUN_TEST(
        test_sensor_smcf_drv_get_sensor_data_sample_fail_invalid_sample_size);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return sensor_smcf_drv_test_main();
}
#endif
