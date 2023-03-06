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
#    include <internal/Mockfwk_core_internal.h>
#endif
#include <Mockmod_scmi_clock_extra.h>
#include <mod_clock.h>

#include <mod_scmi.h>
#include <config_scmi_clock.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#if defined(BUILD_HAS_MOD_RESOURCE_PERMS)
    #include <mod_resource_perms.h>
#endif

#include UNIT_TEST_SRC

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

void setUp(void)
{
    scmi_clock_ctx.config = config_scmi_clock.data;
    scmi_clock_ctx.max_pending_transactions = scmi_clock_ctx.config->max_pending_transactions;
    scmi_clock_ctx.agent_table = scmi_clock_ctx.config->agent_table;

    scmi_clock_ctx.clock_devices = FWK_ARRAY_SIZE(agent_device_table_ospm);

    /* Allocate a table of clock operations */
    scmi_clock_ctx.clock_ops =
        fwk_mm_calloc((unsigned int)scmi_clock_ctx.clock_devices,
        sizeof(struct clock_operations));

    /* Initialize table */
    for (unsigned int i = 0; i < (unsigned int)scmi_clock_ctx.clock_devices; i++) {
        scmi_clock_ctx.clock_ops[i].service_id = FWK_ID_NONE;
    }

    scmi_clock_ctx.scmi_api = &from_protocol_api;
    #if defined(BUILD_HAS_MOD_RESOURCE_PERMS)
        scmi_clock_ctx.res_perms_api = &perm_api;
    #endif
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
        FWK_ID_ELEMENT_INIT(FAKE_MODULE_IDX, FAKE_SCMI_AGENT_IDX_OSPM);

    unsigned int agent_id = FAKE_SCMI_AGENT_IDX_OSPM;

    struct scmi_clock_rate_set_a2p payload = {
        .flags = SCMI_CLOCK_RATE_SET_ROUND_AUTO_MASK,
        .clock_id = CLOCK_DEV_IDX_VPU,
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

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(CLOCK_DEV_IDX_VPU);
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

int scmi_test_main(void)
{
    UNITY_BEGIN();
    #if defined(BUILD_HAS_MOD_RESOURCE_PERMS)
        RUN_TEST(test_function_set_rate);
    #else
        RUN_TEST(test_function_set_rate);
    #endif
    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_test_main();
}
#endif
