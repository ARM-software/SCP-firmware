/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#ifdef TEST_ON_TARGET
#    include <fwk_id.h>
#    include <fwk_module.h>
#else
#    include <Mockfwk_id.h>
#    include <Mockfwk_module.h>
#endif

#include <Mockmod_scmi_sensor_req_extra.h>

#include <mod_scmi.h>
#include <mod_scmi_sensor_req.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC

enum fake_sensors {
    FAKE_SCMI_SENSOR_REQ_1_IDX,
    FAKE_SCMI_SENSOR_REQ_IDX_COUNT,
};

static const struct fwk_element scmi_sensor_req_element_table[] = {
    [FAKE_SCMI_SENSOR_REQ_1_IDX] = {
    .name = "SENSOR_REQ_1",
    .data = &((struct scmi_sensor_req_config){
        .service_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI,
            0),
        .scmi_sensor_id = 0,
        .sensor_hal_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR,0),
        .async_flag = SENSOR_REQ_SYNC,
    }),
    },
    [FAKE_SCMI_SENSOR_REQ_IDX_COUNT] = {0},
};

static const struct fwk_element *get_sensor_req_element_table(
    fwk_id_t module_id)
{
    return scmi_sensor_req_element_table;
}

struct fwk_module_config config_scmi_sensor_req = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_sensor_req_element_table),
};

const struct mod_sensor_driver_response_api resp_api = {
    reading_complete,
};

const struct mod_scmi_from_protocol_req_api scmi_api = {
    .scmi_send_message = scmi_send_message,
    .response_message_handler = response_message_handler,
};

fwk_id_t scmi_module_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI);

void setUp(void)
{
    scmi_sensor_req_ctx.resp_api = &resp_api;
    scmi_sensor_req_ctx.scmi_api = &scmi_api;
    handler_table[MOD_SCMI_SENSOR_READING_GET] = fake_message_handler;
}

void tearDown(void)
{
}

void test_init(void)
{
    int status;

    status = scmi_sensor_req_init(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_SENSOR_REQ), 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_DATA);

    status = scmi_sensor_req_init(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_SENSOR_REQ),
        FAKE_SCMI_SENSOR_REQ_IDX_COUNT,
        NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_elem_init(void)
{
    int status;
    fwk_id_t element_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_SCMI_SENSOR_REQ, FAKE_SCMI_SENSOR_REQ_1_IDX);

    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, FAKE_SCMI_SENSOR_REQ_1_IDX);

    status = scmi_sensor_req_elem_init(
        element_id,
        0,
        scmi_sensor_req_element_table[FAKE_SCMI_SENSOR_REQ_1_IDX].data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    /* Test invalid */
    element_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_SCMI_SENSOR_REQ, FAKE_SCMI_SENSOR_REQ_IDX_COUNT);

    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, FAKE_SCMI_SENSOR_REQ_IDX_COUNT);

    status = scmi_sensor_req_elem_init(
        element_id,
        0,
        scmi_sensor_req_element_table[FAKE_SCMI_SENSOR_REQ_1_IDX].data);
    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}

void test_bind(void)
{
    int status;
    fwk_id_t expected_api_id;
    fwk_id_t expected_sensor_hal_id =
        ((struct scmi_sensor_req_config *)
             scmi_sensor_req_element_table[FAKE_SCMI_SENSOR_REQ_1_IDX]
                 .data)
            ->sensor_hal_id;

    expected_api_id =
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL_REQ);

    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    fwk_module_bind_ExpectAndReturn(
        scmi_module_id,
        expected_api_id,
        &scmi_sensor_req_ctx.scmi_api,
        FWK_SUCCESS);
    status = scmi_sensor_req_bind(scmi_module_id, 0);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    fwk_id_is_type_ExpectAnyArgsAndReturn(false);
    fwk_id_get_element_idx_ExpectAndReturn(
        expected_sensor_hal_id, FAKE_SCMI_SENSOR_REQ_1_IDX);
    fwk_module_bind_ExpectAndReturn(
        expected_sensor_hal_id,
        mod_sensor_api_id_driver_response,
        &scmi_sensor_req_ctx.resp_api,
        FWK_SUCCESS);
    status = scmi_sensor_req_bind(expected_sensor_hal_id, 0);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_process_bind(void)
{
    int status;
    fwk_id_t sensor_module_id;
    fwk_id_t invalid_module_id = { .value = UINT32_MAX };
    fwk_id_t target_id;
    fwk_id_t api_id;
    struct mod_scmi_to_protocol_api *scmi_api;
    struct mod_sensor_driver_api *driver_api;

    fwk_id_get_module_idx_ExpectAndReturn(scmi_module_id, FWK_MODULE_IDX_SCMI);
    status = scmi_sensor_req_process_bind_request(
        scmi_module_id, target_id, api_id, (const void **)&scmi_api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL_PTR(scmi_api, &scmi_sensor_mod_scmi_to_protocol_api);

    sensor_module_id = FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR);
    fwk_id_get_module_idx_ExpectAndReturn(
        sensor_module_id, FWK_MODULE_IDX_SENSOR);
    status = scmi_sensor_req_process_bind_request(
        sensor_module_id, target_id, api_id, (const void **)&driver_api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL_PTR(driver_api, &scmi_sensor_req_api);

    fwk_id_get_module_idx_ExpectAndReturn(invalid_module_id, UINT32_MAX);
    status = scmi_sensor_req_process_bind_request(
        invalid_module_id, target_id, api_id, (const void **)&driver_api);
    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);
}

void test_function_scmi_sensor_req_ret_reading_handler(void)
{
    int status;
    fwk_id_t expected_sensor_hal_id =
        ((struct scmi_sensor_req_config *)
             scmi_sensor_req_element_table[FAKE_SCMI_SENSOR_REQ_1_IDX]
                 .data)
            ->sensor_hal_id;
    fwk_id_t expected_service_id =
        ((struct scmi_sensor_req_config *)
             scmi_sensor_req_element_table[FAKE_SCMI_SENSOR_REQ_1_IDX]
                 .data)
            ->service_id;

    fwk_id_t error_service_id = { .value = 0xFFFF };
    struct mod_sensor_driver_resp_params expected_resp_params = { 0 };
    uint32_t payload[10] = { 0 };

    expected_resp_params = (struct mod_sensor_driver_resp_params){
        .status = FWK_SUCCESS,
        .value = 0,
    };

    fwk_id_is_equal_ExpectAndReturn(
        error_service_id, expected_service_id, false);
    status = scmi_sensor_req_ret_reading_handler(
        error_service_id, payload, sizeof(payload));
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);

    fwk_id_is_equal_ExpectAndReturn(
        expected_service_id, expected_service_id, true);
    reading_complete_ExpectWithArray(
        expected_sensor_hal_id, &expected_resp_params, 1);
    status = scmi_sensor_req_ret_reading_handler(
        expected_service_id, payload, sizeof(payload));
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_function_scmi_sensor_req_get_value(void)
{
    int status;
    mod_sensor_value_t sensor_value;
    fwk_id_t expected_service_id;
    fwk_id_t sensor_id = FWK_ID_ELEMENT_INIT(
        FWK_MODULE_IDX_SCMI_SENSOR_REQ, FAKE_SCMI_SENSOR_REQ_1_IDX);

    struct scmi_sensor_protocol_reading_get_a2p expected_payload = {
        .sensor_id =
            ((struct scmi_sensor_req_config *)
                 scmi_sensor_req_element_table[FAKE_SCMI_SENSOR_REQ_1_IDX]
                     .data)
                ->scmi_sensor_id,
        .flags = (uint32_t)(
            ((struct scmi_sensor_req_config *)
                 scmi_sensor_req_element_table[FAKE_SCMI_SENSOR_REQ_1_IDX]
                     .data)
                ->async_flag),
    };

    expected_service_id =
        ((struct scmi_sensor_req_config *)
             scmi_sensor_req_element_table[FAKE_SCMI_SENSOR_REQ_1_IDX]
                 .data)
            ->service_id;

    fwk_id_get_element_idx_ExpectAndReturn(
        sensor_id, FAKE_SCMI_SENSOR_REQ_1_IDX);
    scmi_send_message_ExpectWithArrayAndReturn(
        (uint8_t)MOD_SCMI_SENSOR_READING_GET,
        (uint8_t)MOD_SCMI_PROTOCOL_ID_SENSOR,
        0,
        expected_service_id,
        &expected_payload,
        1,
        sizeof(expected_payload),
        true,
        FWK_SUCCESS);
    status = scmi_sensor_req_get_value(sensor_id, &sensor_value);
    TEST_ASSERT_EQUAL(status, FWK_PENDING);
}

void test_function_scmi_sensor_req_get_value_invalid(void)
{
    int status;
    mod_sensor_value_t sensor_value;
    fwk_id_t sensor_id = FWK_ID_ELEMENT_INIT(
        FWK_MODULE_IDX_SCMI_SENSOR_REQ, FAKE_SCMI_SENSOR_REQ_IDX_COUNT);
    ;

    fwk_id_get_element_idx_ExpectAndReturn(
        sensor_id, FAKE_SCMI_SENSOR_REQ_IDX_COUNT);

    status = scmi_sensor_req_get_value(sensor_id, &sensor_value);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_function_scmi_sensor_req_get_info(void)
{
    int status;
    fwk_id_t id;
    struct mod_sensor_info sensor_info;
    status = scmi_sensor_req_get_info(id, &sensor_info);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void test_function_scmi_sensor_req_get_scmi_protocol_id(void)
{
    int status;
    fwk_id_t protocol_id;
    uint8_t scmi_protocol_id;
    status =
        scmi_sensor_req_get_scmi_protocol_id(protocol_id, &scmi_protocol_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(scmi_protocol_id, (uint8_t)MOD_SCMI_PROTOCOL_ID_SENSOR);
}

void test_function_scmi_sensor_req_message_handler(void)
{
    int status;
    fwk_id_t expected_service_id;
    fwk_id_t protocol_id;
    uint8_t payload[sizeof(struct scmi_sensor_protocol_reading_get_p2a)] = {
        0
    };

    expected_service_id =
        ((struct scmi_sensor_req_config *)
             scmi_sensor_req_element_table[FAKE_SCMI_SENSOR_REQ_1_IDX]
                 .data)
            ->service_id;

    status = scmi_sensor_req_message_handler(
        protocol_id,
        expected_service_id,
        (uint32_t *)payload,
        sizeof(payload),
        sizeof(handler_table));
    TEST_ASSERT_EQUAL(status, FWK_E_RANGE);

    status = scmi_sensor_req_message_handler(
        protocol_id,
        expected_service_id,
        (uint32_t *)payload,
        sizeof(payload) + 1,
        (uint8_t)MOD_SCMI_SENSOR_READING_GET);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);

    fake_message_handler_ExpectWithArrayAndReturn(
        expected_service_id,
        (uint32_t *)payload,
        sizeof(payload),
        sizeof(payload),
        FWK_SUCCESS);
    response_message_handler_ExpectAndReturn(expected_service_id, FWK_SUCCESS);
    status = scmi_sensor_req_message_handler(
        protocol_id,
        expected_service_id,
        (uint32_t *)payload,
        sizeof(payload),
        (uint8_t)MOD_SCMI_SENSOR_READING_GET);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int scmi_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_init);
    RUN_TEST(test_elem_init);
    RUN_TEST(test_bind);
    RUN_TEST(test_process_bind);
    RUN_TEST(test_function_scmi_sensor_req_ret_reading_handler);
    RUN_TEST(test_function_scmi_sensor_req_get_value);
    RUN_TEST(test_function_scmi_sensor_req_get_value_invalid);
    RUN_TEST(test_function_scmi_sensor_req_get_info);
    RUN_TEST(test_function_scmi_sensor_req_get_scmi_protocol_id);
    RUN_TEST(test_function_scmi_sensor_req_message_handler);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_test_main();
}
#endif
