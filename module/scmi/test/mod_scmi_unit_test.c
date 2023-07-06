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

#include <Mockmod_scmi_extra.h>

#include <internal/mod_scmi.h>

#include <mod_scmi.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC
#include <mod_scmi_base.c>

#define BUILD_HAS_BASE_PROTOCOL

#define FAKE_MODULE_ID 0x5

enum fake_services {
    FAKE_SERVICE_IDX_PSCI,
    FAKE_SERVICE_IDX_OSPM,
    FAKE_SERVICE_IDX_COUNT,
};

#define FAKE_API_IDX_SCMI_TRANSPORT 0x3
#define FAKE_SCMI_AGENT_IDX_PSCI    0x6
#define FAKE_SCMI_AGENT_IDX_OSPM    0x7

static const struct fwk_element element_table[] = {
    [FAKE_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &(struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FAKE_MODULE_ID,
                FAKE_SERVICE_IDX_PSCI),
            .transport_api_id = FWK_ID_API_INIT(
                FAKE_MODULE_ID,
                FAKE_API_IDX_SCMI_TRANSPORT),
            .scmi_agent_id = (unsigned int) FAKE_SCMI_AGENT_IDX_PSCI,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        },
    },

    [FAKE_SERVICE_IDX_OSPM] = {
        .name = "OSPM-A2P-0",
        .data = &(struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FAKE_MODULE_ID,
                FAKE_SERVICE_IDX_OSPM),
            .transport_api_id = FWK_ID_API_INIT(
                FAKE_MODULE_ID,
                FAKE_API_IDX_SCMI_TRANSPORT),
            .scmi_agent_id = (unsigned int) FAKE_SCMI_AGENT_IDX_OSPM,
            .scmi_p2a_id = FWK_ID_NONE_INIT,
        },
    },

    [FAKE_SERVICE_IDX_COUNT] = { 0 },

};

static const struct mod_scmi_agent agent_table[] = {
    [FAKE_SCMI_AGENT_IDX_OSPM] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "OSPM",
    },
    [FAKE_SCMI_AGENT_IDX_PSCI] = {
        .type = SCMI_AGENT_TYPE_PSCI,
        .name = "PSCI",
    },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_scmi = {
    .data =
        &(struct mod_scmi_config){
            .protocol_count_max = 6,
            .agent_count = FWK_ARRAY_SIZE(agent_table) - 1,
            .agent_table = agent_table,
            .vendor_identifier = "arm",
            .sub_vendor_identifier = "arm",
        },

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

struct mod_scmi_to_protocol_api to_protocol_api = {
    .get_scmi_protocol_id = test_get_scmi_protocol_id,
    .message_handler = &test_mod_scmi_message_handler,
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    .notification_handler = &test_mod_scmi_message_handler,
#endif
};

static struct mod_scmi_to_transport_api to_transport_api = {
    .get_secure = mod_scmi_to_transport_api_get_secure,
    .get_max_payload_size = mod_scmi_to_transport_api_get_max_payload_size,
    .get_message_header = mod_scmi_to_transport_api_get_message_header,
    .get_payload = mod_scmi_to_transport_api_get_payload,
    .write_payload = mod_scmi_to_transport_api_write_payload,
    .respond = mod_scmi_to_transport_api_respond,
    .transmit = mod_scmi_to_transport_api_transmit,
    .release_transport_channel_lock =
        mod_scmi_to_transport_api_release_transport_channel_lock,
};

static const struct mod_scmi_from_protocol_api from_protocol_api = {
    .get_agent_count = mod_scmi_from_protocol_get_agent_count,
    .get_agent_id = mod_scmi_from_protocol_get_agent_id,
    .get_agent_type = mod_scmi_from_protocol_get_agent_type,
    .get_max_payload_size = mod_scmi_from_protocol_get_max_payload_size,
    .write_payload = mod_scmi_from_protocol_write_payload,
    .respond = mod_scmi_from_protocol_respond,
    .notify = mod_scmi_from_protocol_notify,
};

void setUp(void)
{
    struct scmi_service_ctx *ctx;
    const struct mod_scmi_to_transport_api *transport_api = &to_transport_api;
    scmi_ctx.config = (struct mod_scmi_config *)config_scmi.data;
    scmi_ctx.protocol_table = fwk_mm_calloc(
        scmi_ctx.config->protocol_count_max +
            PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT,
        sizeof(scmi_ctx.protocol_table[0]));
    scmi_ctx.protocol_table[PROTOCOL_TABLE_BASE_PROTOCOL_IDX].message_handler =
        scmi_base_message_handler;
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    scmi_ctx.protocol_table[PROTOCOL_TABLE_BASE_PROTOCOL_IDX]
        .notification_handler = test_mod_scmi_message_handler;
#endif
    scmi_ctx.service_ctx_table = fwk_mm_calloc(
        FAKE_SERVICE_IDX_COUNT, sizeof(scmi_ctx.service_ctx_table[0]));
    scmi_ctx.scmi_protocol_id_to_idx[MOD_SCMI_PROTOCOL_ID_BASE] =
        PROTOCOL_TABLE_BASE_PROTOCOL_IDX;

    scmi_base_set_shared_ctx(&scmi_ctx);
    scmi_base_set_api(&from_protocol_api);

    ctx = &scmi_ctx.service_ctx_table[FAKE_SERVICE_IDX_PSCI];
    ctx->config =
        (struct mod_scmi_service_config *)element_table[FAKE_SERVICE_IDX_PSCI]
            .data;
    ctx->transport_api = transport_api;
    ctx->transport_id = ctx->config->transport_id;
    ctx->respond = transport_api->respond;
    ctx->transmit = transport_api->transmit;

    ctx = &scmi_ctx.service_ctx_table[FAKE_SERVICE_IDX_OSPM];
    ctx->config =
        (struct mod_scmi_service_config *)element_table[FAKE_SERVICE_IDX_OSPM]
            .data;
    ctx->transport_api = transport_api;
    ctx->transport_id = ctx->config->transport_id;
    ctx->respond = transport_api->respond;
    ctx->transmit = transport_api->transmit;
}

void tearDown(void)
{
}

void test_function_get_max_payload_size_invalid_param(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(FAKE_MODULE_ID, FAKE_SERVICE_IDX_OSPM);

    status = get_max_payload_size(service_id, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_function_get_max_payload_size_valid_param(void)
{
    int status;
    size_t size;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(FAKE_MODULE_ID, FAKE_SERVICE_IDX_OSPM);
#if !defined(TEST_ON_TARGET)
    fwk_id_get_element_idx_ExpectAndReturn(service_id, FAKE_SERVICE_IDX_OSPM);
#endif

    mod_scmi_to_transport_api_get_max_payload_size_ExpectAnyArgsAndReturn(
        FWK_SUCCESS);

    status = get_max_payload_size(service_id, &size);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_function_scmi_base_discover_sub_vendor_handler(void)
{
    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(FAKE_MODULE_ID, FAKE_SERVICE_IDX_OSPM);

#if !defined(TEST_ON_TARGET)
    fwk_id_get_element_idx_ExpectAndReturn(service_id, FAKE_SERVICE_IDX_OSPM);
    fwk_module_get_element_name_ExpectAndReturn(service_id, "OSPM");
#endif

    mod_scmi_from_protocol_respond_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    TEST_ASSERT_EQUAL(
        FWK_SUCCESS, scmi_base_discover_sub_vendor_handler(service_id, NULL));
}

void test_send_to_message_handler(void)
{
    int status;
    struct scmi_protocol *protocol;
    struct scmi_service_ctx ctx;

    uint32_t payload = FWK_SUCCESS;

    struct fwk_event event = {
        .source_id = FWK_ID_NONE,
        .target_id = FWK_ID_ELEMENT_INIT(FAKE_MODULE_ID, FAKE_SERVICE_IDX_OSPM),
        .id = FWK_ID_NONE,
    };

    ctx.scmi_message_id = 0x00;
    ctx.scmi_message_type = MOD_SCMI_MESSAGE_TYPE_COMMAND;
    ctx.scmi_protocol_id = 0x12;
    protocol = &scmi_ctx.protocol_table[PROTOCOL_TABLE_BASE_PROTOCOL_IDX];

    mod_scmi_from_protocol_respond_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    test_mod_scmi_notification_message_handler_ExpectAnyArgsAndReturn(
        FWK_SUCCESS);
    status = send_to_message_handler(
        &ctx, protocol, (const uint32_t *)&payload, sizeof(payload), &event);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_send_to_notification_handler(void)
{
    int status;
    struct scmi_protocol *protocol;
    struct scmi_service_ctx ctx;

    uint32_t payload = FWK_SUCCESS;

    struct fwk_event event = {
        .source_id = FWK_ID_NONE,
        .target_id = FWK_ID_ELEMENT_INIT(FAKE_MODULE_ID, FAKE_SERVICE_IDX_OSPM),
        .id = FWK_ID_NONE,
    };

    ctx.scmi_message_id = 0x00;
    ctx.scmi_message_type = MOD_SCMI_MESSAGE_TYPE_NOTIFICATION;
    ctx.scmi_protocol_id = 0x12;
    protocol = &scmi_ctx.protocol_table[PROTOCOL_TABLE_BASE_PROTOCOL_IDX];

    mod_scmi_from_protocol_respond_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    test_mod_scmi_notification_message_handler_ExpectAnyArgsAndReturn(
        FWK_SUCCESS);
    status = send_to_message_handler(
        &ctx, protocol, (const uint32_t *)&payload, sizeof(payload), &event);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int scmi_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_function_scmi_base_discover_sub_vendor_handler);
    RUN_TEST(test_function_get_max_payload_size_invalid_param);
    RUN_TEST(test_function_get_max_payload_size_valid_param);

    RUN_TEST(test_send_to_message_handler);
    RUN_TEST(test_send_to_notification_handler);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_test_main();
}
#endif
