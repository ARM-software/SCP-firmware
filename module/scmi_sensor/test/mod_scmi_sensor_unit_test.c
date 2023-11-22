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

#include <config_scmi_sensor.h>

static int scmi_sensor_driver_get_data_pass(
    fwk_id_t id,
    struct mod_sensor_data *data)
{
    return FWK_SUCCESS;
}

static int scmi_sensor_driver_get_data_fail(
    fwk_id_t id,
    struct mod_sensor_data *data)
{
    return FWK_PENDING;
}

static int scmi_sensor_driver_respond(
    fwk_id_t service_id,
    const void *payload,
    size_t size)
{
    return scmi_sensor_ctx.sensor_values[SCMI_SENSOR_FAKE_INDEX_0].status;
}

static int scmi_sensor_driver_respond_with_checks(
    fwk_id_t service_id,
    const void *payload,
    size_t size)
{
    struct scmi_sensor_protocol_reading_get_p2a *pReturnValues =
        (struct scmi_sensor_protocol_reading_get_p2a *)payload;

    TEST_ASSERT_EQUAL(pReturnValues->status, SCMI_SUCCESS);
    TEST_ASSERT_EQUAL(
        pReturnValues->sensor_value_low, LONG_SENSOR_VALUE & 0xffffffff);
    TEST_ASSERT_EQUAL(
        pReturnValues->sensor_value_high,
        (LONG_SENSOR_VALUE >> 32) & 0xffffffff);

    return scmi_sensor_ctx.sensor_values[SCMI_SENSOR_FAKE_INDEX_0].status;
}

static struct mod_sensor_api scmi_sensor_driver_api;
static struct mod_scmi_from_protocol_api scmi_driver_api;

void setUp(void)
{
    scmi_sensor_driver_api.get_data = scmi_sensor_driver_get_data_pass;
    scmi_driver_api.respond = scmi_sensor_driver_respond;
}

void tearDown(void)
{
}

void utest_scmi_sensor_init_nz_elem_cnt(void)
{
    int status;

    status = scmi_sensor_init(
        fwk_module_id_scmi_sensor, SCMI_SENSOR_ELEMENT_COUNT_SINGLE, NULL);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_scmi_sensor_init_zero_sensor_cnt(void)
{
    int status;

    fwk_module_get_element_count_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR), 0);

    status = scmi_sensor_init(fwk_module_id_scmi_sensor, 0, NULL);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_scmi_sensor_init_success(void)
{
    int status;

    struct mod_sensor_data test_sensor_data[SCMI_SENSOR_ELEMENT_COUNT_SINGLE];
    struct sensor_operations
        test_sensor_operations[SCMI_SENSOR_ELEMENT_COUNT_SINGLE];

    fwk_id_t local_service_id = FWK_ID_NONE;

    fwk_module_get_element_count_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR), SCMI_SENSOR_ELEMENT_COUNT_SINGLE);
    fwk_mm_calloc_ExpectAndReturn(
        SCMI_SENSOR_ELEMENT_COUNT_SINGLE,
        sizeof(struct mod_sensor_data),
        (void *)test_sensor_data);
    fwk_mm_calloc_ExpectAndReturn(
        SCMI_SENSOR_ELEMENT_COUNT_SINGLE,
        sizeof(struct sensor_operations),
        (void *)test_sensor_operations);

    test_sensor_operations[0].service_id = fwk_module_id_scmi_sensor;

    status = scmi_sensor_init(
        fwk_module_id_scmi_sensor, SCMI_SENSOR_ELEMENT_INDEX_ZERO, NULL);

    TEST_ASSERT_EQUAL_MEMORY(
        &local_service_id,
        &test_sensor_operations[0].service_id,
        sizeof(fwk_id_t));
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_sensor_init_success_cap_elem_count(void)
{
    int status;

    struct mod_sensor_data test_sensor_data[SCMI_SENSOR_ELEMENT_COUNT_SINGLE];
    struct sensor_operations
        test_sensor_operations[SCMI_SENSOR_ELEMENT_COUNT_MAXIMUM];

    fwk_id_t local_service_id = FWK_ID_NONE;

    fwk_module_get_element_count_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR),
        SCMI_SENSOR_ELEMENT_COUNT_OVER_SIZE);
    fwk_mm_calloc_ExpectAndReturn(
        SCMI_SENSOR_ELEMENT_COUNT_OVER_SIZE,
        sizeof(struct mod_sensor_data),
        (void *)test_sensor_data);
    fwk_mm_calloc_ExpectAndReturn(
        SCMI_SENSOR_ELEMENT_COUNT_MAXIMUM,
        sizeof(struct sensor_operations),
        (void *)test_sensor_operations);

    test_sensor_operations[SCMI_SENSOR_ELEMENT_INDEX_ZERO].service_id =
        fwk_module_id_scmi_sensor;

    status = scmi_sensor_init(
        fwk_module_id_scmi_sensor, SCMI_SENSOR_ELEMENT_INDEX_ZERO, NULL);

    TEST_ASSERT_EQUAL_MEMORY(
        &local_service_id,
        &test_sensor_operations[SCMI_SENSOR_ELEMENT_INDEX_ZERO].service_id,
        sizeof(fwk_id_t));
    TEST_ASSERT_EQUAL(
        scmi_sensor_ctx.sensor_count, SCMI_SENSOR_ELEMENT_COUNT_MAXIMUM);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_sensor_bind_round_equals_1_success(void)
{
    int status;

    fwk_id_t elem_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_SENSOR);

    status = scmi_sensor_bind(elem_id, SCMI_SENSOR_ROUND_1);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_sensor_bind_0_scmi_api_fail(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_FAKE_INDEX_0);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_sensor_ctx.scmi_api,
        FWK_E_PARAM);

    status = scmi_sensor_bind(elem_id, SCMI_SENSOR_ROUND_0);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_scmi_sensor_bind_0_sensor_api_fail(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_FAKE_INDEX_0);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_sensor_ctx.scmi_api,
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR),
        mod_sensor_api_id_sensor,
        &scmi_sensor_ctx.sensor_api,
        FWK_E_PARAM);

    status = scmi_sensor_bind(elem_id, SCMI_SENSOR_ROUND_0);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_scmi_sensor_bind_both_binds_success(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_FAKE_INDEX_0);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_sensor_ctx.scmi_api,
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR),
        mod_sensor_api_id_sensor,
        &scmi_sensor_ctx.sensor_api,
        FWK_SUCCESS);

    status = scmi_sensor_bind(elem_id, SCMI_SENSOR_ROUND_0);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_sensor_start(void)
{
    int status;

    fwk_id_t elem_id = FWK_ID_NONE;

    /* nothing to do without SCMI notifications */
    status = scmi_sensor_start(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_sensor_process_bind_request_invalid_source_id(void)
{
    int status = FWK_SUCCESS;

    struct mod_scmi_sensor_api *api;

    fwk_id_t source_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_t api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_API_IDX_TRIP_POINT);

    fwk_id_get_api_idx_ExpectAndReturn(api_id, SCMI_SENSOR_API_IDX_COUNT);

    status = scmi_sensor_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);
}

void utest_scmi_sensor_process_bind_request_src_invalid_id(void)
{
    int status;

    struct mod_scmi_sensor_api *api;

    fwk_id_t source_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_t api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_API_IDX_TRIP_POINT);

    fwk_id_get_api_idx_ExpectAndReturn(api_id, SCMI_SENSOR_API_IDX_REQUEST);

    fwk_id_is_equal_ExpectAndReturn(
        source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI), false);

    status = scmi_sensor_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);
}

void utest_scmi_sensor_process_bind_api_idx_request(void)
{
    int status;

    struct mod_scmi_sensor_api *api;

    fwk_id_t source_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_t api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_API_IDX_TRIP_POINT);

    fwk_id_get_api_idx_ExpectAndReturn(api_id, SCMI_SENSOR_API_IDX_REQUEST);

    fwk_id_is_equal_ExpectAndReturn(
        source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI), true);

    status = scmi_sensor_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(api, &scmi_sensor_mod_scmi_to_protocol_api);
}

void utest_scmi_sensor_process_bind_api_idx_trip_point_request(void)
{
    int status;

    struct mod_scmi_sensor_api *api;

    fwk_id_t source_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_t target_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_t api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_API_IDX_TRIP_POINT);

    fwk_id_get_api_idx_ExpectAndReturn(api_id, SCMI_SENSOR_API_IDX_TRIP_POINT);

    status = scmi_sensor_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(api, &sensor_trip_point_api);
}

void utest_scmi_sensor_process_event_not_hal_request_nor_response(void)
{
    int status;

    struct fwk_event event;
    struct fwk_event response_event;

    event.id = FWK_ID_NONE;

    scmi_driver_api.respond = scmi_sensor_driver_respond;

    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_scmi_sensor_event_id_get_request, false);
    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_sensor_event_id_read_request, false);

    status = scmi_sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_sensor_process_event_not_hal_request_is_response(void)
{
    int status;

    struct fwk_event event;
    struct fwk_event response_event;
    struct sensor_operations local_sensor_op[SCMI_SENSOR_OPERATIONS];
    struct mod_sensor_data local_sensor_values[SCMI_SENSOR_VALUES];

    fwk_id_t local_service_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_t local_source_id = FWK_ID_NONE;

    memset(local_sensor_op, 0, sizeof(local_sensor_op));
    memset(local_sensor_values, 0, sizeof(local_sensor_values));

    local_sensor_values->status = FWK_SUCCESS;
    local_sensor_values->value = LONG_SENSOR_VALUE;

    event.id = mod_sensor_event_id_read_request;
    event.source_id = local_source_id;

    scmi_sensor_ctx.sensor_ops_table =
        &local_sensor_op[SCMI_SENSOR_FAKE_INDEX_0];
    scmi_sensor_ctx.sensor_values =
        &local_sensor_values[SCMI_SENSOR_FAKE_INDEX_0];

    scmi_sensor_ctx.sensor_api = &scmi_sensor_driver_api;
    scmi_sensor_ctx.scmi_api = &scmi_driver_api;

    scmi_sensor_ctx.sensor_ops_table[SCMI_SENSOR_FAKE_INDEX_0].service_id =
        local_service_id;

    scmi_driver_api.respond = scmi_sensor_driver_respond_with_checks;

    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_scmi_sensor_event_id_get_request, false);
    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_sensor_event_id_read_request, true);

    fwk_id_get_element_idx_ExpectAndReturn(
        event.source_id, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(
        event.source_id, SCMI_SENSOR_FAKE_INDEX_0);

    status = scmi_sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(
        scmi_sensor_ctx.sensor_ops_table[SCMI_SENSOR_FAKE_INDEX_0]
            .service_id.value,
        FWK_ID_NONE.value);
}

void utest_scmi_sensor_process_event_is_hal_request_not_pending(void)
{
    int status;

    struct fwk_event event;
    struct fwk_event response_event;

    struct fwk_event *pEvent = &event;
    struct scmi_sensor_event_parameters *scmi_params =
        (struct scmi_sensor_event_parameters *)pEvent->params;

    event.id = mod_scmi_sensor_event_id_get_request;

    struct sensor_operations local_sensor_op[SCMI_SENSOR_OPERATIONS];
    struct mod_sensor_data local_sensor_values[SCMI_SENSOR_VALUES];
    fwk_id_t service_id;

    memset(local_sensor_op, 0, sizeof(local_sensor_op));
    memset(local_sensor_values, 0, sizeof(local_sensor_values));
    memset(&service_id, 0, sizeof(service_id));

    local_sensor_op[SCMI_SENSOR_FAKE_INDEX_0].service_id = service_id;

    scmi_sensor_ctx.sensor_api = &scmi_sensor_driver_api;
    scmi_sensor_ctx.scmi_api = &scmi_driver_api;

    scmi_sensor_ctx.sensor_ops_table =
        &local_sensor_op[SCMI_SENSOR_FAKE_INDEX_0];
    scmi_sensor_ctx.sensor_values =
        &local_sensor_values[SCMI_SENSOR_FAKE_INDEX_0];

    scmi_driver_api.respond = scmi_sensor_driver_respond;

    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_scmi_sensor_event_id_get_request, true);

    fwk_id_get_element_idx_ExpectAndReturn(
        scmi_params->sensor_id, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(
        scmi_params->sensor_id, SCMI_SENSOR_FAKE_INDEX_0);

    status = scmi_sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_sensor_process_event_is_hal_request_pending(void)
{
    int status;

    struct fwk_event event;
    struct fwk_event response_event;

    struct fwk_event *pEvent = &event;
    struct scmi_sensor_event_parameters *scmi_params =
        (struct scmi_sensor_event_parameters *)pEvent->params;

    event.id = mod_scmi_sensor_event_id_get_request;

    struct sensor_operations local_sensor_op[SCMI_SENSOR_OPERATIONS];
    struct mod_sensor_data local_sensor_values[SCMI_SENSOR_VALUES];
    fwk_id_t service_id;

    memset(local_sensor_op, 0, sizeof(local_sensor_op));
    memset(local_sensor_values, 0, sizeof(local_sensor_values));
    memset(&service_id, 0, sizeof(service_id));

    local_sensor_op[SCMI_SENSOR_FAKE_INDEX_0].service_id = service_id;

    scmi_sensor_driver_api.get_data = scmi_sensor_driver_get_data_fail;

    scmi_sensor_ctx.sensor_api = &scmi_sensor_driver_api;
    scmi_sensor_ctx.scmi_api = &scmi_driver_api;

    scmi_sensor_ctx.sensor_ops_table =
        &local_sensor_op[SCMI_SENSOR_FAKE_INDEX_0];
    scmi_sensor_ctx.sensor_values =
        &local_sensor_values[SCMI_SENSOR_FAKE_INDEX_0];

    scmi_driver_api.respond = scmi_sensor_driver_respond;

    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_scmi_sensor_event_id_get_request, true);
    fwk_id_is_equal_ExpectAndReturn(
        event.id, mod_sensor_event_id_read_request, false);

    fwk_id_get_element_idx_ExpectAndReturn(
        scmi_params->sensor_id, SCMI_SENSOR_FAKE_INDEX_0);
    fwk_id_get_element_idx_ExpectAndReturn(
        scmi_params->sensor_id, SCMI_SENSOR_FAKE_INDEX_0);

    status = scmi_sensor_process_event(&event, &response_event);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int sensor_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_scmi_sensor_init_nz_elem_cnt);
    RUN_TEST(utest_scmi_sensor_init_zero_sensor_cnt);
    RUN_TEST(utest_scmi_sensor_init_success);
    RUN_TEST(utest_scmi_sensor_init_success_cap_elem_count);

    RUN_TEST(utest_scmi_sensor_bind_round_equals_1_success);
    RUN_TEST(utest_scmi_sensor_bind_0_scmi_api_fail);
    RUN_TEST(utest_scmi_sensor_bind_0_sensor_api_fail);
    RUN_TEST(utest_scmi_sensor_bind_both_binds_success);

    RUN_TEST(utest_scmi_sensor_start);

    RUN_TEST(utest_scmi_sensor_process_bind_request_invalid_source_id);
    RUN_TEST(utest_scmi_sensor_process_bind_request_src_invalid_id);
    RUN_TEST(utest_scmi_sensor_process_bind_api_idx_request);
    RUN_TEST(utest_scmi_sensor_process_bind_api_idx_trip_point_request);

    RUN_TEST(utest_scmi_sensor_process_event_not_hal_request_nor_response);
    RUN_TEST(utest_scmi_sensor_process_event_not_hal_request_is_response);
    RUN_TEST(utest_scmi_sensor_process_event_is_hal_request_not_pending);
    RUN_TEST(utest_scmi_sensor_process_event_is_hal_request_pending);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return sensor_test_main();
}
#endif
