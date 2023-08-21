/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockmod_scmi_system_power_req_extra.h>

#include <internal/Mockfwk_core_internal.h>

#include <mod_scmi.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC

enum scp_sys_pow_nums {
    MOD_SCMI_SYS_POWER_REQ_IDX_0,
    MOD_SCMI_SYS_POWER_REQ_IDX_1,
    MOD_SCMI_SYS_POWER_REQ_COUNT,
};

/*
 * System Power Req module config
 */
static const struct fwk_element
    system_power_req_element_table[MOD_SCMI_SYS_POWER_REQ_COUNT + 1] = {
    [MOD_SCMI_SYS_POWER_REQ_IDX_0] = {
        .name = "Fake sys power",
        .data = &(const struct mod_scmi_system_power_req_dev_config) {
            .service_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_SCMI,
                0),
        },
    },
    [MOD_SCMI_SYS_POWER_REQ_IDX_1] = {
        .name = "Fake sys power 1",
        .data = &(const struct mod_scmi_system_power_req_dev_config) {
            .service_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_SCMI,
                0),
        },
    },
    [MOD_SCMI_SYS_POWER_REQ_COUNT] = { 0 },
};

static const struct fwk_element *get_system_power_req_element_table(
    fwk_id_t module_id)
{
    return system_power_req_element_table;
}

const struct fwk_module_config config_scmi_system_power_req = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_system_power_req_element_table),
};

const struct mod_scmi_from_protocol_req_api scmi_api = {
    .scmi_send_message = scmi_send_message,
    .response_message_handler = response_message_handler,
};

static struct scmi_system_power_req_dev_ctx
    dev_ctx[MOD_SCMI_SYS_POWER_REQ_COUNT];

void setUp(void)
{
    memset(&mod_ctx, 0, sizeof(mod_ctx));
    memset(
        &dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_0],
        0,
        sizeof(struct scmi_system_power_req_dev_ctx));
    dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_0].config =
        (const struct mod_scmi_system_power_req_dev_config *)
            system_power_req_element_table[MOD_SCMI_SYS_POWER_REQ_IDX_0]
                .data;

    memset(
        &dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_1],
        0,
        sizeof(struct scmi_system_power_req_dev_ctx));
    dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_1].config =
        (const struct mod_scmi_system_power_req_dev_config *)
            system_power_req_element_table[MOD_SCMI_SYS_POWER_REQ_IDX_1]
                .data;

    mod_ctx.dev_ctx_table = dev_ctx;
    mod_ctx.dev_count = MOD_SCMI_SYS_POWER_REQ_COUNT;
    mod_ctx.scmi_api = &scmi_api;
    mod_ctx.token = 0;
    mod_ctx.dev_ctx_table[0].state = MOD_PD_STATE_ON;
    mod_ctx.dev_ctx_table[1].state = MOD_PD_STATE_OFF;

    handler_table[MOD_SCMI_SYS_POWER_REQ_STATE_SET] = fake_message_handler;
}

void tearDown(void)
{
}

void test_function_scmi_system_power_req_init(void)
{
    int status;

    /* Clear module context to ensure it is properly initialized */
    memset(&mod_ctx, 0, sizeof(mod_ctx));

    status = scmi_system_power_req_init(
        fwk_module_id_scmi_system_power_req, 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);

    fwk_mm_calloc_ExpectAndReturn(
        MOD_SCMI_SYS_POWER_REQ_COUNT,
        sizeof(struct scmi_system_power_req_dev_ctx),
        (void *)dev_ctx);

    status = scmi_system_power_req_init(
        fwk_module_id_scmi_system_power_req,
        MOD_SCMI_SYS_POWER_REQ_COUNT,
        NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(mod_ctx.dev_count, MOD_SCMI_SYS_POWER_REQ_COUNT);
    TEST_ASSERT_EQUAL(mod_ctx.dev_ctx_table, dev_ctx);
}

void test_function_scmi_system_power_req_elem_init_0(void)
{
    int status;
    fwk_id_t element_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ, MOD_SCMI_SYS_POWER_REQ_IDX_0);

    /* Clear element context to ensure it is properly initialized */
    memset(
        &dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_0],
        0,
        sizeof(struct scmi_system_power_req_dev_ctx));

    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_IDX_0);
    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_IDX_0);

    status = scmi_system_power_req_elem_init(
        element_id,
        0,
        system_power_req_element_table[MOD_SCMI_SYS_POWER_REQ_IDX_0].data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_COUNT);

    status = scmi_system_power_req_elem_init(
        element_id,
        0,
        system_power_req_element_table[MOD_SCMI_SYS_POWER_REQ_IDX_0].data);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);

    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_IDX_0);
    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_IDX_0);

    status = scmi_system_power_req_elem_init(element_id, 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PANIC);
}

void test_function_scmi_system_power_req_elem_init_1(void)
{
    int status;
    fwk_id_t element_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ, MOD_SCMI_SYS_POWER_REQ_IDX_1);

    /* Clear element context to ensure it is properly initialized */
    memset(
        &dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_1],
        0,
        sizeof(struct scmi_system_power_req_dev_ctx));

    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_IDX_1);
    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_IDX_1);

    status = scmi_system_power_req_elem_init(
        element_id,
        0,
        system_power_req_element_table[MOD_SCMI_SYS_POWER_REQ_IDX_1].data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_COUNT);

    status = scmi_system_power_req_elem_init(
        element_id,
        0,
        system_power_req_element_table[MOD_SCMI_SYS_POWER_REQ_IDX_1].data);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);

    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_IDX_1);
    fwk_id_get_element_idx_ExpectAndReturn(
        element_id, MOD_SCMI_SYS_POWER_REQ_IDX_1);

    status = scmi_system_power_req_elem_init(element_id, 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PANIC);
}

void test_function_scmi_system_power_req_bind(void)
{
    int status;

    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(
        FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ, MOD_SCMI_SYS_POWER_REQ_IDX_0);

    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL_REQ),
        &mod_ctx.scmi_api,
        FWK_SUCCESS);

    status = scmi_system_power_req_bind(fwk_module_id_scmi_system_power_req, 0);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    fwk_id_is_type_ExpectAnyArgsAndReturn(false);

    status = scmi_system_power_req_bind(element_id, 0);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    status = scmi_system_power_req_bind(element_id, 1);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_function_scmi_system_power_req_process_bind(void)
{
    int status;
    fwk_id_t invalid_id = { .value = UINT32_MAX };
    fwk_id_t target_id;
    struct mod_scmi_to_protocol_api *scmi_api;
    struct mod_system_requester_api *sys_api;

    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);
    fwk_id_build_module_id_ExpectAnyArgsAndReturn(invalid_id);

    status = scmi_system_power_req_bind_request(
        invalid_id,
        target_id,
        mod_sys_power_req_scmi_api_id,
        (const void **)&scmi_api);
    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);

    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    fwk_id_build_module_id_ExpectAnyArgsAndReturn(fwk_module_id_scmi);

    status = scmi_system_power_req_bind_request(
        fwk_module_id_scmi,
        target_id,
        mod_sys_power_req_scmi_api_id,
        (const void **)&scmi_api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL_PTR(
        scmi_api, &scmi_system_power_req_scmi_to_protocol_api);

    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);

    status = scmi_system_power_req_bind_request(
        fwk_module_id_scmi_system_power,
        target_id,
        mod_sys_power_req_api_id,
        (const void **)&sys_api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL_PTR(sys_api, &scmi_system_power_req_driver_api);

    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);

    status = scmi_system_power_req_bind_request(
        invalid_id, invalid_id, invalid_id, (const void **)NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void test_function_scmi_system_power_req_get_state(void)
{
    int status;
    uint32_t system_state;
    fwk_id_t id;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    status = scmi_system_power_req_get_state(id, &system_state);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(system_state, MOD_PD_STATE_ON);

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(1);
    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    status = scmi_system_power_req_get_state(id, &system_state);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(system_state, MOD_PD_STATE_OFF);

    status = scmi_system_power_req_get_state(id, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_function_scmi_system_power_req_set_state(void)
{
    int status;
    uint32_t flags = 0xAA;
    uint32_t state = MOD_PD_STATE_OFF;
    uint8_t scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_SYS_POWER;
    uint8_t scmi_message_id = (uint8_t)MOD_SCMI_SYS_POWER_REQ_STATE_SET;

    fwk_id_t id;

    const struct scmi_sys_power_req_state_set_a2p payload = {
        .flags = flags,
        .system_state = state,
    };
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_id_is_type_ExpectAnyArgsAndReturn(true);

    scmi_send_message_ExpectWithArrayAndReturn(
        scmi_message_id,
        scmi_protocol_id,
        0,
        dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_0].config->service_id,
        (const void *)&payload,
        sizeof(payload),
        sizeof(payload),
        true,
        FWK_SUCCESS);
    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    status = scmi_system_power_req_set_state(id, true, state, flags);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_function_scmi_system_power_req_set_state_no_response(void)
{
    int status;
    uint32_t flags = 0xAA;
    uint32_t state = MOD_PD_STATE_OFF;
    uint8_t scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_SYS_POWER;
    uint8_t scmi_message_id = (uint8_t)MOD_SCMI_SYS_POWER_REQ_STATE_SET;

    fwk_id_t id;

    const struct scmi_sys_power_req_state_set_a2p payload = {
        .flags = flags,
        .system_state = state,
    };
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(1);
    fwk_id_is_type_ExpectAnyArgsAndReturn(true);

    scmi_send_message_ExpectWithArrayAndReturn(
        scmi_message_id,
        scmi_protocol_id,
        0,
        dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_1].config->service_id,
        (const void *)&payload,
        sizeof(payload),
        sizeof(payload),
        true,
        FWK_SUCCESS);
    status = scmi_system_power_req_set_state(id, false, state, flags);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_scmi_system_power_req_state_set_handler(void)
{
    int status;
    uint32_t payload;

    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    payload = SCMI_HARDWARE_ERROR;
    status = scmi_system_power_req_state_set_handler(
        dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_0].config->service_id,
        &payload,
        sizeof(payload));
    TEST_ASSERT_EQUAL(status, SCMI_HARDWARE_ERROR);

    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    payload = SCMI_SUCCESS;
    status = scmi_system_power_req_state_set_handler(
        dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_0].config->service_id,
        &payload,
        sizeof(payload));
    TEST_ASSERT_EQUAL(status, SCMI_SUCCESS);

    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    payload = SCMI_SUCCESS;
    status = scmi_system_power_req_state_set_handler(
        dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_1].config->service_id,
        &payload,
        sizeof(payload));
    TEST_ASSERT_EQUAL(status, SCMI_SUCCESS);
}

void test_scmi_system_power_req_get_scmi_protocol_id(void)
{
    int status;
    fwk_id_t protocol_id;
    uint8_t scmi_protocol_id;
    status = scmi_system_power_req_get_scmi_protocol_id(
        protocol_id, &scmi_protocol_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(
        scmi_protocol_id, (uint8_t)MOD_SCMI_PROTOCOL_ID_SYS_POWER);
}

void test_scmi_system_power_req_message_handler(void)
{
    int status;
    fwk_id_t expected_service_id;
    fwk_id_t protocol_id;
    uint8_t payload[sizeof(struct scmi_sys_power_req_state_set_a2p)] = { 0 };

    expected_service_id =
        dev_ctx[MOD_SCMI_SYS_POWER_REQ_IDX_0].config->service_id;

    status = scmi_system_power_req_message_handler(
        protocol_id,
        expected_service_id,
        (uint32_t *)payload,
        sizeof(payload),
        sizeof(handler_table));
    TEST_ASSERT_EQUAL(status, FWK_E_RANGE);

    status = scmi_system_power_req_message_handler(
        protocol_id,
        expected_service_id,
        (uint32_t *)payload,
        sizeof(payload) + 1,
        (uint8_t)MOD_SCMI_SYS_POWER_REQ_STATE_SET);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);

    fake_message_handler_ExpectWithArrayAndReturn(
        expected_service_id,
        (uint32_t *)payload,
        sizeof(payload),
        sizeof(payload),
        FWK_SUCCESS);
    response_message_handler_ExpectAndReturn(expected_service_id, FWK_SUCCESS);

    status = scmi_system_power_req_message_handler(
        protocol_id,
        expected_service_id,
        (uint32_t *)payload,
        sizeof(payload),
        (uint8_t)MOD_SCMI_SYS_POWER_REQ_STATE_SET);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int scmi_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_function_scmi_system_power_req_init);
    RUN_TEST(test_function_scmi_system_power_req_elem_init_0);
    RUN_TEST(test_function_scmi_system_power_req_elem_init_1);
    RUN_TEST(test_function_scmi_system_power_req_bind);
    RUN_TEST(test_function_scmi_system_power_req_process_bind);
    RUN_TEST(test_function_scmi_system_power_req_get_state);
    RUN_TEST(test_function_scmi_system_power_req_set_state);
    RUN_TEST(test_function_scmi_system_power_req_set_state_no_response);
    RUN_TEST(test_scmi_system_power_req_state_set_handler);
    RUN_TEST(test_scmi_system_power_req_get_scmi_protocol_id);
    RUN_TEST(test_scmi_system_power_req_message_handler);
    return UNITY_END();
}

int main(void)
{
    return scmi_test_main();
}
