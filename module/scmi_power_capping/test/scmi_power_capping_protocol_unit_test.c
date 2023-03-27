/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "string.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockfwk_notification.h>
#include <Mockmod_power_allocator_extra.h>
#include <Mockmod_scmi_extra.h>
#include <internal/Mockfwk_core_internal.h>

#include <mod_scmi_power_capping_unit_test.h>

#include <stdarg.h>

#include UNIT_TEST_SRC

#define EXPECT_RESPONSE(ret_payload, ret_payload_size) \
    respond_ExpectWithArrayAndReturn( \
        service_id_1, \
        (void *)&ret_payload, \
        ret_payload_size, \
        ret_payload_size, \
        FWK_SUCCESS)

#define EXPECT_RESPONSE_SUCCESS(ret_payload) \
    EXPECT_RESPONSE(ret_payload, sizeof(ret_payload))

#define EXPECT_RESPONSE_ERROR(ret_payload) \
    EXPECT_RESPONSE(ret_payload, sizeof(ret_payload.status))

#define TEST_SCMI_COMMAND_NO_PAYLOAD(message_id) \
    do { \
        status = scmi_power_capping_message_handler( \
            dummy_protocol_id, \
            service_id_1, \
            (void *)&dummy_protocol_id, \
            0, \
            message_id); \
        TEST_ASSERT_EQUAL(status, FWK_SUCCESS); \
    } while (0)

#define TEST_SCMI_COMMAND(message_id, cmd_payload) \
    do { \
        status = scmi_power_capping_message_handler( \
            dummy_protocol_id, \
            service_id_1, \
            (void *)&cmd_payload, \
            sizeof(cmd_payload), \
            message_id); \
        TEST_ASSERT_EQUAL(status, FWK_SUCCESS); \
    } while (0)

static int status;

static const struct mod_scmi_from_protocol_api scmi_api = {
    .respond = respond,
    .get_agent_id = get_agent_id,
};

static const struct mod_power_allocator_api power_allocator_api = {
    .get_cap = get_cap,
    .set_cap = set_cap,
};

static const struct mod_scmi_power_capping_power_apis power_management_apis = {
    .power_allocator_api = &power_allocator_api,
};

static fwk_id_t service_id_1 =
    FWK_ID_ELEMENT_INIT(FAKE_SCMI_MODULE_ID, FAKE_SERVICE_IDX_1);
static fwk_id_t dummy_protocol_id;
static uint32_t dummy_payload;

static const struct mod_scmi_power_capping_domain_config
    scmi_power_capping_default_config = {
        .parent_idx = __LINE__,
        .power_allocator_domain_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_ALLOCATOR, __LINE__),
        .min_power_cap = MIN_DEFAULT_POWER_CAP,
        .max_power_cap = MAX_DEFAULT_POWER_CAP,
        .power_cap_step = 1,
    };

static const struct mod_scmi_power_capping_domain_config
    scmi_power_capping_config_1 = {
        .parent_idx = __LINE__,
        .power_allocator_domain_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_ALLOCATOR, __LINE__),
        .min_power_cap = MIN_DEFAULT_POWER_CAP,
        .max_power_cap = MIN_DEFAULT_POWER_CAP,
    };

static struct mod_scmi_power_capping_domain_context
    domain_ctx_table[FAKE_POWER_CAPPING_IDX_COUNT];

/* Helper functions */
static void test_set_domain_service_id(
    unsigned int domain_idx,
    fwk_id_t service_id)
{
    domain_ctx_table[domain_idx].cap_pending_service_id = service_id;
}

static void test_set_cap_config_supported(
    unsigned int domain_idx,
    bool cap_config_supported)
{
    domain_ctx_table[domain_idx].cap_config_support = cap_config_supported;
}

/* Test functions */
/* Initialize the tests */
static void test_init(void)
{
    pcapping_protocol_ctx.scmi_api = &scmi_api;
    pcapping_protocol_ctx.power_management_apis = &power_management_apis;
    pcapping_protocol_ctx.power_capping_domain_count =
        FAKE_POWER_CAPPING_IDX_COUNT;
}

void setUp(void)
{
    status = FWK_E_STATE;
    pcapping_protocol_ctx.power_capping_domain_ctx_table = domain_ctx_table;
    memset(domain_ctx_table, 0u, sizeof(domain_ctx_table));
    for (unsigned int domain_id = 0; domain_id < FAKE_POWER_CAPPING_IDX_COUNT;
         domain_id++) {
        domain_ctx_table[domain_id].config = &scmi_power_capping_default_config;
        test_set_domain_service_id(domain_id, FWK_ID_NONE);
    }
}

void tearDown(void)
{
    Mockmod_power_allocator_extra_Verify();
    Mockmod_scmi_extra_Verify();
}

void utest_get_scmi_protocol_id(void)
{
    uint8_t scmi_protocol_id;

    status = scmi_power_capping_get_scmi_protocol_id(
        dummy_protocol_id, &scmi_protocol_id);

    TEST_ASSERT_EQUAL(scmi_protocol_id, MOD_SCMI_PROTOCOL_ID_POWER_CAPPING);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_message_handler_cmd_error(void)
{
    struct scmi_protocol_version_p2a ret_payload = {
        .status = SCMI_PROTOCOL_ERROR,
    };

    EXPECT_RESPONSE_ERROR(ret_payload);
    /* Add payload to a command that normally doesn't expect a payload */
    TEST_SCMI_COMMAND(MOD_SCMI_PROTOCOL_VERSION, dummy_payload);
}

void utest_message_handler_invalid_cmd(void)
{
    struct scmi_protocol_version_p2a ret_payload = {
        .status = SCMI_NOT_FOUND,
    };

    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND_NO_PAYLOAD(MOD_SCMI_POWER_CAPPING_COMMAND_COUNT);
}

void utest_message_handler_un_implemented_message(void)
{
    int (*temp_handle)(fwk_id_t, const uint32_t *);
    struct scmi_protocol_version_p2a ret_payload = {
        .status = SCMI_NOT_SUPPORTED,
    };

    temp_handle = handler_table[MOD_SCMI_PROTOCOL_VERSION];
    handler_table[MOD_SCMI_PROTOCOL_VERSION] = NULL;

    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND_NO_PAYLOAD(MOD_SCMI_PROTOCOL_VERSION);

    handler_table[MOD_SCMI_PROTOCOL_VERSION] = temp_handle;
}

void utest_message_handler_protocol_version(void)
{
    struct scmi_protocol_version_p2a ret_payload = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_POWER_CAPPING,
    };

    EXPECT_RESPONSE_SUCCESS(ret_payload);
    TEST_SCMI_COMMAND_NO_PAYLOAD(MOD_SCMI_PROTOCOL_VERSION);
}

void utest_message_handler_protocol_attributes(void)
{
    struct scmi_protocol_attributes_p2a ret_payload = {
        .status = SCMI_SUCCESS,
        .attributes = FAKE_POWER_CAPPING_IDX_COUNT,
    };

    EXPECT_RESPONSE_SUCCESS(ret_payload);
    TEST_SCMI_COMMAND_NO_PAYLOAD(MOD_SCMI_PROTOCOL_ATTRIBUTES);
}

void utest_message_handler_protocol_msg_attributes_unsupported_msgs(void)
{
    struct scmi_protocol_message_attributes_a2p cmd_payload = {
        .message_id = MOD_SCMI_POWER_CAPPING_COMMAND_COUNT
    };
    struct scmi_protocol_message_attributes_p2a ret_payload = {
        .status = SCMI_NOT_FOUND
    };
    /* Test unsupported messages */
    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES, cmd_payload);
}

void utest_message_handler_protocol_msg_attributes_maxlimits(void)
{
    struct scmi_protocol_message_attributes_a2p cmd_payload = {
        .message_id = UINT32_MAX
    };
    struct scmi_protocol_message_attributes_p2a ret_payload = {
        .status = SCMI_NOT_FOUND
    };

    /* Test unsupported messages */
    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES, cmd_payload);
}

void utest_message_handler_protocol_msg_attributes_supported_msgs(void)
{
    uint32_t message_id;
    struct scmi_protocol_message_attributes_a2p cmd_payload;
    struct scmi_protocol_message_attributes_p2a ret_payload = {
        .status = SCMI_SUCCESS
    };
    /* Test all supported messages */

    for (message_id = 0; message_id < MOD_SCMI_POWER_CAPPING_PAI_GET;
         message_id++) {
        cmd_payload.message_id = message_id;
        EXPECT_RESPONSE_SUCCESS(ret_payload);
        TEST_SCMI_COMMAND(MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES, cmd_payload);
    }
}

void utest_message_handler_domain_invalid(void)
{
    struct scmi_power_capping_domain_attributes_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_COUNT
    };

    struct scmi_power_capping_domain_attributes_p2a ret_payload = {
        .status = SCMI_NOT_FOUND,
    };

    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_DOMAIN_ATTRIBUTES, cmd_payload);
}

void utest_message_handler_domain_attributes_valid(void)
{
    struct mod_scmi_power_capping_domain_context *domain_ctx =
        &domain_ctx_table[FAKE_POWER_CAPPING_IDX_1];
    const struct mod_scmi_power_capping_domain_config *config =
        domain_ctx->config;

    struct scmi_power_capping_domain_attributes_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1
    };

    domain_ctx->cap_config_support = true;

    struct scmi_power_capping_domain_attributes_p2a ret_payload = {
        .status = SCMI_SUCCESS,
        .attributes = 1u << POWER_CAP_CONF_SUP_POS |
            config->power_cap_unit << POWER_UNIT_POS,
        .name = "TestPowerCap",
        .min_power_cap = config->min_power_cap,
        .max_power_cap = config->max_power_cap,
        .power_cap_step = config->power_cap_step,
        .max_sustainable_power = config->max_sustainable_power,
        .parent_id = config->parent_idx,
    };

    fwk_module_get_element_name_ExpectAndReturn(
        FWK_ID_ELEMENT(
            FWK_MODULE_IDX_SCMI_POWER_CAPPING, cmd_payload.domain_id),
        (char *)ret_payload.name);

    EXPECT_RESPONSE_SUCCESS(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_DOMAIN_ATTRIBUTES, cmd_payload);
}

void utest_message_handler_power_capping_get_valid(void)
{
    uint32_t cap = __LINE__; /* Arbitrary value */

    struct scmi_power_capping_cap_get_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1
    };

    struct scmi_power_capping_cap_get_p2a ret_payload = {
        .status = SCMI_SUCCESS,
        .power_cap = cap,
    };

    get_cap_ExpectWithArrayAndReturn(
        scmi_power_capping_default_config.power_allocator_domain_id,
        &cap,
        sizeof(cap),
        FWK_SUCCESS);
    get_cap_IgnoreArg_cap();
    get_cap_ReturnMemThruPtr_cap(&cap, sizeof(cap));

    EXPECT_RESPONSE_SUCCESS(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_GET, cmd_payload);
}

void utest_message_handler_power_capping_get_failure(void)
{
    uint32_t cap = __LINE__; /* Arbitrary value */

    struct scmi_power_capping_cap_get_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1
    };

    struct scmi_power_capping_cap_get_p2a ret_payload = {
        .status = SCMI_GENERIC_ERROR,
    };

    get_cap_ExpectWithArrayAndReturn(
        scmi_power_capping_default_config.power_allocator_domain_id,
        &cap,
        sizeof(cap),
        FWK_E_DEVICE);
    get_cap_IgnoreArg_cap();

    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_GET, cmd_payload);
}

void utest_message_handler_power_capping_set_invalid_flags(void)
{
    struct scmi_power_capping_cap_set_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1,
        .flags = ~(ASYNC_FLAG(1) | IGN_DEL_RESP_FLAG(1)),
    };

    struct scmi_power_capping_cap_set_p2a ret_payload = {
        .status = SCMI_INVALID_PARAMETERS,
    };

    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_SET, cmd_payload);
}

void utest_message_handler_power_capping_set_config_not_supported(void)
{
    struct scmi_power_capping_cap_set_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1,
        .flags = ASYNC_FLAG(1) | IGN_DEL_RESP_FLAG(1),
    };

    struct scmi_power_capping_cap_set_p2a ret_payload = {
        .status = SCMI_NOT_SUPPORTED,
    };

    domain_ctx_table[FAKE_POWER_CAPPING_IDX_1].config =
        &scmi_power_capping_config_1;

    EXPECT_RESPONSE_SUCCESS(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_SET, cmd_payload);
}

void utest_message_handler_power_capping_set_async_del_not_supported(void)
{
    struct scmi_power_capping_cap_set_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_2,
        .flags = (ASYNC_FLAG(1) | IGN_DEL_RESP_FLAG(0)),
    };

    struct scmi_power_capping_cap_set_p2a ret_payload = {
        .status = SCMI_NOT_SUPPORTED,
    };

    EXPECT_RESPONSE_SUCCESS(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_SET, cmd_payload);
}

void utest_message_handler_power_capping_set_domain_busy(void)
{
    uint32_t cap = MIN_DEFAULT_POWER_CAP;
    struct scmi_power_capping_cap_set_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1,
        .power_cap = cap,
        .flags = ASYNC_FLAG(0),
    };

    struct scmi_power_capping_cap_set_p2a ret_payload = {
        .status = SCMI_BUSY,
    };

    test_set_domain_service_id(cmd_payload.domain_id, service_id_1);

    test_set_cap_config_supported(cmd_payload.domain_id, true);

    fwk_id_is_equal_ExpectAndReturn(service_id_1, FWK_ID_NONE, false);

    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_SET, cmd_payload);
}

void utest_message_handler_power_capping_set_less_than_min_cap(void)
{
    uint32_t cap = MIN_DEFAULT_POWER_CAP - 1u;
    struct scmi_power_capping_cap_set_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1,
        .power_cap = cap,
        .flags = ASYNC_FLAG(0),
    };

    struct scmi_power_capping_cap_set_p2a ret_payload = {
        .status = SCMI_OUT_OF_RANGE,
    };

    test_set_cap_config_supported(cmd_payload.domain_id, true);

    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_SET, cmd_payload);
}

void utest_message_handler_power_capping_set_more_than_max_cap(void)
{
    uint32_t cap = MAX_DEFAULT_POWER_CAP + 1u;
    struct scmi_power_capping_cap_set_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1,
        .power_cap = cap,
        .flags = ASYNC_FLAG(0),
    };

    struct scmi_power_capping_cap_set_p2a ret_payload = {
        .status = SCMI_OUT_OF_RANGE,
    };

    test_set_cap_config_supported(cmd_payload.domain_id, true);

    EXPECT_RESPONSE_ERROR(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_SET, cmd_payload);
}

void utest_message_handler_power_capping_set_success_pending(void)
{
    uint32_t cap = MAX_DEFAULT_POWER_CAP;
    int status;

    struct scmi_power_capping_cap_set_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1,
        .power_cap = cap,
        .flags = ASYNC_FLAG(0),
    };

    test_set_cap_config_supported(cmd_payload.domain_id, true);

    set_cap_ExpectAndReturn(
        scmi_power_capping_default_config.power_allocator_domain_id,
        cmd_payload.power_cap,
        FWK_PENDING);

    fwk_id_is_equal_ExpectAndReturn(FWK_ID_NONE, FWK_ID_NONE, true);

    status = scmi_power_capping_message_handler(
        dummy_protocol_id,
        service_id_1,
        (void *)&cmd_payload,
        sizeof(cmd_payload),
        MOD_SCMI_POWER_CAPPING_CAP_SET);

    TEST_ASSERT_EQUAL_UINT32(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL_UINT32(
        domain_ctx_table[cmd_payload.domain_id].cap_pending_service_id.value,
        service_id_1.value);
}

void utest_message_handler_power_capping_set_success_sync(void)
{
    uint32_t cap = MIN_DEFAULT_POWER_CAP;
    struct scmi_power_capping_cap_set_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1,
        .power_cap = cap,
        .flags = ASYNC_FLAG(0),
    };

    struct scmi_power_capping_cap_set_p2a ret_payload = {
        .status = SCMI_SUCCESS,
    };

    test_set_cap_config_supported(cmd_payload.domain_id, true);

    set_cap_ExpectAndReturn(
        scmi_power_capping_default_config.power_allocator_domain_id,
        cmd_payload.power_cap,
        FWK_SUCCESS);

    fwk_id_is_equal_ExpectAndReturn(FWK_ID_NONE, FWK_ID_NONE, true);

    EXPECT_RESPONSE_SUCCESS(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_SET, cmd_payload);
}

void utest_message_handler_power_capping_set_success_sync_uncap(void)
{
    uint32_t cap = DISABLE_CAP_VALUE;
    struct scmi_power_capping_cap_set_a2p cmd_payload = {
        .domain_id = FAKE_POWER_CAPPING_IDX_1,
        .power_cap = cap,
        .flags = ASYNC_FLAG(0),
    };

    struct scmi_power_capping_cap_set_p2a ret_payload = {
        .status = SCMI_SUCCESS,
    };

    test_set_cap_config_supported(cmd_payload.domain_id, true);

    set_cap_ExpectAndReturn(
        scmi_power_capping_default_config.power_allocator_domain_id,
        cmd_payload.power_cap,
        FWK_SUCCESS);

    fwk_id_is_equal_ExpectAndReturn(FWK_ID_NONE, FWK_ID_NONE, true);

    EXPECT_RESPONSE_SUCCESS(ret_payload);
    TEST_SCMI_COMMAND(MOD_SCMI_POWER_CAPPING_CAP_SET, cmd_payload);
}

void utest_pcapping_protocol_init(void)
{
    int domain_count = __LINE__;
    struct mod_scmi_power_capping_context ctx;
    struct mod_scmi_power_capping_domain_context *fake_table_pointer =
        (struct mod_scmi_power_capping_domain_context *)__LINE__;

    ctx.domain_count = domain_count;
    ctx.power_capping_domain_ctx_table = fake_table_pointer;
    pcapping_protocol_init(&ctx);
    TEST_ASSERT_EQUAL(
        pcapping_protocol_ctx.power_capping_domain_count, domain_count);
    TEST_ASSERT_EQUAL(
        pcapping_protocol_ctx.power_capping_domain_ctx_table,
        fake_table_pointer);
}

void utest_pcapping_protocol_domain_init_success(void)
{
    int status;

    status = pcapping_protocol_domain_init(
        FAKE_POWER_CAPPING_IDX_2, &scmi_power_capping_config_1);
    TEST_ASSERT_EQUAL_PTR(
        domain_ctx_table[FAKE_POWER_CAPPING_IDX_2].config,
        &scmi_power_capping_config_1);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_pcapping_protocol_domain_init_failure(void)
{
    int status;
    int domain_idx = __LINE__;
    pcapping_protocol_ctx.power_capping_domain_count = domain_idx;

    status =
        pcapping_protocol_domain_init(domain_idx, &scmi_power_capping_config_1);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_pcapping_protocol_bind_scmi_failure(void)
{
    int status;

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &(pcapping_protocol_ctx.scmi_api),
        FWK_E_DEVICE);

    status = pcapping_protocol_bind();
    TEST_ASSERT_EQUAL(status, FWK_E_DEVICE);
}

void utest_pcapping_protocol_bind(void)
{
    int status;

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &(pcapping_protocol_ctx.scmi_api),
        FWK_SUCCESS);

    status = pcapping_protocol_bind();
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_pcapping_protocol_start_module(void)
{
    int status;
    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING);

    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, true);

    status = pcapping_protocol_start(module_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_pcapping_protocol_start_element(void)
{
    int status;
    const unsigned int element_idx = 0u;
    fwk_id_t element_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING, element_idx);

    fwk_id_get_element_idx_ExpectAndReturn(element_id, element_idx);

    fwk_id_is_type_ExpectAndReturn(element_id, FWK_ID_TYPE_MODULE, false);

    fwk_notification_subscribe_ExpectAndReturn(
        FWK_ID_NOTIFICATION(
            FWK_MODULE_IDX_POWER_ALLOCATOR,
            MOD_POWER_ALLOCATOR_NOTIFICATION_IDX_CAP_CHANGED),
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_ALLOCATOR),
        element_id,
        FWK_SUCCESS);

    status = pcapping_protocol_start(element_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_pcapping_protocol_process_notification(void)
{
    int status;
    const unsigned int element_idx = 0u;

    struct fwk_event notification_event = {
        .target_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING, element_idx),
    };

    domain_ctx_table[element_idx].cap_pending_service_id = service_id_1;

    struct scmi_power_capping_cap_set_p2a ret_payload = {
        .status = SCMI_SUCCESS,
    };

    fwk_id_get_element_idx_ExpectAndReturn(
        notification_event.target_id, element_idx);

    fwk_id_is_equal_ExpectAndReturn(service_id_1, FWK_ID_NONE, false);

    EXPECT_RESPONSE_SUCCESS(ret_payload);

    status = pcapping_protocol_process_notification(&notification_event);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(
        domain_ctx_table[element_idx].cap_pending_service_id.value,
        FWK_ID_NONE.value);
}

void utest_pcapping_protocol_process_bind_request_success(void)
{
    int status;
    const void *api;
    fwk_id_t api_id = FWK_ID_API(
        FWK_MODULE_IDX_SCMI_POWER_CAPPING,
        MOD_SCMI_POWER_CAPPING_API_IDX_REQUEST);

    fwk_id_is_equal_ExpectAndReturn(api_id, api_id, true);

    status = pcapping_protocol_process_bind_request(api_id, &api);
    TEST_ASSERT_EQUAL_PTR(&scmi_power_capping_mod_scmi_to_protocol_api, api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_pcapping_protocol_process_bind_request_failure(void)
{
    int status;
    const void *api;
    fwk_id_t api_id_invalid;
    fwk_id_t api_id_valid = FWK_ID_API(
        FWK_MODULE_IDX_SCMI_POWER_CAPPING,
        MOD_SCMI_POWER_CAPPING_API_IDX_REQUEST);

    fwk_id_is_equal_ExpectAndReturn(api_id_invalid, api_id_valid, false);

    status = pcapping_protocol_process_bind_request(api_id_invalid, &api);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

int scmi_power_capping_protocol_test_main(void)
{
    test_init();
    RUN_TEST(utest_get_scmi_protocol_id);
    RUN_TEST(utest_message_handler_cmd_error);
    RUN_TEST(utest_message_handler_invalid_cmd);
    RUN_TEST(utest_message_handler_protocol_version);
    RUN_TEST(utest_message_handler_protocol_attributes);
    RUN_TEST(utest_message_handler_protocol_msg_attributes_unsupported_msgs);
    RUN_TEST(utest_message_handler_protocol_msg_attributes_maxlimits);
    RUN_TEST(utest_message_handler_protocol_msg_attributes_supported_msgs);
    RUN_TEST(utest_message_handler_domain_invalid);
    RUN_TEST(utest_message_handler_domain_attributes_valid);
    RUN_TEST(utest_message_handler_power_capping_get_valid);
    RUN_TEST(utest_message_handler_power_capping_get_failure);
    RUN_TEST(utest_message_handler_power_capping_set_invalid_flags);
    RUN_TEST(utest_message_handler_power_capping_set_config_not_supported);
    RUN_TEST(utest_message_handler_power_capping_set_async_del_not_supported);
    RUN_TEST(utest_message_handler_power_capping_set_domain_busy);
    RUN_TEST(utest_message_handler_power_capping_set_less_than_min_cap);
    RUN_TEST(utest_message_handler_power_capping_set_more_than_max_cap);
    RUN_TEST(utest_message_handler_power_capping_set_success_pending);
    RUN_TEST(utest_message_handler_power_capping_set_success_sync);
    RUN_TEST(utest_message_handler_power_capping_set_success_sync_uncap);
    RUN_TEST(utest_message_handler_un_implemented_message);
    RUN_TEST(utest_pcapping_protocol_init);
    RUN_TEST(utest_pcapping_protocol_domain_init_success);
    RUN_TEST(utest_pcapping_protocol_domain_init_failure);
    RUN_TEST(utest_pcapping_protocol_bind_scmi_failure);
    RUN_TEST(utest_pcapping_protocol_bind);
    RUN_TEST(utest_pcapping_protocol_start_module);
    RUN_TEST(utest_pcapping_protocol_start_element);
    RUN_TEST(utest_pcapping_protocol_process_notification);
    RUN_TEST(utest_pcapping_protocol_process_bind_request_success);
    RUN_TEST(utest_pcapping_protocol_process_bind_request_failure);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_power_capping_protocol_test_main();
}
#endif
