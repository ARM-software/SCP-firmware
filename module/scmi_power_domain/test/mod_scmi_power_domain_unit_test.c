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

void setUp(void)
{
    memset(&scmi_pd_ctx, 0, sizeof(scmi_pd_ctx));
    scmi_pd_ctx.pd_api = &pd_api_ut;
    scmi_pd_ctx.scmi_api = &from_protocol_api;
    scmi_pd_ctx.domain_count = 2;

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

int scmi_power_domain_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_scmi_power_domain_pass);
    RUN_TEST(test_process_request_event_successful_set_state);
    RUN_TEST(test_process_request_event_failed_set_state);
    RUN_TEST(test_process_request_event_failed_set_state_failed_respond);
    return UNITY_END();
}

int main(void)
{
    return scmi_power_domain_test_main();
}
