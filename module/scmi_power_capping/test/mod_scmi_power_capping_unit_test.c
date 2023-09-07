/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockmod_power_allocator_extra.h>
#include <Mockscmi_power_capping_protocol.h>

#include <stdarg.h>

#include UNIT_TEST_SRC

#include "mod_scmi_power_capping_unit_test.h"

static int status;

static const struct mod_scmi_power_capping_domain_config
    scmi_power_capping_default_config = {
        .parent_idx = __LINE__,
    };

/* Test functions */
/* Initialize the tests */

void setUp(void)
{
}

void tearDown(void)
{
}

void utest_scmi_power_capping_init_invalid_element_count(void)
{
    status = scmi_power_capping_init(fwk_module_id_scmi_power_capping, 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_scmi_power_capping_init_success(void)
{
    uint32_t domain_count = __LINE__;
    struct mod_scmi_power_capping_domain_context *domain_table =
        (struct mod_scmi_power_capping_domain_context *)__LINE__;

    struct mod_scmi_power_capping_context expected_ctx = {
        .power_capping_domain_ctx_table = domain_table,
        .domain_count = domain_count,
    };

    fwk_mm_calloc_ExpectAndReturn(
        domain_count,
        sizeof(struct mod_scmi_power_capping_domain_context),
        domain_table);

    pcapping_protocol_init_ExpectWithArray(&expected_ctx, 1);

    status = scmi_power_capping_init(
        fwk_module_id_scmi_power_capping, domain_count, NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_power_capping_element_init_success(void)
{
    uint32_t domain_idx = __LINE__;
    uint32_t domain_count = __LINE__;
    fwk_id_t domain_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING, domain_idx);

    fwk_id_get_element_idx_ExpectAndReturn(domain_id, domain_idx);

    pcapping_protocol_domain_init_ExpectAndReturn(
        domain_idx, &scmi_power_capping_default_config, FWK_SUCCESS);

    status = scmi_power_capping_element_init(
        domain_id, domain_count, &scmi_power_capping_default_config);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_power_capping_element_init_null_config(void)
{
    fwk_id_t domain_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING, __LINE__);
    uint32_t domain_count = __LINE__;
    status = scmi_power_capping_element_init(domain_id, domain_count, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_scmi_power_capping_element_init_failure(void)
{
    uint32_t domain_count = __LINE__;
    fwk_id_t domain_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING, domain_count);

    fwk_id_get_element_idx_ExpectAndReturn(domain_id, domain_count);

    pcapping_protocol_domain_init_ExpectAndReturn(
        domain_count, &scmi_power_capping_default_config, FWK_E_PARAM);

    status = scmi_power_capping_element_init(
        domain_id, domain_count, &scmi_power_capping_default_config);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_scmi_power_capping_bind_success(void)
{
    fwk_id_t bind_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING);
    fwk_id_is_type_ExpectAndReturn(bind_id, FWK_ID_TYPE_ELEMENT, false);
    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_ALLOCATOR),
        FWK_ID_API(
            FWK_MODULE_IDX_POWER_ALLOCATOR, MOD_POWER_ALLOCATOR_API_IDX_CAP),
        &(power_management_apis.power_allocator_api),
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_COORDINATOR),
        FWK_ID_API(
            FWK_MODULE_IDX_POWER_COORDINATOR,
            MOD_POWER_COORDINATOR_API_IDX_PERIOD),
        &(power_management_apis.power_coordinator_api),
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_METER),
        FWK_ID_API(
            FWK_MODULE_IDX_POWER_METER, MOD_POWER_METER_API_IDX_MEASUREMENT),
        &(power_management_apis.power_meter_api),
        FWK_SUCCESS);

    pcapping_protocol_bind_ExpectAndReturn(FWK_SUCCESS);
    pcapping_protocol_set_power_apis_Expect(&power_management_apis);
    status = scmi_power_capping_bind(bind_id, 0);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_power_capping_bind_success_round_1(void)
{
    fwk_id_t bind_id;
    status = scmi_power_capping_bind(bind_id, 1);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_power_capping_bind_failure(void)
{
    fwk_id_t bind_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING);
    fwk_id_is_type_ExpectAndReturn(bind_id, FWK_ID_TYPE_ELEMENT, false);
    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_ALLOCATOR),
        FWK_ID_API(
            FWK_MODULE_IDX_POWER_ALLOCATOR, MOD_POWER_ALLOCATOR_API_IDX_CAP),
        &(power_management_apis.power_allocator_api),
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_COORDINATOR),
        FWK_ID_API(
            FWK_MODULE_IDX_POWER_COORDINATOR,
            MOD_POWER_COORDINATOR_API_IDX_PERIOD),
        &(power_management_apis.power_coordinator_api),
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_METER),
        FWK_ID_API(
            FWK_MODULE_IDX_POWER_METER, MOD_POWER_METER_API_IDX_MEASUREMENT),
        &(power_management_apis.power_meter_api),
        FWK_SUCCESS);

    pcapping_protocol_bind_ExpectAndReturn(FWK_E_INIT);
    status = scmi_power_capping_bind(bind_id, 0);
    TEST_ASSERT_EQUAL(status, FWK_E_INIT);
}

void utest_scmi_power_capping_start(void)
{
    fwk_id_t id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_CAPPING, 10u);
    int status = FWK_SUCCESS;

    pcapping_protocol_start_ExpectAndReturn(id, FWK_SUCCESS);

    status = scmi_power_capping_start(id);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_power_capping_process_notification(void)
{
    int status;

    struct fwk_event notification_event;

    pcapping_protocol_process_notification_ExpectAndReturn(
        &notification_event, FWK_SUCCESS);
    status = scmi_power_capping_process_notification(&notification_event, NULL);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_power_capping_process_bind_request(void)
{
    fwk_id_t source_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI);
    fwk_id_t target_id;
    const void *api;
    fwk_id_t api_id = FWK_ID_API(
        FWK_MODULE_IDX_SCMI_POWER_CAPPING,
        MOD_SCMI_POWER_CAPPING_API_IDX_REQUEST);

    fwk_id_is_equal_ExpectAndReturn(source_id, source_id, true);
    pcapping_protocol_process_bind_request_ExpectAndReturn(
        api_id, &api, FWK_SUCCESS);

    status = scmi_power_capping_process_bind_request(
        source_id, target_id, api_id, &api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_scmi_power_capping_process_bind_request_failure(void)
{
    fwk_id_t source_id;
    fwk_id_t target_id;
    const void *api;
    fwk_id_t api_id = FWK_ID_API(
        FWK_MODULE_IDX_SCMI_POWER_CAPPING,
        MOD_SCMI_POWER_CAPPING_API_IDX_REQUEST);

    fwk_id_is_equal_ExpectAndReturn(
        source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI), false);

    status = scmi_power_capping_process_bind_request(
        source_id, target_id, api_id, &api);
    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);
}

int scmi_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(utest_scmi_power_capping_init_invalid_element_count);
    RUN_TEST(utest_scmi_power_capping_init_success);
    RUN_TEST(utest_scmi_power_capping_element_init_success);
    RUN_TEST(utest_scmi_power_capping_element_init_null_config);
    RUN_TEST(utest_scmi_power_capping_element_init_failure);
    RUN_TEST(utest_scmi_power_capping_bind_success);
    RUN_TEST(utest_scmi_power_capping_bind_success_round_1);
    RUN_TEST(utest_scmi_power_capping_bind_failure);
    RUN_TEST(utest_scmi_power_capping_start);
    RUN_TEST(utest_scmi_power_capping_process_notification);
    RUN_TEST(utest_scmi_power_capping_process_bind_request);
    RUN_TEST(utest_scmi_power_capping_process_bind_request_failure);
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_test_main();
}
#endif
