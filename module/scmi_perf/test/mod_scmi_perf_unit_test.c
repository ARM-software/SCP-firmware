/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <internal/scmi_perf.h>

#ifdef TEST_ON_TARGET
#    include <fwk_id.h>
#    include <fwk_module.h>
#else
#    include <Mockfwk_id.h>
#    include <Mockfwk_module.h>

#    include <internal/Mockfwk_core_internal.h>
#endif
#include <Mockmod_scmi_perf_extra.h>
#include <config_scmi_perf.h>

#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC

static struct mod_scmi_perf_ctx scmi_perf_ctx;

struct mod_scmi_to_protocol_api *to_protocol_api = NULL;

struct mod_scmi_from_protocol_api from_protocol_api = {
    .get_agent_count = mod_scmi_from_protocol_api_get_agent_count,
    .get_agent_id = mod_scmi_from_protocol_api_get_agent_id,
    .get_agent_type = mod_scmi_from_protocol_api_get_agent_type,
    .get_max_payload_size = mod_scmi_from_protocol_api_get_max_payload_size,
    .write_payload = mod_scmi_from_protocol_api_write_payload,
    .respond = mod_scmi_from_protocol_api_respond,
    .notify = mod_scmi_from_protocol_api_notify,
};

void setUp(void)
{
    scmi_perf_ctx.scmi_api = &from_protocol_api;
    scmi_perf_ctx.config = config_scmi_perf.data;
    scmi_perf_ctx.domain_count = scmi_perf_ctx.config->perf_doms_count;

    fwk_id_get_api_idx_ExpectAnyArgsAndReturn(MOD_SCMI_PERF_PROTOCOL_API);

    module_scmi_perf.process_bind_request(
        fwk_module_id_scmi,
        fwk_module_id_scmi_perf,
        FWK_ID_API(FWK_MODULE_IDX_SCMI_PERF, 0),
        (const void **)&to_protocol_api);
}

void tearDown(void)
{
}

/*
 * Test that the scmi_perf_protocol_version_handler function returns the correct
 * version of the protocol
 */

/* Check that the return values are as expected */
int version_handler_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_protocol_version_p2a *return_values;
    return_values = (struct scmi_protocol_version_p2a *)payload;

    TEST_ASSERT_EQUAL((int32_t)SCMI_SUCCESS, return_values->status);
    TEST_ASSERT_EQUAL(SCMI_PROTOCOL_VERSION_PERF, return_values->version);

    return FWK_SUCCESS;
}

void utest_scmi_perf_protocol_version_handler(void)
{
    int status;
    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    uint32_t payload = 0;

    /*
     * Whenever the mocked function mod_scmi_from_protocol_api_respond is
     * encountered, the function version_handler_respond_callback will be called
     */
    mod_scmi_from_protocol_api_respond_Stub(version_handler_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PROTOCOL_VERSION],
        MOD_SCMI_PROTOCOL_VERSION);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*
 * Test that the scmi_perf_protocol_attributes_handler function returns the
 * correct attributes associated with this protocol
 */

int attributes_handler_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_perf_protocol_attributes_p2a *return_values;
    return_values = (struct scmi_perf_protocol_attributes_p2a *)payload;

    TEST_ASSERT_EQUAL((int32_t)SCMI_SUCCESS, return_values->status);
    TEST_ASSERT_EQUAL(
        SCMI_PERF_PROTOCOL_ATTRIBUTES(true, scmi_perf_ctx.domain_count),
        return_values->attributes);
    TEST_ASSERT_EQUAL(0, return_values->statistics_len);

    return FWK_SUCCESS;
}

void utest_scmi_perf_protocol_attributes_handler(void)
{
    int status;
    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);
    struct scmi_protocol_attributes_p2a payload = {};

    mod_scmi_from_protocol_api_respond_Stub(
        attributes_handler_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PROTOCOL_ATTRIBUTES],
        MOD_SCMI_PROTOCOL_ATTRIBUTES);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/* Test the message_attributes_handler function with a valid message_id */

int message_attributes_handler_valid_param_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_protocol_message_attributes_p2a *return_values;
    return_values = (struct scmi_protocol_message_attributes_p2a *)payload;

    TEST_ASSERT_EQUAL((int32_t)SCMI_SUCCESS, return_values->status);

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    TEST_ASSERT_EQUAL(1, return_values->attributes);
#else
    TEST_ASSERT_EQUAL(0, return_values->attributes);
#endif

    return FWK_SUCCESS;
}

void utest_scmi_perf_protocol_message_attributes_handler_valid_param(void)
{
    int status;
    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_protocol_message_attributes_a2p payload = {
        .message_id = MOD_SCMI_PERF_LIMITS_GET,
    };

    mod_scmi_from_protocol_api_respond_Stub(
        message_attributes_handler_valid_param_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES],
        MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/* Test the message_attributes_handler function with an invalid message_id */

int message_attributes_handler_invalid_param_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_protocol_message_attributes_p2a *return_values;
    return_values = (struct scmi_protocol_message_attributes_p2a *)payload;

    TEST_ASSERT_EQUAL((int32_t)SCMI_NOT_FOUND, return_values->status);

    return FWK_SUCCESS;
}

void utest_scmi_perf_protocol_message_attributes_handler_invalid_param(void)
{
    int status;
    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_protocol_message_attributes_a2p payload = {
        .message_id = FWK_ARRAY_SIZE(handler_table),
    };

    mod_scmi_from_protocol_api_respond_Stub(
        message_attributes_handler_invalid_param_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES],
        MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

int scmi_perf_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_scmi_perf_protocol_version_handler);
    RUN_TEST(utest_scmi_perf_protocol_attributes_handler);
    RUN_TEST(utest_scmi_perf_protocol_message_attributes_handler_valid_param);
    RUN_TEST(utest_scmi_perf_protocol_message_attributes_handler_invalid_param);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_perf_test_main();
}
#endif
