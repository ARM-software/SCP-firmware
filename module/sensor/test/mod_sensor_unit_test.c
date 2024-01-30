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
#define FAKE_RETURN_VALUE       4
#define TRIP_POINT_API_ID_VALUE 4

#define INITIAL_UPDATE_INTERVAL             0xAA55
#define INITIAL_UPDATE_INTERVAL_MULTIPLIER  0x55AA
#define MODIFIED_UPDATE_INTERVAL            0x1234
#define MODIFIED_UPDATE_INTERVAL_MULTIPLIER 0x4321

static struct mod_sensor_config sensor_configuration;

static struct sensor_trip_point_ctx
    sensor_trip_point_context[SENSOR_TRIP_POINT_COUNT];

static unsigned int updated_update_interval = 0;
static unsigned int updated_update_interval_multiplier = 0;

static int sensor_driver_get_value(fwk_id_t id, mod_sensor_value_t *value)
{
    return FWK_SUCCESS;
}

static int sensor_driver_get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    info->update_interval = MODIFIED_UPDATE_INTERVAL;
    info->update_interval_multiplier = MODIFIED_UPDATE_INTERVAL_MULTIPLIER;

    return FWK_SUCCESS;
}

static int sensor_driver_get_info_enabled(
    fwk_id_t id,
    struct mod_sensor_info *info)
{
    info->disabled = false;
    return FWK_SUCCESS;
}

static int sensor_driver_get_info_disabled(
    fwk_id_t id,
    struct mod_sensor_info *info)
{
    info->disabled = true;
    return FWK_SUCCESS;
}

static int sensor_driver_set_update_success(
    fwk_id_t id,
    unsigned int time_interval,
    int time_interval_multiplier)
{
    updated_update_interval = time_interval;
    updated_update_interval_multiplier = time_interval_multiplier;

    return FWK_SUCCESS;
}

static int sensor_driver_set_update_returns_error(
    fwk_id_t id,
    unsigned int time_interval,
    int time_interval_multiplier)
{
    return FWK_E_PARAM;
}

static int sensor_driver_get_value_error(fwk_id_t id, mod_sensor_value_t *value)
{
    return FWK_E_PARAM;
}

static int sensor_driver_get_info_error(
    fwk_id_t id,
    struct mod_sensor_info *info)
{
    return FWK_E_PARAM;
}

static int sensor_driver_enable_succeeds(fwk_id_t id)
{
    return FWK_SUCCESS;
}

static int sensor_driver_enable_not_supported(fwk_id_t id)
{
    return FWK_E_SUPPORT;
}

static int sensor_driver_disable_succeeds(fwk_id_t id)
{
    return FWK_SUCCESS;
}

static int sensor_driver_disable_not_supported(fwk_id_t id)
{
    return FWK_E_SUPPORT;
}

static struct mod_sensor_driver_api sensor_driver_api = {
    .get_value = sensor_driver_get_value,
    .get_info = sensor_driver_get_info,
    .set_update_interval = sensor_driver_set_update_success,
};

static struct mod_sensor_driver_api sensor_driver_api_error = {
    .get_value = sensor_driver_get_value_error,
    .get_info = sensor_driver_get_info_error,
    .set_update_interval = sensor_driver_set_update_returns_error,
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

    sensor_trip_point_context[SENSOR_FAKE_INDEX_0].params.mode =
        MOD_SENSOR_TRIP_POINT_MODE_POSITIVE;
    sensor_trip_point_context[SENSOR_FAKE_INDEX_1].params.mode =
        MOD_SENSOR_TRIP_POINT_MODE_NEGATIVE;

    sensor_dev_context[SENSOR_FAKE_INDEX_0].trip_point_ctx =
        &sensor_trip_point_context[SENSOR_FAKE_INDEX_0];
    sensor_dev_context[SENSOR_FAKE_INDEX_1].trip_point_ctx =
        &sensor_trip_point_context[SENSOR_FAKE_INDEX_1];
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

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

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

    ctx_table[SENSOR_FAKE_INDEX_1].driver_api = &sensor_driver_api;

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

void memcpy_callback(
    void *destination,
    const void *source,
    long unsigned int number,
    int cmock_num_calls)
{
    memcpy(destination, source, number);
}

void utest_sensor_get_data_not_valid(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    status = get_data(elem_id, NULL);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_sensor_get_data_sensor_disabled(void)
{
    int status;

    struct mod_sensor_data returned_data;

    memset(&returned_data, 0, sizeof(returned_data));

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.get_info = sensor_driver_get_info_disabled;

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = get_data(elem_id, &returned_data);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_sensor_get_data_valid_dequeue(void)
{
    int status;

    struct mod_sensor_data initial_data;
    struct mod_sensor_data returned_data;

    memset(&initial_data, 0, sizeof(initial_data));
    memset(&returned_data, 0, sizeof(returned_data));

    initial_data.value = FAKE_RETURN_VALUE;
    initial_data.status = FWK_SUCCESS;

    ctx_table[SENSOR_FAKE_INDEX_0].concurrency_readings.dequeuing = true;
    ctx_table[SENSOR_FAKE_INDEX_0].last_read = initial_data;
    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.get_info = sensor_driver_get_info_enabled;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    fwk_str_memcpy_StubWithCallback(memcpy_callback);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = get_data(elem_id, &returned_data);

    TEST_ASSERT_EQUAL(returned_data.value, FAKE_RETURN_VALUE);
    TEST_ASSERT_EQUAL(returned_data.status, FWK_SUCCESS);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_get_data_valid_call_zero_pending_requests(void)
{
    int status;

    struct mod_sensor_data initial_data;
    struct mod_sensor_data returned_data;

    memset(&initial_data, 0, sizeof(initial_data));
    memset(&returned_data, 0, sizeof(returned_data));

    initial_data.value = FAKE_RETURN_VALUE;
    initial_data.status = FWK_SUCCESS;

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;
    ctx_table[SENSOR_FAKE_INDEX_1].driver_api = &sensor_driver_api;

    ctx_table[SENSOR_FAKE_INDEX_0].concurrency_readings.dequeuing = false;
    ctx_table[SENSOR_FAKE_INDEX_0].last_read = initial_data;

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.get_info = sensor_driver_get_info_enabled;

    ctx_table[SENSOR_FAKE_INDEX_0].concurrency_readings.pending_requests = 0;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    fwk_str_memcpy_StubWithCallback(memcpy_callback);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = get_data(elem_id, &returned_data);

    TEST_ASSERT_EQUAL(returned_data.value, FAKE_RETURN_VALUE);
    TEST_ASSERT_EQUAL(returned_data.status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(ctx_table->last_read.status, FWK_SUCCESS);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_get_info_get_ctx_if_valid_call_returns_error(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    status = get_info(elem_id, NULL);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_sensor_get_info_driver_api_get_info_returns_error(void)
{
    int status;

    struct mod_sensor_complete_info returned_info;

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api_error;
    ctx_table[SENSOR_FAKE_INDEX_1].driver_api = &sensor_driver_api_error;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = get_info(elem_id, &returned_info);

    TEST_ASSERT_EQUAL(status, FWK_E_DEVICE);
}

void utest_sensor_get_info_succeeds(void)
{
    int status;

    struct mod_sensor_complete_info returned_info;

    memset(&returned_info, 0, sizeof(returned_info));

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;
    ctx_table[SENSOR_FAKE_INDEX_1].driver_api = &sensor_driver_api;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_1);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_1);

    status = get_info(elem_id, &returned_info);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(returned_info.trip_point.count, SENSOR_TRIP_POINT_1);
}

void utest_sensor_get_trip_point_0_errors(void)
{
    int status;

    struct mod_sensor_trip_point_params returned_params;

    memset(&returned_params, 0, sizeof(returned_params));

    returned_params.mode = MOD_SENSOR_TRIP_POINT_MODE_TRANSITION;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    uint32_t trip_point_index = 0;

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = sensor_get_trip_point(elem_id, trip_point_index, &returned_params);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
    TEST_ASSERT_EQUAL(
        returned_params.mode, MOD_SENSOR_TRIP_POINT_MODE_TRANSITION);
}

void utest_sensor_get_trip_point_1(void)
{
    int status;

    struct mod_sensor_trip_point_params returned_params;

    memset(&returned_params, 0, sizeof(returned_params));

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_1);

    uint32_t trip_point_index = 0;

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_1);

    status = sensor_get_trip_point(elem_id, trip_point_index, &returned_params);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(
        returned_params.mode, MOD_SENSOR_TRIP_POINT_MODE_NEGATIVE);
}

void utest_sensor_set_trip_point_null_params(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    uint32_t trip_point_index = 0;

    status = sensor_set_trip_point(elem_id, trip_point_index, NULL);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_sensor_set_trip_point_tp_idx_out_of_range(void)
{
    int status;

    struct mod_sensor_trip_point_params send_params;
    struct mod_sensor_trip_point_params returned_params;

    memset(&send_params, 0, sizeof(send_params));
    memset(&returned_params, 0, sizeof(returned_params));

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    uint32_t trip_point_index = SENSOR_TRIP_POINT_COUNT;

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = sensor_set_trip_point(elem_id, trip_point_index, &send_params);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_sensor_set_trip_point_tp_idx_ok(void)
{
    int status;

    struct mod_sensor_trip_point_params send_params;
    struct mod_sensor_trip_point_params returned_params;

    memset(&send_params, 0, sizeof(send_params));
    memset(&returned_params, 0, sizeof(returned_params));

    send_params.mode = MOD_SENSOR_TRIP_POINT_MODE_TRANSITION;
    returned_params.mode = MOD_SENSOR_TRIP_POINT_MODE_POSITIVE;

    sensor_trip_point_context[SENSOR_FAKE_INDEX_1].above_threshold = true;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_1);

    uint32_t trip_point_index = 0;

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_1);

    status = sensor_set_trip_point(elem_id, trip_point_index, &send_params);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(
        sensor_trip_point_context[SENSOR_FAKE_INDEX_1].above_threshold, false);
}

void utest_sensor_enable_no_driver_support(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.enable = NULL;

    status = sensor_enable(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_sensor_enable_driver_succeeded(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.enable = sensor_driver_enable_succeeds;

    status = sensor_enable(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_enable_driver_returned_error(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.enable = sensor_driver_enable_not_supported;

    status = sensor_enable(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_sensor_disable_no_driver_support(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.disable = NULL;

    status = sensor_disable(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_sensor_disable_driver_succeeded(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.disable = sensor_driver_disable_succeeds;

    status = sensor_disable(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_disable_driver_returned_error(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.disable = sensor_driver_disable_not_supported;

    status = sensor_disable(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_sensor_get_time_interval_get_info_returns_error(void)
{
    int status = FWK_SUCCESS;

    unsigned int current_update_interval = INITIAL_UPDATE_INTERVAL;
    int current_update_interval_multiplier = INITIAL_UPDATE_INTERVAL_MULTIPLIER;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api_error;

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = sensor_get_update_interval(
        elem_id, &current_update_interval, &current_update_interval_multiplier);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
    TEST_ASSERT_EQUAL(current_update_interval, INITIAL_UPDATE_INTERVAL);
    TEST_ASSERT_EQUAL(
        current_update_interval_multiplier, INITIAL_UPDATE_INTERVAL_MULTIPLIER);
}

void utest_sensor_get_time_interval_get_info_returns_success(void)
{
    int status = FWK_SUCCESS;

    unsigned int current_update_interval = INITIAL_UPDATE_INTERVAL;
    int current_update_interval_multiplier = INITIAL_UPDATE_INTERVAL_MULTIPLIER;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.get_info = sensor_driver_get_info;

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = sensor_get_update_interval(
        elem_id, &current_update_interval, &current_update_interval_multiplier);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(current_update_interval, MODIFIED_UPDATE_INTERVAL);
    TEST_ASSERT_EQUAL(
        current_update_interval_multiplier,
        MODIFIED_UPDATE_INTERVAL_MULTIPLIER);
}

void utest_sensor_set_time_interval_null_func_ptr(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.set_update_interval = NULL;

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = sensor_set_update_interval(
        elem_id, INITIAL_UPDATE_INTERVAL, INITIAL_UPDATE_INTERVAL_MULTIPLIER);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_sensor_set_time_interval_valid_func_ptr_ret_success(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;

    sensor_driver_api.set_update_interval = sensor_driver_set_update_success,

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = sensor_set_update_interval(
        elem_id, INITIAL_UPDATE_INTERVAL, INITIAL_UPDATE_INTERVAL_MULTIPLIER);

    TEST_ASSERT_EQUAL(updated_update_interval, INITIAL_UPDATE_INTERVAL);
    TEST_ASSERT_EQUAL(
        updated_update_interval_multiplier, INITIAL_UPDATE_INTERVAL_MULTIPLIER);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_set_time_interval_valid_func_ptr_ret_error(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    ctx_table[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api_error;

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_0);

    status = sensor_set_update_interval(
        elem_id, INITIAL_UPDATE_INTERVAL, INITIAL_UPDATE_INTERVAL_MULTIPLIER);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
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

    RUN_TEST(utest_sensor_get_data_not_valid);
    RUN_TEST(utest_sensor_get_data_sensor_disabled);
    RUN_TEST(utest_sensor_get_data_valid_dequeue);
    RUN_TEST(utest_sensor_get_data_valid_call_zero_pending_requests);

    RUN_TEST(utest_sensor_get_info_get_ctx_if_valid_call_returns_error);
    RUN_TEST(utest_sensor_get_info_driver_api_get_info_returns_error);
    RUN_TEST(utest_sensor_get_info_succeeds);

    RUN_TEST(utest_sensor_get_trip_point_0_errors);
    RUN_TEST(utest_sensor_get_trip_point_1);

    RUN_TEST(utest_sensor_set_trip_point_null_params);
    RUN_TEST(utest_sensor_set_trip_point_tp_idx_out_of_range);
    RUN_TEST(utest_sensor_set_trip_point_tp_idx_ok);

    RUN_TEST(utest_sensor_enable_no_driver_support);
    RUN_TEST(utest_sensor_enable_driver_succeeded);
    RUN_TEST(utest_sensor_enable_driver_returned_error);

    RUN_TEST(utest_sensor_disable_no_driver_support);
    RUN_TEST(utest_sensor_disable_driver_succeeded);
    RUN_TEST(utest_sensor_disable_driver_returned_error);

    RUN_TEST(utest_sensor_get_time_interval_get_info_returns_error);
    RUN_TEST(utest_sensor_get_time_interval_get_info_returns_success);
    RUN_TEST(utest_sensor_set_time_interval_null_func_ptr);
    RUN_TEST(utest_sensor_set_time_interval_valid_func_ptr_ret_success);
    RUN_TEST(utest_sensor_set_time_interval_valid_func_ptr_ret_error);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return sensor_test_main();
}
#endif
