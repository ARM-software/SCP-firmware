/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_core.h>
#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockfwk_string.h>
#include <internal/Mockfwk_core_internal.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_element.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include UNIT_TEST_SRC

#include <config_sensor.h>

#define NOTIFICATION_ID_VALUE   3
#define TRIP_POINT_API_ID_VALUE 4

static struct mod_sensor_config sensor_configuration;

static int sensor_driver_get_value(fwk_id_t id, mod_sensor_value_t *value)
{
    return FWK_SUCCESS;
}

static int sensor_driver_get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    return FWK_SUCCESS;
}

static const struct mod_sensor_driver_api sensor_driver_api = {
    .get_value = sensor_driver_get_value,
    .get_info = sensor_driver_get_info,
};

void setUp(void)
{
    ctx_table = sensor_dev_context;
    memset(
        sensor_dev_context,
        0,
        SENSOR_ELEMENT_COUNT * sizeof(struct sensor_dev_ctx));

    sensor_dev_context[SENSOR_FAKE_INDEX_0].config =
        (struct mod_sensor_dev_config *)
            sensor_element_table[SENSOR_FAKE_INDEX_0]
                .data;
    sensor_dev_context[SENSOR_FAKE_INDEX_1].config =
        (struct mod_sensor_dev_config *)
            sensor_element_table[SENSOR_FAKE_INDEX_1]
                .data;
}

void tearDown(void)
{
}

void utest_sensor_init(void)
{
    int status;

    struct sensor_dev_ctx device_context[SENSOR_ELEMENT_COUNT];

    fwk_mm_calloc_ExpectAndReturn(
        SENSOR_ELEMENT_COUNT,
        sizeof(struct sensor_dev_ctx),
        (void *)device_context);

    fwk_str_memset_Expect(&sensor_mod_ctx, 0, sizeof(sensor_mod_ctx));

    sensor_mod_ctx.config = NULL;

    fwk_id_t notification_id =
        FWK_ID_NOTIFICATION(FWK_MODULE_IDX_REG_SENSOR, 0);
    fwk_id_t api_id = FWK_ID_API(FWK_MODULE_IDX_REG_SENSOR, 0);

    sensor_configuration.notification_id = notification_id;
    sensor_configuration.trip_point_api_id = api_id;

    sensor_configuration.notification_id.value = NOTIFICATION_ID_VALUE;
    sensor_configuration.trip_point_api_id.value = TRIP_POINT_API_ID_VALUE;

    status = sensor_init(
        fwk_module_id_sensor,
        SENSOR_ELEMENT_COUNT,
        (void *)&sensor_configuration);

    TEST_ASSERT_EQUAL(
        (void *)sensor_mod_ctx.config, (void *)&sensor_configuration);
    TEST_ASSERT_EQUAL(
        NOTIFICATION_ID_VALUE, sensor_mod_ctx.config->notification_id.value);
    TEST_ASSERT_EQUAL(
        TRIP_POINT_API_ID_VALUE,
        sensor_mod_ctx.config->trip_point_api_id.value);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_dev_init_zero_trip_point_count(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].last_read.status = FWK_SUCCESS;

    status = sensor_dev_init(
        elem_id, 0, sensor_element_table[SENSOR_FAKE_INDEX_0].data);

    TEST_ASSERT_EQUAL(
        ctx_table[SENSOR_FAKE_INDEX_0].last_read.status, FWK_E_DEVICE);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_dev_init_non_zero_trip_point_count(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_1);

    struct sensor_trip_point_ctx trip_point_context[SENSOR_TRIP_POINT_COUNT];

    fwk_mm_calloc_ExpectAndReturn(
        1, sizeof(struct sensor_trip_point_ctx), (void *)trip_point_context);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_1);

    status = sensor_dev_init(
        elem_id, 0, sensor_element_table[SENSOR_FAKE_INDEX_1].data);

    TEST_ASSERT_EQUAL(
        ctx_table[SENSOR_FAKE_INDEX_1].trip_point_ctx->enabled, true);

    TEST_ASSERT_EQUAL(
        ctx_table[SENSOR_FAKE_INDEX_1].last_read.status, FWK_E_DEVICE);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

static int bind_callback_fail(
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void *api,
    int cmock_num_calls)
{
    return FWK_E_PARAM;
}

static int bind_callback_success(
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void *api,
    int cmock_num_calls)
{
    const struct mod_sensor_driver_api **sensor_api =
        (const struct mod_sensor_driver_api **)api;

    *sensor_api = &sensor_driver_api;

    return FWK_SUCCESS;
}

void utest_sensor_bind_round_1_success(void)
{
    int status;

    fwk_id_t module_id = FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR);

    status = sensor_bind(module_id, SENSOR_ROUND_1);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_bind_id_type_module_config_not_null(void)
{
    int status;

    sensor_mod_ctx.config = &sensor_configuration;

    fwk_id_t module_id = FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR);

    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, true);

    status = sensor_bind(module_id, SENSOR_ROUND_0);

    TEST_ASSERT_EQUAL(
        (void *)sensor_mod_ctx.config, (void *)&sensor_configuration);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_bind_element_bind_fails(void)
{
    int status;

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = NULL;
    ctx_table[SENSOR_FAKE_INDEX_1].driver_api = NULL;

    ctx_table[SENSOR_FAKE_INDEX_0].last_read.status = FWK_SUCCESS;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);
    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_MODULE, false);

    fwk_module_bind_StubWithCallback(bind_callback_fail);

    status = sensor_bind(elem_id, SENSOR_ROUND_0);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
    TEST_ASSERT_EQUAL(ctx_table[SENSOR_FAKE_INDEX_0].driver_api, NULL);
    TEST_ASSERT_EQUAL(ctx_table[SENSOR_FAKE_INDEX_1].driver_api, NULL);
}

void utest_sensor_bind_type_mismatch_driver_bind_idx_0_success(void)
{
    int status;

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = NULL;
    ctx_table[SENSOR_FAKE_INDEX_1].driver_api = NULL;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);
    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_MODULE, false);

    fwk_module_bind_StubWithCallback(bind_callback_success);

    status = sensor_bind(elem_id, SENSOR_ROUND_0);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(
        ctx_table[SENSOR_FAKE_INDEX_0].driver_api, &sensor_driver_api);
    TEST_ASSERT_EQUAL(ctx_table[SENSOR_FAKE_INDEX_1].driver_api, NULL);
}

void utest_sensor_process_module_bind_request(void)
{
    int status;

    struct mod_sensor_api *api;

    fwk_id_t source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_FAKE_MODULE, 0);
    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_t api_id =
        FWK_ID_API(FWK_MODULE_IDX_SENSOR, MOD_SENSOR_API_IDX_SENSOR);

    fwk_id_is_equal_ExpectAndReturn(api_id, mod_sensor_api_id_sensor, true);

    status = sensor_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(api, &sensor_api);
}

void utest_sensor_process_bind_request_from_driver(void)
{
    int status;

    struct mod_sensor_api *api;

    fwk_id_t source_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_FAKE_MODULE, SENSOR_FAKE_INDEX_0);
    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_t api_id =
        FWK_ID_API(FWK_MODULE_IDX_SENSOR, MOD_SENSOR_API_IDX_SENSOR);

    fwk_id_is_equal_ExpectAndReturn(api_id, mod_sensor_api_id_sensor, false);
    fwk_id_is_equal_ExpectAndReturn(
        api_id, mod_sensor_api_id_driver_response, true);

    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);

    fwk_id_get_element_idx_ExpectAndReturn(target_id, SENSOR_FAKE_INDEX_0);

    fwk_id_get_module_idx_ExpectAndReturn(
        ctx_table->config->driver_id, SENSOR_FAKE_MODULE_0);
    fwk_id_get_module_idx_ExpectAndReturn(source_id, SENSOR_FAKE_MODULE_0);

    fwk_id_get_element_idx_ExpectAndReturn(
        ctx_table->config->driver_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(source_id, SENSOR_FAKE_INDEX_0);

    status = sensor_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(api, &sensor_driver_response_api);
}

void utest_sensor_process_bind_request_target_not_elem(void)
{
    int status;

    struct mod_sensor_api *api;

    fwk_id_t source_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_FAKE_MODULE, SENSOR_FAKE_INDEX_0);
    fwk_id_t target_id = FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR);
    fwk_id_t api_id =
        FWK_ID_API(FWK_MODULE_IDX_SENSOR, MOD_SENSOR_API_IDX_SENSOR);
    fwk_id_t mod_sensor_api_id_sensor =
        FWK_ID_API(FWK_MODULE_IDX_SENSOR, MOD_SENSOR_API_IDX_SENSOR);

    fwk_id_is_equal_ExpectAndReturn(api_id, mod_sensor_api_id_sensor, false);
    fwk_id_is_equal_ExpectAndReturn(
        api_id, mod_sensor_api_id_driver_response, true);

    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, false);

    status = sensor_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_sensor_process_bind_request_sensor_idx_mismatch(void)
{
    int status;

    struct mod_sensor_api *api;

    fwk_id_t source_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_FAKE_MODULE, SENSOR_FAKE_INDEX_0);
    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_t api_id =
        FWK_ID_API(FWK_MODULE_IDX_SENSOR, MOD_SENSOR_API_IDX_SENSOR);
    fwk_id_t mod_sensor_api_id_sensor =
        FWK_ID_API(FWK_MODULE_IDX_SENSOR, MOD_SENSOR_API_IDX_SENSOR);

    fwk_id_is_equal_ExpectAndReturn(api_id, mod_sensor_api_id_sensor, false);
    fwk_id_is_equal_ExpectAndReturn(
        api_id, mod_sensor_api_id_driver_response, true);

    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);

    fwk_id_get_module_idx_ExpectAndReturn(
        ctx_table->config->driver_id, SENSOR_FAKE_MODULE_0);
    fwk_id_get_module_idx_ExpectAndReturn(source_id, SENSOR_FAKE_MODULE_0);

    fwk_id_get_element_idx_ExpectAndReturn(target_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(
        ctx_table->config->driver_id, SENSOR_FAKE_INDEX_1);
    fwk_id_get_element_idx_ExpectAndReturn(source_id, SENSOR_FAKE_INDEX_2);

    status = sensor_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);
}

void utest_sensor_process_event_invalid_element_id(void)
{
    int status;

    struct fwk_event event;
    struct fwk_event response_event;

    fwk_id_t event_id = FWK_ID_EVENT(FWK_MODULE_IDX_SENSOR, 0);

    event.target_id = event_id;

    fwk_module_is_valid_element_id_ExpectAndReturn(event_id, false);

    status = sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_sensor_process_event_invalid_event_id_type(void)
{
    int status;

    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_t event_id = FWK_ID_EVENT(FWK_MODULE_IDX_SENSOR, 0);

    struct fwk_event event;
    struct fwk_event response_event;

    event.target_id = target_id;
    event.id = event_id;

    fwk_module_is_valid_element_id_ExpectAndReturn(target_id, true);

    fwk_id_get_element_idx_ExpectAndReturn(target_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_event_idx_ExpectAndReturn(event.id, SENSOR_EVENT_IDX_COUNT);

    status = sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_sensor_process_event_read_request_no_pending_requests(void)
{
    int status;

    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_t event_id = FWK_ID_EVENT(FWK_MODULE_IDX_SENSOR, 0);

    struct fwk_event event;
    struct fwk_event response_event;

    event.target_id = target_id;
    event.id = event_id;

    fwk_module_is_valid_element_id_ExpectAndReturn(target_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_event_idx_ExpectAndReturn(
        event.id, SENSOR_EVENT_IDX_READ_REQUEST);

    ctx_table->concurrency_readings.pending_requests = 0;

    status = sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_TRUE(response_event.is_delayed_response);
}

void utest_sensor_process_event_get_delayed_response_fails(void)
{
    int status;

    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_t event_id = FWK_ID_EVENT(FWK_MODULE_IDX_SENSOR, 0);

    struct fwk_event event;
    struct fwk_event response_event;

    event.target_id = target_id;
    event.id = event_id;

    fwk_module_is_valid_element_id_ExpectAndReturn(target_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_event_idx_ExpectAndReturn(
        event.id, SENSOR_EVENT_IDX_READ_COMPLETE);

    /*
     * This function has to be "ExpectAnyArgs" because one of the
     * arguments (at run time) is a local variable in the function
     * under test.
     */

    fwk_get_delayed_response_ExpectAnyArgsAndReturn(FWK_E_PARAM);

    status = sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_sensor_process_event_put_event_fails(void)
{
    int status;

    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_t event_id = FWK_ID_EVENT(FWK_MODULE_IDX_SENSOR, 0);

    struct fwk_event event;
    struct fwk_event response_event;

    event.target_id = target_id;
    event.id = event_id;

    fwk_module_is_valid_element_id_ExpectAndReturn(target_id, true);

    fwk_id_get_element_idx_ExpectAndReturn(target_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_event_idx_ExpectAndReturn(
        event.id, SENSOR_EVENT_IDX_READ_COMPLETE);

    /*
     * This function has to be "ExpectAnyArgs" because one of the
     * arguments (at run time) is a local variable in the function
     * under test.
     */

    fwk_get_delayed_response_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    fwk_str_memcpy_ExpectAnyArgs();

    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_E_PARAM);

    status = sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_sensor_process_event_rd_cmplt_del_rsp_put_evt_success(void)
{
    int status;

    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_t event_id = FWK_ID_EVENT(FWK_MODULE_IDX_SENSOR, 0);

    struct fwk_event event;
    struct fwk_event response_event;

    event.target_id = target_id;
    event.id = event_id;

    fwk_module_is_valid_element_id_ExpectAndReturn(target_id, true);

    fwk_id_get_element_idx_ExpectAndReturn(target_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, SENSOR_FAKE_INDEX_0);

    fwk_id_get_event_idx_ExpectAndReturn(
        event.id, SENSOR_EVENT_IDX_READ_COMPLETE);

    fwk_get_delayed_response_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    fwk_str_memcpy_ExpectAnyArgs();

    fwk_is_delayed_response_list_empty_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int sensor_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_sensor_init);
    RUN_TEST(utest_sensor_dev_init_zero_trip_point_count);
    RUN_TEST(utest_sensor_dev_init_non_zero_trip_point_count);

    RUN_TEST(utest_sensor_bind_round_1_success);
    RUN_TEST(utest_sensor_bind_id_type_module_config_not_null);
    RUN_TEST(utest_sensor_bind_element_bind_fails);
    RUN_TEST(utest_sensor_bind_type_mismatch_driver_bind_idx_0_success);

    RUN_TEST(utest_sensor_process_module_bind_request);
    RUN_TEST(utest_sensor_process_bind_request_from_driver);
    RUN_TEST(utest_sensor_process_bind_request_target_not_elem);
    RUN_TEST(utest_sensor_process_bind_request_sensor_idx_mismatch);

    RUN_TEST(utest_sensor_process_event_invalid_element_id);
    RUN_TEST(utest_sensor_process_event_invalid_event_id_type);
    RUN_TEST(utest_sensor_process_event_read_request_no_pending_requests);
    RUN_TEST(utest_sensor_process_event_get_delayed_response_fails);
    RUN_TEST(utest_sensor_process_event_put_event_fails);
    RUN_TEST(utest_sensor_process_event_rd_cmplt_del_rsp_put_evt_success);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return sensor_test_main();
}
#endif
