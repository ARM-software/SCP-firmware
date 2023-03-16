/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_system_power_ut.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockmod_scmi_system_power_extra.h>

#include <internal/Mockfwk_core_internal.h>

#include <mod_scmi.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC

const struct mod_pd_restricted_api test_pd_api = {
    .get_domain_type = power_domain_get_domain_type,
    .get_domain_parent_id = power_domain_get_domain_parent_id,
    .set_state = power_domain_set_state,
    .get_state = power_domain_get_state,
    .reset = power_domain_reset,
    .system_suspend = power_domain_system_suspend,
    .system_shutdown = power_domain_system_shutdown,
};

const struct mod_scmi_from_protocol_api mock_scmi_api = {
    .get_agent_count = scmi_get_agent_count,
    .get_agent_id = scmi_get_agent_id,
    .get_agent_type = scmi_get_agent_type,
    .get_max_payload_size = scmi_get_max_payload_size,
    .notify = scmi_notify,
    .respond = scmi_respond,
    .write_payload = scmi_write_payload,
};

void setUp(void)
{
    memset(&scmi_sys_power_ctx, 0, sizeof(scmi_sys_power_ctx));

    scmi_sys_power_ctx.pd_api = &test_pd_api;
    scmi_sys_power_ctx.scmi_api = &mock_scmi_api;
    scmi_sys_power_ctx.config = &fake_config;

    fwk_id_t pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, 0);

    fake_config.wakeup_power_domain_id = pd_id;
    fake_config.wakeup_composite_state = 0;
    fake_config.system_suspend_state = MOD_PD_SYSTEM_SHUTDOWN;
}

void tearDown(void)
{
}

void test_state_set_reset_handler_success(void)
{
    int status;
    uint32_t mod_scmi_system_state = SCMI_SYSTEM_STATE_COLD_RESET;
    int32_t returned;

    power_domain_system_shutdown_ExpectAndReturn(
        MOD_PD_SYSTEM_COLD_RESET, FWK_SUCCESS);
    returned = (int32_t)scmi_sys_power_state_set_reset_handler(
        mod_scmi_system_state, &status);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(returned, SCMI_SUCCESS);
}

void test_state_set_reset_handler_pending(void)
{
    int status;
    uint32_t mod_scmi_system_state = SCMI_SYSTEM_STATE_COLD_RESET;
    int32_t returned;

    power_domain_system_shutdown_ExpectAndReturn(
        MOD_PD_SYSTEM_COLD_RESET, FWK_PENDING);
    returned = (int32_t)scmi_sys_power_state_set_reset_handler(
        mod_scmi_system_state, &status);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(returned, SCMI_SUCCESS);
}

void test_state_set_reset_handler_failed(void)
{
    int status;
    uint32_t mod_scmi_system_state = SCMI_SYSTEM_STATE_COLD_RESET;
    int32_t returned;

    power_domain_system_shutdown_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    returned = (int32_t)scmi_sys_power_state_set_reset_handler(
        mod_scmi_system_state, &status);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
    TEST_ASSERT_EQUAL(returned, SCMI_GENERIC_ERROR);
}

void test_state_set_suspend_handler_success(void)
{
    int status;
    int32_t returned;

    power_domain_system_suspend_ExpectAndReturn(
        MOD_PD_SYSTEM_SHUTDOWN, FWK_SUCCESS);

    returned = (int32_t)scmi_sys_power_state_set_suspend_handler(&status);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(returned, SCMI_SUCCESS);
}

void test_state_set_suspend_handler_denied_state(void)
{
    int status;
    int32_t returned;

    power_domain_system_suspend_ExpectAnyArgsAndReturn(FWK_E_STATE);
    returned = (int32_t)scmi_sys_power_state_set_suspend_handler(&status);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(returned, SCMI_DENIED);
}

void test_state_set_suspend_handler_failed_suspend(void)
{
    int status;
    int32_t returned;

    power_domain_system_suspend_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    returned = (int32_t)scmi_sys_power_state_set_suspend_handler(&status);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
    TEST_ASSERT_EQUAL(returned, SCMI_GENERIC_ERROR);
}

void test_state_set_power_up_handler_unsupported_agent(void)
{
    int status;
    int32_t returned;

    returned = (int32_t)scmi_sys_power_state_set_power_up_handler(
        &status, SCMI_AGENT_TYPE_PSCI);
    TEST_ASSERT_EQUAL(returned, SCMI_NOT_SUPPORTED);
}

void test_state_set_power_up_handler_failed_get_state(void)
{
    int status;
    int32_t returned;

    power_domain_get_state_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    returned = (int32_t)scmi_sys_power_state_set_power_up_handler(
        &status, SCMI_AGENT_TYPE_MANAGEMENT);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
    TEST_ASSERT_EQUAL(returned, SCMI_GENERIC_ERROR);
}

void test_state_set_power_up_handler_unsupported_state(void)
{
    int status;
    int32_t returned;

    uint32_t tmp_state = MOD_PD_STATE_ON;
    power_domain_get_state_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    power_domain_get_state_ReturnThruPtr_state(&tmp_state);
    returned = (int32_t)scmi_sys_power_state_set_power_up_handler(
        &status, SCMI_AGENT_TYPE_MANAGEMENT);
    TEST_ASSERT_EQUAL(returned, SCMI_DENIED);
}

void test_state_set_power_up_handler_success(void)
{
    int status;
    int32_t returned;

    uint32_t tmp_state = MOD_PD_STATE_OFF;
    power_domain_get_state_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    power_domain_get_state_ReturnThruPtr_state(&tmp_state);

    power_domain_set_state_ExpectAndReturn(
        fake_config.wakeup_power_domain_id, false, 0, FWK_SUCCESS);

    returned = (int32_t)scmi_sys_power_state_set_power_up_handler(
        &status, SCMI_AGENT_TYPE_MANAGEMENT);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(returned, SCMI_SUCCESS);
}

void test_state_set_power_up_handler_failed_set_state(void)
{
    int status;
    int32_t returned;

    power_domain_get_state_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    power_domain_set_state_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    returned = (int32_t)scmi_sys_power_state_set_power_up_handler(
        &status, SCMI_AGENT_TYPE_MANAGEMENT);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
    TEST_ASSERT_EQUAL(returned, SCMI_GENERIC_ERROR);
}

void test_state_set_sanity_checking_success(void)
{
    fwk_id_t service_id;
    int status;
    int32_t returned;
    enum scmi_agent_type agent_type;

    enum scmi_agent_type temp_agent_type = SCMI_AGENT_TYPE_MANAGEMENT;
    scmi_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ReturnThruPtr_agent_type(&temp_agent_type);
    returned = (int32_t)scmi_sys_power_state_set_sanity_checking(
        service_id, &agent_type, &status);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(returned, SCMI_SUCCESS);
}

void test_state_set_sanity_checking_unsuported_agent(void)
{
    fwk_id_t service_id;
    int status;
    int32_t returned;
    enum scmi_agent_type agent_type;

    enum scmi_agent_type temp_agent_type = SCMI_AGENT_TYPE_OSPM;
    scmi_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ReturnThruPtr_agent_type(&temp_agent_type);
    returned = (int32_t)scmi_sys_power_state_set_sanity_checking(
        service_id, &agent_type, &status);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(returned, SCMI_NOT_SUPPORTED);
}

void test_state_set_sanity_checking_incorrect_agent_type(void)
{
    fwk_id_t service_id;
    int status;
    int32_t returned;
    enum scmi_agent_type agent_type;

    scmi_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    returned = (int32_t)scmi_sys_power_state_set_sanity_checking(
        service_id, &agent_type, &status);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
    TEST_ASSERT_EQUAL(returned, SCMI_GENERIC_ERROR);
}

void test_state_set_sanity_checking_incorrect_agent_id(void)
{
    fwk_id_t service_id;
    int status;
    int32_t returned;
    enum scmi_agent_type agent_type;

    scmi_get_agent_id_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    returned = (int32_t)scmi_sys_power_state_set_sanity_checking(
        service_id, &agent_type, &status);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
    TEST_ASSERT_EQUAL(returned, SCMI_GENERIC_ERROR);
}

void test_state_set_handler_test_shutdown(void)
{
    fwk_id_t service_id;
    int32_t returned;
    enum scmi_agent_type temp_agent_type;
    struct scmi_sys_power_state_set_a2p payload;

    payload.flags = 0;
    payload.system_state = 0;
    temp_agent_type = SCMI_AGENT_TYPE_MANAGEMENT;
    scmi_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ReturnThruPtr_agent_type(&temp_agent_type);
    power_domain_system_shutdown_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_respond_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    returned = (int32_t)scmi_sys_power_state_set_handler(
        service_id, (uint32_t *)&payload);
    TEST_ASSERT_EQUAL(returned, SCMI_SUCCESS);
}

void test_state_set_handler_test_suspend(void)
{
    fwk_id_t service_id;
    int32_t returned;
    enum scmi_agent_type temp_agent_type;
    struct scmi_sys_power_state_set_a2p payload;

    payload.flags = 0;
    payload.system_state = 4;
    temp_agent_type = SCMI_AGENT_TYPE_MANAGEMENT;
    scmi_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ReturnThruPtr_agent_type(&temp_agent_type);
    power_domain_system_suspend_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_respond_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    returned = (int32_t)scmi_sys_power_state_set_handler(
        service_id, (uint32_t *)&payload);
    TEST_ASSERT_EQUAL(returned, SCMI_SUCCESS);
}

void test_state_set_handler_test_power_up(void)
{
    fwk_id_t service_id;
    int32_t returned;
    enum scmi_agent_type temp_agent_type;
    struct scmi_sys_power_state_set_a2p payload;

    payload.flags = 0;
    payload.system_state = 3;
    temp_agent_type = SCMI_AGENT_TYPE_MANAGEMENT;
    scmi_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_get_agent_type_ReturnThruPtr_agent_type(&temp_agent_type);
    power_domain_get_state_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    power_domain_set_state_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    scmi_respond_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    returned = (int32_t)scmi_sys_power_state_set_handler(
        service_id, (uint32_t *)&payload);
    TEST_ASSERT_EQUAL(returned, SCMI_SUCCESS);
}

void test_system_state_get_power_up(void)
{
    int32_t returned;
    enum scmi_system_state system_state;

    uint32_t tmp_state = MOD_PD_STATE_ON;
    power_domain_get_state_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    power_domain_get_state_ReturnThruPtr_state(&tmp_state);

    returned = (int32_t)system_state_get(&system_state);
    TEST_ASSERT_EQUAL(returned, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(system_state, SCMI_SYSTEM_STATE_POWER_UP);
}

void test_system_state_get_shutdown(void)
{
    int32_t returned;
    enum scmi_system_state system_state;

    uint32_t tmp_state = MOD_PD_STATE_OFF;
    power_domain_get_state_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    power_domain_get_state_ReturnThruPtr_state(&tmp_state);

    returned = (int32_t)system_state_get(&system_state);
    TEST_ASSERT_EQUAL(returned, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(system_state, SCMI_SYSTEM_STATE_SHUTDOWN);
}

void test_system_state_get_suspend(void)
{
    int32_t returned;
    enum scmi_system_state system_state;

    uint32_t tmp_state = MOD_PD_STATE_SLEEP;
    power_domain_get_state_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    power_domain_get_state_ReturnThruPtr_state(&tmp_state);

    returned = (int32_t)system_state_get(&system_state);
    TEST_ASSERT_EQUAL(returned, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(system_state, SCMI_SYSTEM_STATE_SUSPEND);
}

int scmi_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_state_set_reset_handler_success);
    RUN_TEST(test_state_set_reset_handler_pending);
    RUN_TEST(test_state_set_reset_handler_failed);

    RUN_TEST(test_state_set_suspend_handler_success);
    RUN_TEST(test_state_set_suspend_handler_denied_state);
    RUN_TEST(test_state_set_suspend_handler_failed_suspend);

    RUN_TEST(test_state_set_power_up_handler_unsupported_agent);
    RUN_TEST(test_state_set_power_up_handler_failed_get_state);
    RUN_TEST(test_state_set_power_up_handler_unsupported_state);
    RUN_TEST(test_state_set_power_up_handler_success);
    RUN_TEST(test_state_set_power_up_handler_failed_set_state);

    RUN_TEST(test_state_set_sanity_checking_success);
    RUN_TEST(test_state_set_sanity_checking_unsuported_agent);
    RUN_TEST(test_state_set_sanity_checking_incorrect_agent_type);
    RUN_TEST(test_state_set_sanity_checking_incorrect_agent_id);

    RUN_TEST(test_state_set_handler_test_shutdown);
    RUN_TEST(test_state_set_handler_test_suspend);
    RUN_TEST(test_state_set_handler_test_power_up);

    RUN_TEST(test_system_state_get_power_up);
    RUN_TEST(test_system_state_get_shutdown);
    RUN_TEST(test_system_state_get_suspend);

    return UNITY_END();
}

int main(void)
{
    return scmi_test_main();
}
