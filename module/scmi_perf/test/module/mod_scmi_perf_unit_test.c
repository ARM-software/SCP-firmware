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
#    include <Mockfwk_mm.h>
#    include <Mockfwk_module.h>

#    include <internal/Mockfwk_core_internal.h>
#endif
#include <Mockmod_scmi_perf_extra.h>
#include <config_scmi_perf.h>

#include <mod_dvfs.h>
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

static const struct mod_dvfs_domain_api dvfs_domain_api = {
    .get_current_opp = mod_dvfs_domain_api_get_current_opp,
    .get_sustained_opp = mod_dvfs_domain_api_get_sustained_opp,
    .get_nth_opp = mod_dvfs_domain_api_get_nth_opp,
    .get_level_id = mod_dvfs_domain_api_get_level_id,
    .get_opp_count = mod_dvfs_domain_api_get_opp_count,
    .get_latency = mod_dvfs_domain_api_get_latency,
    .set_level = mod_dvfs_domain_api_set_level,
};

char *name = "Test Name";

static int return_status;

void setUp(void)
{
    scmi_perf_ctx.scmi_api = &from_protocol_api;
    scmi_perf_ctx.config = config_scmi_perf.data;
    scmi_perf_ctx.domain_count = scmi_perf_ctx.config->perf_doms_count;

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    scmi_perf_ctx.fast_channels_rate_limit = SCMI_PERF_FC_MIN_RATE_LIMIT;
#endif

    fwk_id_get_api_idx_ExpectAnyArgsAndReturn(MOD_SCMI_PERF_PROTOCOL_API);

    module_scmi_perf.process_bind_request(
        fwk_module_id_scmi,
        fwk_module_id_scmi_perf,
        FWK_ID_API(FWK_MODULE_IDX_SCMI_PERF, 0),
        (const void **)&to_protocol_api);

    scmi_perf_ctx.dvfs_api = &dvfs_domain_api;
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

int version_handler_respond_callback_fail(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_protocol_version_p2a *return_values;
    return_values = (struct scmi_protocol_version_p2a *)payload;

    TEST_ASSERT_EQUAL(size, sizeof(int32_t));
    TEST_ASSERT_EQUAL(return_status, return_values->status);

    return FWK_SUCCESS;
}

void utest_scmi_perf_message_handler_bad_message_id(void)
{
    int status;
    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);
    struct scmi_protocol_version_p2a payload = { 0 };

    mod_scmi_from_protocol_api_respond_Stub(
        version_handler_respond_callback_fail);

    return_status = SCMI_NOT_SUPPORTED;
    status = scmi_perf_message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PROTOCOL_VERSION],
        MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_scmi_perf_message_handler_not_found(void)
{
    int status;
    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);
    struct scmi_protocol_version_p2a payload = { 0 };

    mod_scmi_from_protocol_api_respond_Stub(
        version_handler_respond_callback_fail);

    return_status = SCMI_NOT_FOUND;
    status = scmi_perf_message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PROTOCOL_VERSION],
        FWK_ARRAY_SIZE(handler_table));

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
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

/* Test the domain_attributes_handler function with a valid domain_id */

int domain_attributes_handler_valid_param_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_perf_domain_attributes_p2a *return_values;
    return_values = (struct scmi_perf_domain_attributes_p2a *)payload;

    TEST_ASSERT_EQUAL((int32_t)SCMI_SUCCESS, return_values->status);

    bool notifications = false;
    bool fast_channels = false;

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    fast_channels = true;
#endif

    uint32_t permissions = ((uint32_t)MOD_SCMI_PERF_PERMS_SET_LIMITS) |
        ((uint32_t)MOD_SCMI_PERF_PERMS_SET_LEVEL);

    uint32_t expected_attributes = SCMI_PERF_DOMAIN_ATTRIBUTES(
        (uint32_t)notifications,
        (uint32_t)notifications,
        ((permissions & (uint32_t)MOD_SCMI_PERF_PERMS_SET_LEVEL) !=
         (uint32_t)0) ?
            1U :
            0U,
        ((permissions & (uint32_t)MOD_SCMI_PERF_PERMS_SET_LIMITS) !=
         (uint32_t)0) ?
            1U :
            0U,
        (uint32_t)fast_channels);

    TEST_ASSERT_EQUAL(expected_attributes, return_values->attributes);

    TEST_ASSERT_EQUAL(0, return_values->rate_limit);

    struct mod_dvfs_opp expected_opp_values = test_dvfs_config.opps[0];

    TEST_ASSERT_EQUAL(
        expected_opp_values.frequency, return_values->sustained_freq);
    TEST_ASSERT_EQUAL(
        expected_opp_values.level, return_values->sustained_perf_level);

    TEST_ASSERT_EQUAL_STRING(name, (char *)return_values->name);

    return FWK_SUCCESS;
}

void utest_scmi_perf_domain_attributes_handler_valid_param(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_perf_domain_attributes_a2p payload = {
        .domain_id = 0,
    };

    unsigned int agent_id = TEST_SCMI_AGENT_IDX_0;

    mod_scmi_from_protocol_api_get_agent_id_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mod_scmi_from_protocol_api_get_agent_id_ReturnThruPtr_agent_id(&agent_id);

    struct mod_dvfs_opp test_opp_values = test_dvfs_config.opps[0];

    mod_dvfs_domain_api_get_sustained_opp_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mod_dvfs_domain_api_get_sustained_opp_ReturnThruPtr_opp(&test_opp_values);

    fwk_module_get_element_name_ExpectAnyArgsAndReturn(name);

    mod_scmi_from_protocol_api_respond_Stub(
        domain_attributes_handler_valid_param_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PERF_DOMAIN_ATTRIBUTES],
        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/* Test the domain_attributes_handler function with an invalid domain_id */
int domain_attributes_handler_invalid_param_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_perf_domain_attributes_p2a *return_values;
    return_values = (struct scmi_perf_domain_attributes_p2a *)payload;

    TEST_ASSERT_EQUAL((int32_t)SCMI_NOT_FOUND, return_values->status);

    return FWK_SUCCESS;
}

void utest_scmi_perf_domain_attributes_handler_invalid_param(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_perf_domain_attributes_a2p payload = {
        .domain_id = scmi_perf_ctx.domain_count,
    };

    mod_scmi_from_protocol_api_respond_Stub(
        domain_attributes_handler_invalid_param_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PERF_DOMAIN_ATTRIBUTES],
        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*
 * As the dvfs_get_nth_opp function is mocked, we must manually return the
 * requested OPP.
 */
int get_nth_opp_callback(
    fwk_id_t domain_id,
    size_t n,
    struct mod_dvfs_opp *opp,
    int NumCalls)
{
    *opp = test_dvfs_config.opps[n];
    return FWK_SUCCESS;
}

/*
 * Check that each of the returned levels is correct. Also, check that the
 * final returned status and number of levels is correct.
 */
int describe_levels_handler_valid_param_write_payload_callback(
    fwk_id_t service_id,
    size_t offset,
    const void *payload,
    size_t size,
    int NumCalls)
{
    if (NumCalls < TEST_OPP_COUNT) {
        struct scmi_perf_level *returned_perf_level =
            (struct scmi_perf_level *)payload;

        TEST_ASSERT_EQUAL(
            test_dvfs_config.opps[NumCalls].voltage,
            returned_perf_level->power_cost);
        TEST_ASSERT_EQUAL(
            test_dvfs_config.opps[NumCalls].level,
            returned_perf_level->performance_level);
        TEST_ASSERT_EQUAL(
            test_dvfs_config.latency, returned_perf_level->attributes);

    } else if (NumCalls == TEST_OPP_COUNT) {
        struct scmi_perf_describe_levels_p2a *return_values =
            (struct scmi_perf_describe_levels_p2a *)payload;
        TEST_ASSERT_EQUAL(SCMI_SUCCESS, return_values->status);
        TEST_ASSERT_EQUAL(TEST_OPP_COUNT, return_values->num_levels);

    } else {
        return FWK_E_RANGE;
    }

    return FWK_SUCCESS;
}

void utest_scmi_perf_describe_levels_handler_valid_param(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_perf_describe_levels_a2p payload = {
        .domain_id = 0,
        .level_index = 0,
    };

    size_t size = UINT16_MAX;
    size_t opp_count = TEST_OPP_COUNT;
    uint16_t latency = test_dvfs_config.latency;

    /*
     * Return an arbitrarily large max_payload_size as we are just testing
     * whether the returned levels are correct and in the right order
     */
    mod_scmi_from_protocol_api_get_max_payload_size_ExpectAnyArgsAndReturn(
        FWK_SUCCESS);
    mod_scmi_from_protocol_api_get_max_payload_size_ReturnThruPtr_size(&size);

    mod_dvfs_domain_api_get_opp_count_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mod_dvfs_domain_api_get_opp_count_ReturnThruPtr_opp_count(&opp_count);

    mod_dvfs_domain_api_get_latency_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mod_dvfs_domain_api_get_latency_ReturnThruPtr_latency(&latency);

    mod_dvfs_domain_api_get_nth_opp_Stub(get_nth_opp_callback);

    mod_scmi_from_protocol_api_write_payload_Stub(
        describe_levels_handler_valid_param_write_payload_callback);

    /* Disable the previous callback for this function */
    mod_scmi_from_protocol_api_respond_Stub(NULL);
    mod_scmi_from_protocol_api_respond_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PERF_DESCRIBE_LEVELS],
        MOD_SCMI_PERF_DESCRIBE_LEVELS);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/* Test the describe_levels_handler function with an invalid domain_id */
int describe_levels_handler_invalid_domain_id_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    int32_t *status = (int32_t *)payload;
    TEST_ASSERT_EQUAL((int32_t)SCMI_NOT_FOUND, *status);

    return FWK_SUCCESS;
}

void utest_scmi_perf_describe_levels_handler_invalid_domain_id(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_perf_describe_levels_a2p payload = {
        .domain_id = scmi_perf_ctx.domain_count,
        .level_index = 0,
    };

    size_t size = UINT16_MAX;

    mod_scmi_from_protocol_api_get_max_payload_size_ExpectAnyArgsAndReturn(
        FWK_SUCCESS);
    mod_scmi_from_protocol_api_get_max_payload_size_ReturnThruPtr_size(&size);

    mod_scmi_from_protocol_api_respond_Stub(
        describe_levels_handler_invalid_domain_id_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PERF_DESCRIBE_LEVELS],
        MOD_SCMI_PERF_DESCRIBE_LEVELS);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/* Test the describe_levels_handler function with an invalid level_index */
int describe_levels_handler_invalid_level_index_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    int32_t *status = (int32_t *)payload;
    TEST_ASSERT_EQUAL((int32_t)SCMI_INVALID_PARAMETERS, *status);

    return FWK_SUCCESS;
}

void utest_scmi_perf_describe_levels_handler_invalid_level_index(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_perf_describe_levels_a2p payload = {
        .domain_id = 0,
        .level_index = TEST_OPP_COUNT,
    };

    size_t size = UINT16_MAX;
    size_t opp_count = TEST_OPP_COUNT;

    mod_scmi_from_protocol_api_get_max_payload_size_ExpectAnyArgsAndReturn(
        FWK_SUCCESS);
    mod_scmi_from_protocol_api_get_max_payload_size_ReturnThruPtr_size(&size);

    mod_dvfs_domain_api_get_opp_count_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mod_dvfs_domain_api_get_opp_count_ReturnThruPtr_opp_count(&opp_count);

    mod_scmi_from_protocol_api_respond_Stub(
        describe_levels_handler_invalid_level_index_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PERF_DESCRIBE_LEVELS],
        MOD_SCMI_PERF_DESCRIBE_LEVELS);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS

/*
 * Test the scmi_perf_describe_fast_channels function with a set of valid
 * parameters
 */
int describe_fast_channels_valid_params_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_perf_describe_fc_p2a *return_values =
        (struct scmi_perf_describe_fc_p2a *)payload;

    TEST_ASSERT_EQUAL((int32_t)SCMI_SUCCESS, return_values->status);
    TEST_ASSERT_EQUAL(0, return_values->attributes);
    TEST_ASSERT_EQUAL(
        scmi_perf_ctx.fast_channels_rate_limit, return_values->rate_limit);

    int chan_index = (uint32_t)MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET;
    TEST_ASSERT_EQUAL(
        (uint32_t)(domains[0].fast_channels_addr_ap[chan_index] & ~0UL),
        return_values->chan_addr_low);
    TEST_ASSERT_EQUAL(
        (uint32_t)(domains[0].fast_channels_addr_ap[chan_index] >> 32),
        return_values->chan_addr_high);

    TEST_ASSERT_EQUAL(
        fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET],
        return_values->chan_size);

    return FWK_SUCCESS;
}

void utest_scmi_perf_describe_fast_channels_valid_params(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_perf_describe_fc_a2p payload = {
        .domain_id = 0,
        .message_id = MOD_SCMI_PERF_LEVEL_GET,
    };

    mod_scmi_from_protocol_api_respond_Stub(
        describe_fast_channels_valid_params_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL],
        MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*
 * Test the scmi_perf_describe_fast_channels function with an invalid
 * domain_id
 */
int describe_fast_channels_invalid_domain_id_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_perf_describe_fc_p2a *return_values =
        (struct scmi_perf_describe_fc_p2a *)payload;

    TEST_ASSERT_EQUAL((int32_t)SCMI_NOT_FOUND, return_values->status);

    return FWK_SUCCESS;
}

void utest_scmi_perf_describe_fast_channels_invalid_domain_id(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_perf_describe_fc_a2p payload = {
        .domain_id = scmi_perf_ctx.domain_count,
        .message_id = MOD_SCMI_PERF_LEVEL_GET,
    };

    mod_scmi_from_protocol_api_respond_Stub(
        describe_fast_channels_invalid_domain_id_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL],
        MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*
 * Test the scmi_perf_describe_fast_channels function with an invalid
 * message_id
 */
int describe_fast_channels_invalid_message_id_respond_callback(
    fwk_id_t service_id,
    const void *payload,
    size_t size,
    int NumCalls)
{
    struct scmi_perf_describe_fc_p2a *return_values =
        (struct scmi_perf_describe_fc_p2a *)payload;

    TEST_ASSERT_EQUAL((int32_t)SCMI_NOT_FOUND, return_values->status);

    return FWK_SUCCESS;
}

void utest_scmi_perf_describe_fast_channels_invalid_message_id(void)
{
    int status;

    fwk_id_t service_id =
        FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, TEST_SCMI_AGENT_IDX_0);

    struct scmi_perf_describe_fc_a2p payload = {
        .domain_id = 0,
        .message_id = MOD_SCMI_PERF_COMMAND_COUNT,
    };

    mod_scmi_from_protocol_api_respond_Stub(
        describe_fast_channels_invalid_message_id_respond_callback);

    status = to_protocol_api->message_handler(
        (fwk_id_t)MOD_SCMI_PROTOCOL_ID_PERF,
        service_id,
        (const uint32_t *)&payload,
        payload_size_table[MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL],
        MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

#endif

/*
 * Test the find_opp_for_level function with a valid level, without
 * use_nearest
 */
void utest_find_opp_for_level_valid_level(void)
{
    int status;
    uint32_t valid_level = test_dvfs_config.opps[1].level;

    struct perf_opp_table opp_table = {
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    };

    struct mod_scmi_perf_level_limits level_limits = {
        .minimum = test_dvfs_config.opps[0].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    struct scmi_perf_domain_ctx domain_ctx = {
        .level_limits = level_limits,
        .opp_table = &opp_table,
    };

    status = find_opp_for_level(&domain_ctx, &valid_level, false);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(opp_table.opps[1].level, valid_level);
}

/*
 * Test the find_opp_for_level function with an invalid level, without
 * use_nearest
 */
void utest_find_opp_for_level_invalid_level(void)
{
    int status;
    uint32_t invalid_level = test_dvfs_config.opps[1].level + 1000000UL;

    struct perf_opp_table opp_table = {
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    };

    struct mod_scmi_perf_level_limits level_limits = {
        .minimum = test_dvfs_config.opps[0].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    struct scmi_perf_domain_ctx domain_ctx = {
        .level_limits = level_limits,
        .opp_table = &opp_table,
    };

    status = find_opp_for_level(&domain_ctx, &invalid_level, false);
    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

/*
 * Test the find_opp_for_level function with use_nearest. In this case, expect
 * an OPP to be found for any level.
 */
void utest_find_opp_for_level_use_nearest(void)
{
    int status;

    struct perf_opp_table opp_table = {
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    };

    struct mod_scmi_perf_level_limits level_limits = {
        .minimum = test_dvfs_config.opps[0].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    struct scmi_perf_domain_ctx domain_ctx = {
        .level_limits = level_limits,
        .opp_table = &opp_table,
    };

    uint32_t approximate_level = level_limits.maximum + 1000000UL;

    status = find_opp_for_level(&domain_ctx, &approximate_level, true);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(level_limits.maximum, approximate_level);
}

/*
 * Test the validate_new_limits function with a valid set of limits, without
 * approximating the level.
 */
void utest_validate_new_limits_valid_limits(void)
{
    int status;

    struct perf_opp_table opp_table = {
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    };

    struct mod_scmi_perf_level_limits level_limits = {
        .minimum = test_dvfs_config.opps[0].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    struct scmi_perf_domain_ctx domain_ctx = {
        .level_limits = level_limits,
        .opp_table = &opp_table,
    };

    struct mod_scmi_perf_level_limits valid_level_limits = {
        .minimum = test_dvfs_config.opps[1].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    status = validate_new_limits(&domain_ctx, &valid_level_limits);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*
 * Test the validate_new_limits function with an invalid set of limits, without
 * approximating the level.
 */
void utest_validate_new_limits_invalid_limits(void)
{
    int status;

    struct perf_opp_table opp_table = {
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    };

    struct mod_scmi_perf_level_limits level_limits = {
        .minimum = test_dvfs_config.opps[0].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    struct scmi_perf_domain_ctx domain_ctx = {
        .level_limits = level_limits,
        .opp_table = &opp_table,
    };

    struct mod_scmi_perf_level_limits invalid_level_limits = {
        .minimum = test_dvfs_config.opps[0].level + 1000000UL,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    status = validate_new_limits(&domain_ctx, &invalid_level_limits);
    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

/*
 * Test the validate_new_limits function, approximating the level. In this
 * case, expect any set of limits to succeed.
 */
void utest_validate_new_limits_approximate_level(void)
{
    int status;

    struct fwk_module_config config_scmi_perf_approximate_level = {
        .data = &((struct mod_scmi_perf_config){
            .domains = &domains,
            .perf_doms_count = FWK_ARRAY_SIZE(domains),
            .fast_channels_alarm_id = FWK_ID_NONE_INIT,
            .approximate_level = true,
        }),
    };

    scmi_perf_ctx.config = config_scmi_perf_approximate_level.data;

    struct perf_opp_table opp_table = {
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    };
    struct mod_scmi_perf_level_limits level_limits = {
        .minimum = test_dvfs_config.opps[0].level - 1000000UL,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level + 1000000UL,
    };

    struct scmi_perf_domain_ctx domain_ctx = {
        .level_limits = level_limits,
        .opp_table = &opp_table,
    };

    status = validate_new_limits(&domain_ctx, &level_limits);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

int scmi_perf_test_main(void)
{
    UNITY_BEGIN();

#ifndef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    RUN_TEST(utest_scmi_perf_message_handler_bad_message_id);
    RUN_TEST(utest_scmi_perf_message_handler_not_found);
#endif

    RUN_TEST(utest_scmi_perf_protocol_version_handler);
    RUN_TEST(utest_scmi_perf_protocol_attributes_handler);

    RUN_TEST(utest_scmi_perf_protocol_message_attributes_handler_valid_param);
    RUN_TEST(utest_scmi_perf_protocol_message_attributes_handler_invalid_param);

    RUN_TEST(utest_scmi_perf_domain_attributes_handler_valid_param);
    RUN_TEST(utest_scmi_perf_domain_attributes_handler_invalid_param);

    RUN_TEST(utest_scmi_perf_describe_levels_handler_valid_param);
    RUN_TEST(utest_scmi_perf_describe_levels_handler_invalid_domain_id);
    RUN_TEST(utest_scmi_perf_describe_levels_handler_invalid_level_index);

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    RUN_TEST(utest_scmi_perf_describe_fast_channels_valid_params);
    RUN_TEST(utest_scmi_perf_describe_fast_channels_invalid_domain_id);
    RUN_TEST(utest_scmi_perf_describe_fast_channels_invalid_message_id);
#endif

    RUN_TEST(utest_find_opp_for_level_valid_level);
    RUN_TEST(utest_find_opp_for_level_invalid_level);
    RUN_TEST(utest_find_opp_for_level_use_nearest);

    RUN_TEST(utest_validate_new_limits_valid_limits);
    RUN_TEST(utest_validate_new_limits_invalid_limits);
    RUN_TEST(utest_validate_new_limits_approximate_level);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_perf_test_main();
}
#endif
