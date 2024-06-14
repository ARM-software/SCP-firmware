/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_scmi_power_domain_ut.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_core.h>
#include <Mockfwk_id.h>
#include <Mockfwk_module.h>
#include <Mockmod_scmi_power_domain_extra.h>
#include <internal/Mockfwk_core_internal.h>

#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_scmi_power_domain.h>
#include <mod_scmi_power_domain_extra.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_mm.h>

#include UNIT_TEST_SRC

#define TEST_MODULE_IDX           0x5
#define TEST_DEVICE_PD_ATTRIBUTES SCMI_PD_POWER_STATE_SET_SYNC

#define EXPECT_SCMI_RESPONSE(service_id, resp_payload, resp_payload_size) \
    mod_scmi_from_protocol_api_respond_ExpectWithArrayAndReturn( \
        service_id, \
        (void *)&resp_payload, \
        resp_payload_size, \
        resp_payload_size, \
        FWK_SUCCESS)

#define TEST_SCMI_COMMAND( \
    service_id, agent_id, message_id, cmd_payload, expected_handler_status) \
    do { \
        mod_scmi_from_protocol_api_scmi_frame_validation_ExpectAnyArgsAndReturn( \
            SCMI_SUCCESS); \
        mod_scmi_from_protocol_api_get_agent_id_ExpectAnyArgsAndReturn( \
            FWK_SUCCESS); \
        mod_scmi_from_protocol_api_get_agent_id_ReturnThruPtr_agent_id( \
            &agent_id); \
        status = scmi_pd_message_handler( \
            test_protocol_id, \
            service_id, \
            (void *)&cmd_payload, \
            sizeof(cmd_payload), \
            message_id); \
        TEST_ASSERT_EQUAL(status, expected_handler_status); \
    } while (0)

static fwk_id_t test_protocol_id;
static uint32_t test_null_payload;

void setUp(void)
{
    memset(&scmi_pd_ctx, 0, sizeof(scmi_pd_ctx));
    scmi_pd_ctx.pd_api = &pd_api_ut;
    scmi_pd_ctx.scmi_api = &from_protocol_api;
#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    scmi_pd_ctx.domain_count = SCMI_PD_LOGICAL_DOMAIN_COUNT;
    scmi_pd_ctx.config = &agent_test_config;
#else
    scmi_pd_ctx.domain_count = 2;
#endif

    scmi_pd_ctx.ops = fwk_mm_calloc(
        scmi_pd_ctx.domain_count, sizeof(struct scmi_pd_operations));

    for (unsigned int i = 0; i < scmi_pd_ctx.domain_count; i++) {
        scmi_pd_ctx.ops[i].service_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_POWER_DOMAIN, i);
        scmi_pd_ctx.ops[i].agent_id = i;
    }
}

void tearDown(void)
{
}

void test_scmi_power_domain_pass(void)
{
    TEST_ASSERT_EQUAL(0, FWK_SUCCESS);
}

void test_process_request_event_successful_set_state(void)
{
    int status;
    struct fwk_event event;

    fwk_id_get_event_idx_ExpectAnyArgsAndReturn(SCMI_PD_EVENT_IDX_SET_STATE);
    set_state_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    status = process_request_event(&event);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_process_request_event_failed_set_state(void)
{
    int status;
    struct fwk_event event;
    struct scmi_pd_power_state_set_p2a retval_set = { .status =
                                                          SCMI_GENERIC_ERROR };

    fwk_id_get_event_idx_ExpectAnyArgsAndReturn(SCMI_PD_EVENT_IDX_SET_STATE);
    set_state_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    mod_scmi_from_protocol_api_respond_ExpectAndReturn(
        scmi_pd_ctx.ops[0].service_id,
        &retval_set,
        sizeof(retval_set.status),
        FWK_SUCCESS);

    status = process_request_event(&event);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_process_request_event_failed_set_state_failed_respond(void)
{
    int status;
    struct fwk_event event;
    struct scmi_pd_power_state_set_p2a retval_set = { .status =
                                                          SCMI_GENERIC_ERROR };

    fwk_id_get_event_idx_ExpectAnyArgsAndReturn(SCMI_PD_EVENT_IDX_SET_STATE);
    set_state_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    mod_scmi_from_protocol_api_respond_ExpectAndReturn(
        scmi_pd_ctx.ops[0].service_id,
        &retval_set,
        sizeof(retval_set.status),
        FWK_E_SUPPORT);

    status = process_request_event(&event);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void test_scmi_pd_get_domain_id_exceeding_domain_id(void)
{
    int status;
    fwk_id_t pd_id;

    status = scmi_pd_get_domain_id(
        SCMI_AGENT_0, SCMI_PD_LOGICAL_DOMAIN_COUNT, &pd_id);
    TEST_ASSERT_EQUAL(status, FWK_E_RANGE);
}

void test_scmi_pd_get_domain_id_invalid_power_domain_element_id(void)
{
    int status;
    fwk_id_t pd_id, expected_pd_id;
#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    expected_pd_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_POWER_DOMAIN,
        agent_logical_domain_table[SCMI_AGENT_1].domains[2]);
#else
    expected_pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 2);
#endif

    fwk_module_is_valid_element_id_ExpectAndReturn(expected_pd_id, false);
    status = scmi_pd_get_domain_id(SCMI_AGENT_1, 2, &pd_id);

    TEST_ASSERT_EQUAL(status, FWK_E_RANGE);
}

void test_scmi_pd_get_domain_id_success(void)
{
    int status;
    fwk_id_t pd_id, expected_pd_id;
#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    expected_pd_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_POWER_DOMAIN,
        agent_logical_domain_table[SCMI_AGENT_0].domains[1]);
#else
    expected_pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 1);
#endif

    fwk_module_is_valid_element_id_ExpectAndReturn(expected_pd_id, true);
    status = scmi_pd_get_domain_id(SCMI_AGENT_0, 1, &pd_id);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(
        pd_id.element.element_idx, expected_pd_id.element.element_idx);
}

#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
void test_agents_protocol_attributes(void)
{
    int status;
    fwk_id_t test_service_id;
    unsigned int agent_idx;
    struct scmi_pd_protocol_attributes_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
    };

    for (agent_idx = SCMI_AGENT_0; agent_idx < SCMI_AGENT_COUNT; ++agent_idx) {
        test_service_id = FWK_ID_ELEMENT(TEST_MODULE_IDX, agent_idx);
        return_values.attributes =
            agent_logical_domain_table[agent_idx].domain_count;
        EXPECT_SCMI_RESPONSE(
            test_service_id, return_values, sizeof(return_values));
        TEST_SCMI_COMMAND(
            test_service_id,
            agent_idx,
            MOD_SCMI_PROTOCOL_ATTRIBUTES,
            test_null_payload,
            FWK_SUCCESS);
    }
}
void test_agents_power_domain_attributes(void)
{
    int status;
    fwk_id_t test_service_id;
    unsigned int agent_idx, agent_type, pd_type, expected_pd_idx;
    fwk_id_t expected_pd_id;

    /* Test domain attributes for 2nd domain for each agent. */
    struct scmi_pd_power_domain_attributes_a2p cmd_payload = {
        .domain_id = 1,
    };
    struct scmi_pd_power_domain_attributes_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
    };

    for (agent_idx = SCMI_AGENT_0; agent_idx < SCMI_AGENT_COUNT; ++agent_idx) {
        agent_type = SCMI_AGENT_TYPE_OTHER;
        pd_type = MOD_PD_TYPE_DEVICE;
        expected_pd_idx = agent_logical_domain_table[agent_idx].domains[1];
        expected_pd_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, expected_pd_idx);
        mod_scmi_from_protocol_api_get_agent_type_ExpectAnyArgsAndReturn(
            FWK_SUCCESS);
        mod_scmi_from_protocol_api_get_agent_type_ReturnThruPtr_agent_type(
            &agent_type);
        get_domain_type_ExpectAnyArgsAndReturn(FWK_SUCCESS);
        get_domain_type_ReturnThruPtr_type(&pd_type);
        fwk_id_get_element_idx_ExpectAndReturn(expected_pd_id, expected_pd_idx);
        fwk_module_is_valid_element_id_ExpectAndReturn(expected_pd_id, true);
        fwk_module_get_element_name_ExpectAndReturn(
            expected_pd_id, test_domain_names[expected_pd_idx]);

        test_service_id = FWK_ID_ELEMENT(TEST_MODULE_IDX, agent_idx);
        return_values.attributes = TEST_DEVICE_PD_ATTRIBUTES;
        memcpy(return_values.name, test_domain_names[expected_pd_idx], 4);
        EXPECT_SCMI_RESPONSE(
            test_service_id, return_values, sizeof(return_values));
        TEST_SCMI_COMMAND(
            test_service_id,
            agent_idx,
            MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES,
            cmd_payload,
            FWK_SUCCESS);
    }
}

void test_agent_power_domain_attributes_exceeding_domain_range(void)
{
    int status;
    fwk_id_t test_service_id;
    unsigned int agent_id;

    /* Agent 0 only has 2 domains exposed */
    struct scmi_pd_power_domain_attributes_a2p cmd_payload = {
        .domain_id = agent_logical_domain_table[SCMI_AGENT_0].domain_count,
    };
    struct scmi_pd_power_domain_attributes_p2a return_values = {
        .status = (int32_t)SCMI_NOT_FOUND,
    };

    /* SCMI Agent 0 */
    agent_id = SCMI_AGENT_0;
    test_service_id = FWK_ID_ELEMENT(TEST_MODULE_IDX, agent_id);
    EXPECT_SCMI_RESPONSE(
        test_service_id, return_values, sizeof(return_values.status));
    TEST_SCMI_COMMAND(
        test_service_id,
        agent_id,
        MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES,
        cmd_payload,
        FWK_E_RANGE);
}
#endif

int scmi_power_domain_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_scmi_power_domain_pass);
    RUN_TEST(test_process_request_event_successful_set_state);
    RUN_TEST(test_process_request_event_failed_set_state);
    RUN_TEST(test_process_request_event_failed_set_state_failed_respond);
    RUN_TEST(test_scmi_pd_get_domain_id_exceeding_domain_id);
    RUN_TEST(test_scmi_pd_get_domain_id_invalid_power_domain_element_id);
    RUN_TEST(test_scmi_pd_get_domain_id_success);
#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    RUN_TEST(test_agents_protocol_attributes);
    RUN_TEST(test_agents_power_domain_attributes);
    RUN_TEST(test_agent_power_domain_attributes_exceeding_domain_range);
#endif
    return UNITY_END();
}

int main(void)
{
    return scmi_power_domain_test_main();
}
