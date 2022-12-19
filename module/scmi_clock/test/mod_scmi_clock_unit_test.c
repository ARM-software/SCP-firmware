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
#    include <Mockfwk_mm.h>
#    include <Mockfwk_module.h>
#    include <internal/Mockfwk_core_internal.h>
#endif
#include <Mockmod_scmi_clock_extra.h>
#include <mod_clock.h>

#include <mod_scmi.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#if defined(BUILD_HAS_MOD_RESOURCE_PERMS)
    #include <mod_resource_perms.h>
#endif

#include UNIT_TEST_SRC
#include <config_scmi_clock.h>

static struct mod_scmi_clock_ctx scmi_clock_ctx;

struct mod_scmi_from_protocol_api from_protocol_api = {
    .get_agent_count = mod_scmi_from_protocol_api_get_agent_count,
    .get_agent_id = mod_scmi_from_protocol_api_get_agent_id,
    .get_agent_type = mod_scmi_from_protocol_api_get_agent_type,
    .get_max_payload_size = mod_scmi_from_protocol_api_get_max_payload_size,
    .write_payload = mod_scmi_from_protocol_api_write_payload,
    .respond = mod_scmi_from_protocol_api_respond,
    .notify = mod_scmi_from_protocol_api_notify,
};

#if defined(BUILD_HAS_MOD_RESOURCE_PERMS)
struct mod_res_permissions_api perm_api = {
    .agent_has_protocol_permission = mod_res_permissions_api_agent_has_protocol_permission,
    .agent_has_message_permission = mod_res_permissions_api_agent_has_message_permission,
    .agent_has_resource_permission = mod_res_permissions_api_agent_has_resource_permission,
    .agent_set_device_permission = mod_res_permissions_api_agent_set_device_permission,
    .agent_set_device_protocol_permission = mod_res_permissions_api_agent_set_device_protocol_permission,
    .agent_reset_config = mod_res_permissions_api_agent_reset_config,
};
#endif

void assert_clock_state_and_ref_count_meets_expectations(void)
{
    TEST_ASSERT_EQUAL_INT8_ARRAY(
        agent_clock_state_table_expected,
        agent_clock_state_table,
        FAKE_SCMI_AGENT_IDX_COUNT * CLOCK_DEV_IDX_COUNT);
    TEST_ASSERT_EQUAL_INT8_ARRAY(
        dev_clock_ref_count_table_expected,
        dev_clock_ref_count_table,
        CLOCK_DEV_IDX_COUNT);
}

void setup_agent_state_table(
    unsigned int agent_id,
    unsigned int scmi_clock_idx,
    enum mod_clock_state state)
{
    agent_clock_state_table
        [agent_id * scmi_clock_ctx.clock_devices + scmi_clock_idx] =
        (unsigned int)state;
}

void setup_ref_count_table(
    unsigned int clock_idx,
    unsigned int ref_count)
{
    dev_clock_ref_count_table[clock_idx] = ref_count;
}

void setup_expected_agent_state_table(
    unsigned int agent_id,
    unsigned int scmi_clock_idx,
    enum mod_clock_state state)
{
    agent_clock_state_table_expected
        [agent_id * scmi_clock_ctx.clock_devices + scmi_clock_idx] =
        (unsigned int)state;
}

void setup_expected_ref_count_table(
    unsigned int clock_idx,
    unsigned int ref_count)
{
    dev_clock_ref_count_table_expected[clock_idx] = ref_count;
}


void setUp(void)
{
    scmi_clock_ctx.config = config_scmi_clock.data;
    scmi_clock_ctx.max_pending_transactions = scmi_clock_ctx.config->max_pending_transactions;
    scmi_clock_ctx.agent_table = scmi_clock_ctx.config->agent_table;

    scmi_clock_ctx.clock_devices = CLOCK_DEV_IDX_COUNT;

    /* Allocate a table of clock operations */
    scmi_clock_ctx.clock_ops = clock_ops_table;
    memset(clock_ops_table, 0,
           CLOCK_DEV_IDX_COUNT * sizeof(struct clock_operations));

    /* Initialize table */
    for (unsigned int i = 0; i < (unsigned int)scmi_clock_ctx.clock_devices; i++) {
        scmi_clock_ctx.clock_ops[i].service_id = FWK_ID_NONE;
    }

    scmi_clock_ctx.scmi_api = &from_protocol_api;
    #if defined(BUILD_HAS_MOD_RESOURCE_PERMS)
        scmi_clock_ctx.res_perms_api = &perm_api;
    #endif

    scmi_clock_ctx.dev_clock_ref_count_table = dev_clock_ref_count_table;
    scmi_clock_ctx.agent_clock_state_table = agent_clock_state_table;
    memcpy(
        dev_clock_ref_count_table,
        dev_clock_ref_count_table_default,
        FWK_ARRAY_SIZE(dev_clock_ref_count_table));
    memcpy(
        agent_clock_state_table,
        agent_clock_state_table_default,
        FWK_ARRAY_SIZE(agent_clock_state_table));
    memcpy(
        dev_clock_ref_count_table_expected,
        dev_clock_ref_count_table_default,
        FWK_ARRAY_SIZE(dev_clock_ref_count_table));
    memcpy(
        agent_clock_state_table_expected,
        agent_clock_state_table_default,
        FWK_ARRAY_SIZE(agent_clock_state_table));

}

void tearDown(void)
{
}

int fwk_put_event_callback(struct fwk_event *event, int numCalls)
{
    struct scmi_clock_event_request_params *params;
    params = (struct scmi_clock_event_request_params *)event->params;

    TEST_ASSERT_EQUAL(0x00000001, params->request_data.set_rate_data.rate[0]);
    TEST_ASSERT_EQUAL(0x00000001, params->request_data.set_rate_data.rate[1]);
    TEST_ASSERT_EQUAL(MOD_CLOCK_ROUND_MODE_NEAREST,
                      params->request_data.set_rate_data.round_mode);

    return FWK_SUCCESS;
}

void test_function_set_rate(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(FAKE_MODULE_IDX, FAKE_SCMI_AGENT_IDX_OSPM0);

    unsigned int agent_id = FAKE_SCMI_AGENT_IDX_OSPM0;

    struct scmi_clock_rate_set_a2p payload = {
        .flags = SCMI_CLOCK_RATE_SET_ROUND_AUTO_MASK,
        .clock_id = CLOCK_DEV_IDX_FAKE0,
        .rate[0] = 0x00000001,
        .rate[1] = 0x00000001,
    };

    mod_scmi_from_protocol_api_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mod_scmi_from_protocol_api_get_agent_id_ReturnThruPtr_agent_id(&agent_id);

    #if defined(BUILD_HAS_MOD_RESOURCE_PERMS)
        mod_scmi_from_protocol_api_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
        mod_res_permissions_api_agent_has_resource_permission_ExpectAnyArgsAndReturn(MOD_RES_PERMS_ACCESS_ALLOWED);
    #endif

    fwk_module_is_valid_element_id_ExpectAnyArgsAndReturn(true);

    mod_scmi_from_protocol_api_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mod_scmi_from_protocol_api_get_agent_id_ReturnThruPtr_agent_id(&agent_id);

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(CLOCK_DEV_IDX_FAKE0);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);

    __fwk_put_event_Stub(fwk_put_event_callback);

    status = scmi_clock_message_handler(
                                    (fwk_id_t)MOD_SCMI_PROTOCOL_ID_CLOCK,
                                    service_id,
                                    (const uint32_t *)&payload,
                                    payload_size_table[MOD_SCMI_CLOCK_RATE_SET],
                                    MOD_SCMI_CLOCK_RATE_SET);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void test_clock_ref_count_allocate(void)
{
    uint8_t *agent_clock_state_table_return = (uint8_t *)0xAAAAAAAA;
    uint8_t *dev_clock_ref_count_table_return = (uint8_t *)0xAAAAAAAA;

    scmi_clock_ctx.dev_clock_ref_count_table = NULL;
    scmi_clock_ctx.agent_clock_state_table = NULL;

    fwk_mm_calloc_ExpectAndReturn(
            (unsigned int)scmi_clock_ctx.config->agent_count *
                (unsigned int)scmi_clock_ctx.clock_devices,
            sizeof(*scmi_clock_ctx.agent_clock_state_table),
            agent_clock_state_table_return);
    fwk_mm_calloc_ExpectAndReturn(
            (unsigned int)scmi_clock_ctx.clock_devices,
            sizeof(*scmi_clock_ctx.dev_clock_ref_count_table),
            dev_clock_ref_count_table_return);

    clock_ref_count_allocate();
    TEST_ASSERT_EQUAL_PTR(
        scmi_clock_ctx.agent_clock_state_table,
        agent_clock_state_table_return);
    TEST_ASSERT_EQUAL_PTR(
        scmi_clock_ctx.dev_clock_ref_count_table,
        dev_clock_ref_count_table_return);
}

void test_clock_ref_count_init(void)
{
    /* Make sure that the tables are cleared before running tests. */
    memset(agent_clock_state_table, 0,
        FAKE_SCMI_AGENT_IDX_COUNT * CLOCK_DEV_IDX_COUNT);
    memset(dev_clock_ref_count_table, 0, CLOCK_DEV_IDX_COUNT);

    /* OSPM0 */
    for (unsigned int i = 0; i < CLOCK_DEV_IDX_COUNT; i++) {
        fwk_id_get_element_idx_ExpectAndReturn(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, i),
            i);
    }
    /* OSPM1 */
    fwk_id_get_element_idx_ExpectAndReturn(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_CLOCK, CLOCK_DEV_IDX_FAKE3),
            CLOCK_DEV_IDX_FAKE3);

    clock_ref_count_init();
    TEST_ASSERT_EQUAL_INT8_ARRAY(
        agent_clock_state_table_default,
        agent_clock_state_table,
        FAKE_SCMI_AGENT_IDX_COUNT * CLOCK_DEV_IDX_COUNT);
    TEST_ASSERT_EQUAL_INT8_ARRAY(
        dev_clock_ref_count_table_default,
        dev_clock_ref_count_table,
        CLOCK_DEV_IDX_COUNT);
}

void test_mod_scmi_clock_request_state_check_no_change_running(void)
{
    enum mod_scmi_clock_policy_status status;

    /* Set `SCMI_CLOCK_OSPM0_IDX0` as RUNNING and ref_count == 1 */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE0, 1);

    status = mod_scmi_clock_request_state_check(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        CLOCK_DEV_IDX_FAKE0,
        MOD_CLOCK_STATE_RUNNING);
    TEST_ASSERT_EQUAL(MOD_SCMI_CLOCK_SKIP_MESSAGE_HANDLER, status);
}

void test_mod_scmi_clock_request_state_check_no_change_stopped(void)
{
    enum mod_scmi_clock_policy_status status;

    /* Set `SCMI_CLOCK_OSPM0_IDX0` as STOPPED and ref_count == 0  */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_STOPPED);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE0, 0);

    status = mod_scmi_clock_request_state_check(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        CLOCK_DEV_IDX_FAKE0,
        MOD_CLOCK_STATE_STOPPED);
    TEST_ASSERT_EQUAL(MOD_SCMI_CLOCK_SKIP_MESSAGE_HANDLER, status);
}

void test_mod_scmi_clock_request_state_check_ref_count_0_running(void)
{
    enum mod_scmi_clock_policy_status status;

    /* Set `SCMI_CLOCK_OSPM0_IDX0` as STOPPED and ref_count == 0 */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_STOPPED);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE0, 0);

    status = mod_scmi_clock_request_state_check(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        CLOCK_DEV_IDX_FAKE0,
        MOD_CLOCK_STATE_RUNNING);
    TEST_ASSERT_EQUAL(MOD_SCMI_CLOCK_EXECUTE_MESSAGE_HANDLER, status);
}

void test_mod_scmi_clock_request_state_check_ref_count_1_stopped(void)
{

    enum mod_scmi_clock_policy_status status;

    /* Set `SCMI_CLOCK_OSPM0_IDX0` as RUNNING and ref_count == 1 */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE0, 1);

    status = mod_scmi_clock_request_state_check(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        CLOCK_DEV_IDX_FAKE0,
        MOD_CLOCK_STATE_STOPPED);
    TEST_ASSERT_EQUAL(MOD_SCMI_CLOCK_EXECUTE_MESSAGE_HANDLER, status);
}

void test_mod_scmi_clock_request_state_check_ref_count_0_stopped(void)
{
    enum mod_scmi_clock_policy_status status;

    /*
     * Create an invalid situation where it is set `SCMI_CLOCK_OSPM0_IDX0`
     * as RUNNING and ref_count == 0
     */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE0, 0);

    status = mod_scmi_clock_request_state_check(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        CLOCK_DEV_IDX_FAKE0,
        MOD_CLOCK_STATE_STOPPED);
    TEST_ASSERT_EQUAL(MOD_SCMI_CLOCK_SKIP_MESSAGE_HANDLER, status);
}

void test_mod_scmi_clock_request_state_check_ref_count_1_running(void)
{
    enum mod_scmi_clock_policy_status status;

    /*
     * Set `SCMI_CLOCK_OSPM0_IDX3` as RUNNING,
     * set `SCMI_CLOCK_OSPM1_IDX0` as STOPPED and
     * ref_count == 1
     */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX3,
        MOD_CLOCK_STATE_RUNNING);
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        MOD_CLOCK_STATE_STOPPED);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE3, 1);

    status = mod_scmi_clock_request_state_check(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        CLOCK_DEV_IDX_FAKE3,
        MOD_CLOCK_STATE_RUNNING);
    TEST_ASSERT_EQUAL(MOD_SCMI_CLOCK_SKIP_MESSAGE_HANDLER, status);
}

void test_mod_scmi_clock_request_state_check_ref_count_2_stopped(void)
{
    enum mod_scmi_clock_policy_status status;

    /*
     * Set `SCMI_CLOCK_OSPM0_IDX3` as RUNNING,
     * set `SCMI_CLOCK_OSPM1_IDX0` as RUNNING and
     * ref_count == 2
     */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX3,
        MOD_CLOCK_STATE_RUNNING);
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE3, 2);

    status = mod_scmi_clock_request_state_check(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        CLOCK_DEV_IDX_FAKE3,
        MOD_CLOCK_STATE_STOPPED);
    TEST_ASSERT_EQUAL(MOD_SCMI_CLOCK_SKIP_MESSAGE_HANDLER, status);
}

void test_mod_scmi_clock_state_update_no_change_running(void)
{
    int status;

    /*
     * Set `SCMI_CLOCK_OSPM0_IDX0` as RUNNING,
     * set expected `SCMI_CLOCK_OSPM0_IDX0` as RUNNING and
     * ref_count == 1 and expected ref_count == 1
     */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE0, 1);

    setup_expected_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_expected_ref_count_table(CLOCK_DEV_IDX_FAKE0, 1);

    status = mod_scmi_clock_state_update(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        CLOCK_DEV_IDX_FAKE0,
        MOD_CLOCK_STATE_RUNNING);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    assert_clock_state_and_ref_count_meets_expectations();
}

void test_mod_scmi_clock_state_update_no_change_stopped(void)
{
    int status;

    /*
     * Set `SCMI_CLOCK_OSPM0_IDX0` as STOPPED,
     * set expected `SCMI_CLOCK_OSPM0_IDX0` as STOPPED,
     * ref_count == 0 and expected ref_count == 0
     */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_STOPPED);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE0, 0);

    setup_expected_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_STOPPED);
    setup_expected_ref_count_table(CLOCK_DEV_IDX_FAKE0, 0);

    status = mod_scmi_clock_state_update(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        CLOCK_DEV_IDX_FAKE0,
        MOD_CLOCK_STATE_STOPPED);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    assert_clock_state_and_ref_count_meets_expectations();
}

void test_mod_scmi_clock_state_update_ref_count_0_running(void)
{
    int status;

    /*
     * Set SCMI clock 0 from `FAKE_SCMI_AGENT_IDX_OSPM0` as STOPPED,
     * set expected SCMI clock 0 from `FAKE_SCMI_AGENT_IDX_OSPM0` as RUNNING,
     * ref_count == 0 and expected ref_count == 1
     */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_STOPPED);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE0, 0);

    setup_expected_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_expected_ref_count_table(CLOCK_DEV_IDX_FAKE0, 1);

    status = mod_scmi_clock_state_update(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        CLOCK_DEV_IDX_FAKE0,
        MOD_CLOCK_STATE_RUNNING);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    assert_clock_state_and_ref_count_meets_expectations();
}

void test_mod_scmi_clock_state_update_ref_count_1_running(void)
{
    int status;

    /*
     * Set `SCMI_CLOCK_OSPM0_IDX3` as RUNNING,
     * set `SCMI_CLOCK_OSPM1_IDX0` as STOPPED,
     * set expected `SCMI_CLOCK_OSPM0_IDX3` as RUNNING,
     * set expected `SCMI_CLOCK_OSPM1_IDX0` as RUNNING,
     * ref_count == 2 and expected ref_count == 1
     */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX3,
        MOD_CLOCK_STATE_RUNNING);
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        MOD_CLOCK_STATE_STOPPED);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE3, 1);

    setup_expected_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX3,
        MOD_CLOCK_STATE_RUNNING);
    setup_expected_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_expected_ref_count_table(CLOCK_DEV_IDX_FAKE3, 2);

    status = mod_scmi_clock_state_update(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        CLOCK_DEV_IDX_FAKE3,
        MOD_CLOCK_STATE_RUNNING);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    assert_clock_state_and_ref_count_meets_expectations();
}

void test_mod_scmi_clock_state_update_ref_count_2_stopped(void)
{
    int status;

    /*
     * Set `SCMI_CLOCK_OSPM0_IDX3` as RUNNING,
     * set `SCMI_CLOCK_OSPM1_IDX0` as RUNNING,
     * set expected `SCMI_CLOCK_OSPM0_IDX3` as RUNNING,
     * set expected `SCMI_CLOCK_OSPM1_IDX0` as STOPPED,
     * ref_count == 2 and expected ref_count == 1
     */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX3,
        MOD_CLOCK_STATE_RUNNING);
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE3, 2);

    setup_expected_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX3,
        MOD_CLOCK_STATE_RUNNING);
    setup_expected_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        MOD_CLOCK_STATE_STOPPED);
    setup_expected_ref_count_table(CLOCK_DEV_IDX_FAKE3, 1);

    status = mod_scmi_clock_state_update(
        FAKE_SCMI_AGENT_IDX_OSPM1,
        SCMI_CLOCK_OSPM1_IDX0,
        CLOCK_DEV_IDX_FAKE3,
        MOD_CLOCK_STATE_STOPPED);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    assert_clock_state_and_ref_count_meets_expectations();
}

void test_mod_scmi_clock_state_update_ref_count_1_stopped(void)
{
    int status;

    /*
     * Set SCMI clock 0 from `FAKE_SCMI_AGENT_IDX_OSPM0` as RUNNING,
     * set expected SCMI clock 0 from `FAKE_SCMI_AGENT_IDX_OSPM0` as STOPPED,
     * ref_count == 1 and expected ref_count == 0
     */
    setup_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_RUNNING);
    setup_expected_ref_count_table(CLOCK_DEV_IDX_FAKE0, 0);

    setup_expected_agent_state_table(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        MOD_CLOCK_STATE_STOPPED);
    setup_ref_count_table(CLOCK_DEV_IDX_FAKE0, 1);

    status = mod_scmi_clock_state_update(
        FAKE_SCMI_AGENT_IDX_OSPM0,
        SCMI_CLOCK_OSPM0_IDX0,
        CLOCK_DEV_IDX_FAKE0,
        MOD_CLOCK_STATE_STOPPED);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    assert_clock_state_and_ref_count_meets_expectations();
}

int scmi_test_main(void)
{
    UNITY_BEGIN();
    #if defined(BUILD_HAS_MOD_RESOURCE_PERMS)
        RUN_TEST(test_function_set_rate);
    #else
        RUN_TEST(test_function_set_rate);
        RUN_TEST(test_clock_ref_count_allocate);
        RUN_TEST(test_clock_ref_count_init);
        RUN_TEST(test_mod_scmi_clock_request_state_check_no_change_running);
        RUN_TEST(test_mod_scmi_clock_request_state_check_no_change_stopped);
        RUN_TEST(test_mod_scmi_clock_request_state_check_ref_count_0_running);
        RUN_TEST(test_mod_scmi_clock_request_state_check_ref_count_1_stopped);
        RUN_TEST(test_mod_scmi_clock_request_state_check_ref_count_0_stopped);
        RUN_TEST(test_mod_scmi_clock_request_state_check_ref_count_1_running);
        RUN_TEST(test_mod_scmi_clock_request_state_check_ref_count_2_stopped);
        RUN_TEST(test_mod_scmi_clock_state_update_no_change_running);
        RUN_TEST(test_mod_scmi_clock_state_update_no_change_stopped);
        RUN_TEST(test_mod_scmi_clock_state_update_ref_count_0_running);
        RUN_TEST(test_mod_scmi_clock_state_update_ref_count_1_running);
        RUN_TEST(test_mod_scmi_clock_state_update_ref_count_2_stopped);
        RUN_TEST(test_mod_scmi_clock_state_update_ref_count_1_stopped);

    #endif
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_test_main();
}
#endif
