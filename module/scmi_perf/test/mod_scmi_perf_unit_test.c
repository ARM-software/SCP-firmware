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

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
#    include <perf_plugins_handler.c>
#    include <perf_plugins_handler.h>
#endif

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

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
static int plugin_api_num_calls;
static int plugin_update_num_calls;
static struct perf_plugins_dev_ctx test_dev_ctx[DVFS_ELEMENT_IDX_COUNT];

/*
 * Helper to allocate plugins-handler's own memory
 */
static void malloc_perf_plugins_alloc_tables(
    size_t count,
    struct perf_plugins_perf_update *table)
{
    table->level = malloc(count * sizeof(uint32_t));
    table->max_limit = malloc(count * sizeof(uint32_t));
    table->adj_max_limit = malloc(count * sizeof(uint32_t));
    table->min_limit = malloc(count * sizeof(uint32_t));
    table->adj_min_limit = malloc(count * sizeof(uint32_t));

    fwk_mm_calloc_ExpectAndReturn(count, sizeof(uint32_t), table->level);

    fwk_mm_calloc_ExpectAndReturn(count, sizeof(uint32_t), table->max_limit);
    fwk_mm_calloc_ExpectAndReturn(
        count, sizeof(uint32_t), table->adj_max_limit);

    fwk_mm_calloc_ExpectAndReturn(count, sizeof(uint32_t), table->min_limit);
    fwk_mm_calloc_ExpectAndReturn(
        count, sizeof(uint32_t), table->adj_min_limit);
}

/*
 * Helper to free plugins-handler's own memory
 */
static void dealloc_perf_plugins_alloc_tables(
    struct perf_plugins_perf_update *table)
{
    free(table->level);
    free(table->max_limit);
    free(table->adj_max_limit);
    free(table->min_limit);
    free(table->adj_min_limit);
}

/*
 * Helpers for internal data handling
 */
static void malloc_dev_ctx_tables(
    size_t count,
    struct perf_plugins_perf_update *table)
{
    table->level = malloc(count * sizeof(uint32_t));
    table->max_limit = malloc(count * sizeof(uint32_t));
    table->adj_max_limit = malloc(count * sizeof(uint32_t));
    table->min_limit = malloc(count * sizeof(uint32_t));
    table->adj_min_limit = malloc(count * sizeof(uint32_t));
}

static void free_dev_ctx_tables(struct perf_plugins_perf_update *table)
{
    free(table->level);
    free(table->max_limit);
    free(table->adj_max_limit);
    free(table->min_limit);
    free(table->adj_min_limit);
}

static void domain_aggregate_malloc(
    struct perf_plugins_perf_update *this_dom,
    struct perf_plugins_perf_update *phy_dom)
{
    malloc_dev_ctx_tables(2, &test_dev_ctx[0].perf_table);

    this_dom->level = &test_dev_ctx[0].perf_table.level[0];
    this_dom->max_limit = &test_dev_ctx[0].perf_table.max_limit[0];
    this_dom->min_limit = &test_dev_ctx[0].perf_table.min_limit[0];

    phy_dom->level = &test_dev_ctx[0].perf_table.level[1];
    phy_dom->max_limit = &test_dev_ctx[0].perf_table.max_limit[1];
    phy_dom->min_limit = &test_dev_ctx[0].perf_table.min_limit[1];
}

static void domain_aggregate_free(void)
{
    free(test_dev_ctx[0].perf_table.level);
    free(test_dev_ctx[0].perf_table.max_limit);
    free(test_dev_ctx[0].perf_table.adj_max_limit);
    free(test_dev_ctx[0].perf_table.min_limit);
    free(test_dev_ctx[0].perf_table.adj_min_limit);
}

#endif

void setUp(void)
{
    scmi_perf_ctx.scmi_api = &from_protocol_api;
    scmi_perf_ctx.config = config_scmi_perf.data;
    scmi_perf_ctx.domain_count = scmi_perf_ctx.config->perf_doms_count;

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    scmi_perf_ctx.fast_channels_rate_limit = SCMI_PERF_FC_MIN_RATE_LIMIT;
#endif

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
    perf_config.plugins_count = FWK_ARRAY_SIZE(plugins_table);
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

/* Test the describe_levels_handler function with a set of valid parameters */

/*
 * As the dvfs_get_nth_opp function is mocked, we must manually return the
 * requested OPP
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
 * approximating the level
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
 * approximating the level
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

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
void utest_perf_eval_performance_invalid_limits(void)
{
    struct mod_scmi_perf_level_limits limits = {
        .minimum = 1000,
        .maximum = 100,
    };
    uint32_t level = 500;
    fwk_id_t perf_id = FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, 0);

    perf_eval_performance(perf_id, &limits, &level);
    TEST_ASSERT_EQUAL(500, level);
}

void utest_perf_eval_performance_unchanged_limits(void)
{
    struct scmi_perf_domain_ctx domain0_ctx;
    uint32_t level;

    domain0_ctx.level_limits.minimum = test_dvfs_config.opps[0].level;
    domain0_ctx.level_limits.maximum =
        test_dvfs_config.opps[TEST_OPP_COUNT - 1].level;

    struct mod_scmi_perf_level_limits limits = {
        .minimum = test_dvfs_config.opps[0].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    domain0_ctx.opp_table = &((struct perf_opp_table){
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    });

    scmi_perf_ctx.domain_ctx_table = &domain0_ctx;
    /*
     * The level to be evaluated is just above the second OPP. Due to the
     * default approximation for `perf_eval_performance`, it should return the
     * next OPP just above it.
     */
    level = test_dvfs_config.opps[1].level + 1;
    fwk_id_t perf_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_PERF, 0);

    fwk_id_get_element_idx_ExpectAndReturn(perf_id, 0);

    perf_eval_performance(perf_id, &limits, &level);
    TEST_ASSERT_EQUAL(test_dvfs_config.opps[2].level, level);
}

void utest_perf_eval_performance_unchanged_limits_level_down(void)
{
    struct scmi_perf_domain_ctx domain0_ctx;
    uint32_t level;

    domain0_ctx.level_limits.minimum = test_dvfs_config.opps[0].level;
    domain0_ctx.level_limits.maximum =
        test_dvfs_config.opps[TEST_OPP_COUNT - 2].level;

    struct mod_scmi_perf_level_limits limits = {
        .minimum = test_dvfs_config.opps[0].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 2].level,
    };

    domain0_ctx.opp_table = &((struct perf_opp_table){
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    });

    scmi_perf_ctx.domain_ctx_table = &domain0_ctx;
    /* The level will be reduced to the max limit */
    level = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level;
    fwk_id_t perf_id = FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, 0);

    fwk_id_get_element_idx_ExpectAndReturn(perf_id, 0);

    perf_eval_performance(perf_id, &limits, &level);
    TEST_ASSERT_EQUAL(test_dvfs_config.opps[TEST_OPP_COUNT - 2].level, level);
}

void utest_perf_eval_performance_unchanged_limits_level_up(void)
{
    struct scmi_perf_domain_ctx domain0_ctx;
    uint32_t level;

    domain0_ctx.level_limits.minimum = test_dvfs_config.opps[1].level;
    domain0_ctx.level_limits.maximum =
        test_dvfs_config.opps[TEST_OPP_COUNT - 1].level;

    struct mod_scmi_perf_level_limits limits = {
        .minimum = test_dvfs_config.opps[1].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    domain0_ctx.opp_table = &((struct perf_opp_table){
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    });

    scmi_perf_ctx.domain_ctx_table = &domain0_ctx;
    /* The level will be increased to the min limit */
    level = test_dvfs_config.opps[0].level;
    fwk_id_t perf_id = FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, 0);

    fwk_id_get_element_idx_ExpectAndReturn(perf_id, 0);

    perf_eval_performance(perf_id, &limits, &level);
    TEST_ASSERT_EQUAL(test_dvfs_config.opps[1].level, level);
}

void utest_perf_eval_performance_new_limits(void)
{
    struct scmi_perf_domain_ctx domain_table[2];
    struct perf_plugins_api *plugins_api_table[1];
    uint32_t level;

    domain_table[1].level_limits.minimum = test_dvfs_config.opps[0].level;
    domain_table[1].level_limits.maximum =
        test_dvfs_config.opps[TEST_OPP_COUNT - 2].level;

    struct mod_scmi_perf_level_limits new_limits = {
        .minimum = test_dvfs_config.opps[0].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    domain_table[1].opp_table = &((struct perf_opp_table){
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    });

    scmi_perf_ctx.domain_ctx_table = &domain_table[0];
    level = test_dvfs_config.opps[1].level;
    fwk_id_t perf_id = FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, 1);

    fwk_id_get_element_idx_ExpectAndReturn(perf_id, 1);
    fwk_id_get_element_idx_ExpectAndReturn(perf_id, 1);

    perf_plugins_ctx.config = config_scmi_perf.data;
    plugins_api_table[0] = &((struct perf_plugins_api){
        .update = NULL,
        .report = NULL,
    });
    perf_plugins_ctx.plugins_api_table = &plugins_api_table[0];

    perf_eval_performance(perf_id, &new_limits, &level);
    TEST_ASSERT_EQUAL(test_dvfs_config.opps[1].level, level);
}

void utest_perf_eval_performance_new_limits_level_down(void)
{
    struct scmi_perf_domain_ctx domain_table[2];
    struct perf_plugins_api *plugins_api_table[1];
    uint32_t level;

    domain_table[1].level_limits.minimum = test_dvfs_config.opps[0].level;
    domain_table[1].level_limits.maximum =
        test_dvfs_config.opps[TEST_OPP_COUNT - 1].level;

    struct mod_scmi_perf_level_limits new_limits = {
        .minimum = test_dvfs_config.opps[0].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 2].level,
    };

    domain_table[1].opp_table = &((struct perf_opp_table){
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    });

    scmi_perf_ctx.domain_ctx_table = &domain_table[0];
    /* The level will be reduced to the max limit */
    level = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level;
    fwk_id_t perf_id = FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, 1);

    fwk_id_get_element_idx_ExpectAndReturn(perf_id, 1);
    fwk_id_get_element_idx_ExpectAndReturn(perf_id, 1);

    perf_plugins_ctx.config = config_scmi_perf.data;
    plugins_api_table[0] = &((struct perf_plugins_api){
        .update = NULL,
        .report = NULL,
    });
    perf_plugins_ctx.plugins_api_table = &plugins_api_table[0];

    perf_eval_performance(perf_id, &new_limits, &level);
    TEST_ASSERT_EQUAL(test_dvfs_config.opps[TEST_OPP_COUNT - 2].level, level);
}

void utest_perf_eval_performance_new_limits_level_up(void)
{
    struct scmi_perf_domain_ctx domain_table[2];
    struct perf_plugins_api *plugins_api_table[1];
    uint32_t level;

    domain_table[1].level_limits.minimum = test_dvfs_config.opps[0].level;
    domain_table[1].level_limits.maximum =
        test_dvfs_config.opps[TEST_OPP_COUNT - 1].level;

    struct mod_scmi_perf_level_limits new_limits = {
        .minimum = test_dvfs_config.opps[1].level,
        .maximum = test_dvfs_config.opps[TEST_OPP_COUNT - 1].level,
    };

    domain_table[1].opp_table = &((struct perf_opp_table){
        .opps = test_dvfs_config.opps,
        .opp_count = TEST_OPP_COUNT,
    });

    scmi_perf_ctx.domain_ctx_table = &domain_table[0];
    /* The level will be increased to the min limit */
    level = test_dvfs_config.opps[0].level;
    fwk_id_t perf_id = FWK_ID_ELEMENT_INIT(TEST_MODULE_IDX, 1);

    fwk_id_get_element_idx_ExpectAndReturn(perf_id, 1);
    fwk_id_get_element_idx_ExpectAndReturn(perf_id, 1);

    perf_plugins_ctx.config = config_scmi_perf.data;
    plugins_api_table[0] = &((struct perf_plugins_api){
        .update = NULL,
        .report = NULL,
    });
    perf_plugins_ctx.plugins_api_table = &plugins_api_table[0];

    perf_eval_performance(perf_id, &new_limits, &level);
    TEST_ASSERT_EQUAL(test_dvfs_config.opps[1].level, level);
}

void utest_perf_plugins_handler_init_fail_no_dvfs_doms(void)
{
    int status;
    fwk_module_get_element_count_ExpectAnyArgsAndReturn(0);

    status = perf_plugins_handler_init(config_scmi_perf.data);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void utest_perf_plugins_handler_init_success(void)
{
    struct perf_plugins_dev_ctx dev_ctx[DVFS_ELEMENT_IDX_COUNT];
    fwk_id_t dep_id_table[SCMI_PERF_ELEMENT_IDX_COUNT];
    int status;

    memset(dev_ctx, 0, sizeof(*dev_ctx) * DVFS_ELEMENT_IDX_COUNT);

    fwk_module_get_element_count_ExpectAnyArgsAndReturn(DVFS_ELEMENT_IDX_COUNT);

    fwk_mm_calloc_ExpectAndReturn(
        DVFS_ELEMENT_IDX_COUNT,
        sizeof(struct perf_plugins_dev_ctx),
        &dev_ctx[0]);

    malloc_perf_plugins_alloc_tables(2, &dev_ctx[0].perf_table);
    malloc_perf_plugins_alloc_tables(3, &dev_ctx[1].perf_table);

    malloc_perf_plugins_alloc_tables(
        DVFS_ELEMENT_IDX_COUNT, &perf_plugins_ctx.full_perf_table);

    fwk_mm_calloc_ExpectAndReturn(
        SCMI_PERF_ELEMENT_IDX_COUNT, sizeof(fwk_id_t), &dep_id_table[0]);

    /* To build the number of logical domains */
    fwk_optional_id_is_defined_ExpectAndReturn(domains[0].phy_group_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(
        domains[0].phy_group_id, DVFS_ELEMENT_IDX_0);

    fwk_optional_id_is_defined_ExpectAndReturn(domains[1].phy_group_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(
        domains[1].phy_group_id, DVFS_ELEMENT_IDX_1);

    fwk_optional_id_is_defined_ExpectAndReturn(domains[2].phy_group_id, true);
    fwk_id_get_element_idx_ExpectAndReturn(
        domains[2].phy_group_id, DVFS_ELEMENT_IDX_1);

    /* To build dependency domains */
    fwk_optional_id_is_defined_ExpectAndReturn(domains[0].phy_group_id, false);
    fwk_optional_id_is_defined_ExpectAndReturn(domains[0].phy_group_id, false);
    fwk_optional_id_is_defined_ExpectAndReturn(domains[0].phy_group_id, false);

    status = perf_plugins_handler_init(config_scmi_perf.data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        dealloc_perf_plugins_alloc_tables(&dev_ctx[i].perf_table);
    }
    dealloc_perf_plugins_alloc_tables(&perf_plugins_ctx.full_perf_table);
}

void utest_perf_plugins_handler_bind_success_no_plugins(void)
{
    int status;

    perf_config.plugins_count = 0;
    perf_plugins_ctx.config = config_scmi_perf.data;

    status = perf_plugins_handler_bind();
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    perf_config.plugins_count = FWK_ARRAY_SIZE(plugins_table);
}

void utest_perf_plugins_handler_bind_success(void)
{
    struct perf_plugins_api plugins_api_table[PERF_PLUGIN_IDX_COUNT];
    int status;

    fwk_mm_calloc_ExpectAndReturn(
        perf_config.plugins_count,
        sizeof(struct perf_plugins_api *),
        &plugins_api_table);

    fwk_id_get_module_idx_ExpectAndReturn(
        plugins_table[0].id, FWK_MODULE_IDX_PERF_PLUGIN);

    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = perf_plugins_handler_bind();
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_perf_plugins_handler_bind_fail(void)
{
    struct perf_plugins_api plugins_api_table[PERF_PLUGIN_IDX_COUNT];
    int status;

    fwk_mm_calloc_ExpectAndReturn(
        perf_config.plugins_count,
        sizeof(struct perf_plugins_api *),
        &plugins_api_table);

    fwk_id_get_module_idx_ExpectAndReturn(
        plugins_table[0].id, FWK_MODULE_IDX_PERF_PLUGIN);

    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_E_PANIC);

    status = perf_plugins_handler_bind();
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_perf_plugins_handler_process_bind_request_success(void)
{
    struct perf_plugins_handler_api *ph_api;
    int status;

    status = perf_plugins_handler_process_bind_request(
        FWK_ID_NONE, FWK_ID_NONE, FWK_ID_NONE, (const void **)&ph_api);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(ph_api, &handler_api);
}

int perf_plugins_handler_report_callback_no_call(
    struct perf_plugins_perf_report *data,
    int cmock_num_calls)
{
    TEST_ASSERT(false);
}

/*
 * If the plugin does not implement the report API, then the callback should not
 * be called
 */
void utest_perf_plugins_handler_report_null_api(void)
{
    struct perf_plugins_perf_report data;
    struct perf_plugins_api *plugins_api_table[1];

    plugins_api_table[0] = &((struct perf_plugins_api){
        .update = NULL,
        .report = NULL,
    });
    perf_plugins_ctx.plugins_api_table = &plugins_api_table[0];

    plugin_report_Stub(perf_plugins_handler_report_callback_no_call);
    perf_plugins_handler_report(&data);
    TEST_ASSERT(true);

    plugin_report_Stub(NULL);
}

int perf_plugins_handler_report_callback_plugin_call_success(
    struct perf_plugins_perf_report *data,
    int cmock_num_calls)
{
    plugin_api_num_calls++;
    return FWK_SUCCESS;
}

/* The plugin implements the report API, then the callback should be called */
void utest_perf_plugins_handler_report_plugin_success(void)
{
    struct perf_plugins_perf_report data;

    perf_plugins_ctx.plugins_api_table[0] = &test_perf_plugins_api;

    plugin_report_Stub(
        perf_plugins_handler_report_callback_plugin_call_success);

    plugin_api_num_calls = 0;
    perf_plugins_handler_report(&data);
    TEST_ASSERT_EQUAL(plugin_api_num_calls, perf_config.plugins_count);

    plugin_report_Stub(NULL);
}

int perf_plugins_handler_report_callback_plugin_call_fail(
    struct perf_plugins_perf_report *data,
    int cmock_num_calls)
{
    plugin_api_num_calls++;
    return FWK_E_PANIC;
}

/*
 * The plugin implements the report API, but its implementation returns an error
 * which is discared by the plugins-handler.
 */
void utest_perf_plugins_handler_report_plugin_fail(void)
{
    struct perf_plugins_perf_report data;

    perf_plugins_ctx.plugins_api_table[0] = &test_perf_plugins_api;

    plugin_report_Stub(perf_plugins_handler_report_callback_plugin_call_fail);

    plugin_api_num_calls = 0;
    perf_plugins_handler_report(&data);
    TEST_ASSERT_EQUAL(plugin_api_num_calls, perf_config.plugins_count);

    plugin_report_Stub(NULL);
}

/*
 * The aggregation policy for level is: MAX(logical-domains).
 * If a logical domain has higher value than what's stored for the physical
 * domain, then it will be used.
 */
void utest_domain_aggregate_test_policy_level_up(void)
{
    struct perf_plugins_perf_update this_dom;
    struct perf_plugins_perf_update phy_dom;

    domain_aggregate_malloc(&this_dom, &phy_dom);

    this_dom.level[0] = 6;
    this_dom.max_limit[0] = 10;
    this_dom.min_limit[0] = 1;

    phy_dom.level[0] = 5;
    phy_dom.max_limit[0] = 10;
    phy_dom.min_limit[0] = 1;

    domain_aggregate(&this_dom, &phy_dom);

    TEST_ASSERT_EQUAL(phy_dom.level[0], 6);

    domain_aggregate_free();
}

/*
 * The aggregation policy for level is: MAX(logical-domains).
 * If a logical domain has lower value than what's stored for the physical
 * domain, then it will not affect the domain.
 */
void utest_domain_aggregate_test_policy_level_down(void)
{
    struct perf_plugins_perf_update this_dom;
    struct perf_plugins_perf_update phy_dom;

    domain_aggregate_malloc(&this_dom, &phy_dom);

    this_dom.level[0] = 4;
    this_dom.max_limit[0] = 10;
    this_dom.min_limit[0] = 1;

    phy_dom.level[0] = 5;
    phy_dom.max_limit[0] = 10;
    phy_dom.min_limit[0] = 1;

    domain_aggregate(&this_dom, &phy_dom);

    TEST_ASSERT_EQUAL(phy_dom.level[0], 5);

    domain_aggregate_free();
}

/*
 * The aggregation policy for max_limit is: MIN(logical-domains).
 * If a logical domain has bigger max_limit than what's stored for the physical
 * domain, then it will not affect the domain.
 */
void utest_domain_aggregate_test_policy_max_limit_up(void)
{
    struct perf_plugins_perf_update this_dom;
    struct perf_plugins_perf_update phy_dom;

    domain_aggregate_malloc(&this_dom, &phy_dom);

    this_dom.level[0] = 5;
    this_dom.max_limit[0] = 11;
    this_dom.min_limit[0] = 1;

    phy_dom.level[0] = 5;
    phy_dom.max_limit[0] = 10;
    phy_dom.min_limit[0] = 1;

    domain_aggregate(&this_dom, &phy_dom);

    TEST_ASSERT_EQUAL(phy_dom.max_limit[0], 10);

    domain_aggregate_free();
}

/*
 * The aggregation policy for max_limit is: MIN(logical-domains).
 * If a logical domain has lower max_limit than what's stored for the physical
 * domain, then it will affect the domain.
 */
void utest_domain_aggregate_test_policy_max_limit_down(void)
{
    struct perf_plugins_perf_update this_dom;
    struct perf_plugins_perf_update phy_dom;

    domain_aggregate_malloc(&this_dom, &phy_dom);

    this_dom.level[0] = 5;
    this_dom.max_limit[0] = 9;
    this_dom.min_limit[0] = 1;

    phy_dom.level[0] = 5;
    phy_dom.max_limit[0] = 10;
    phy_dom.min_limit[0] = 1;

    domain_aggregate(&this_dom, &phy_dom);

    TEST_ASSERT_EQUAL(phy_dom.max_limit[0], 9);

    domain_aggregate_free();
}

/*
 * The aggregation policy for min_limit is: MAX(logical-domains).
 * If a logical domain has higher min_limit than what's stored for the physical
 * domain, then it will affect the domain.
 */
void utest_domain_aggregate_test_policy_min_limit_up(void)
{
    struct perf_plugins_perf_update this_dom;
    struct perf_plugins_perf_update phy_dom;

    domain_aggregate_malloc(&this_dom, &phy_dom);

    this_dom.level[0] = 5;
    this_dom.max_limit[0] = 10;
    this_dom.min_limit[0] = 2;

    phy_dom.level[0] = 5;
    phy_dom.max_limit[0] = 10;
    phy_dom.min_limit[0] = 1;

    domain_aggregate(&this_dom, &phy_dom);

    TEST_ASSERT_EQUAL(phy_dom.min_limit[0], 2);

    domain_aggregate_free();
}

/*
 * The aggregation policy for min_limit is: MAX(logical-domains).
 * If a logical domain has lower min_limit than what's stored for the physical
 * domain, then it will not affect the domain.
 */
void utest_domain_aggregate_test_policy_min_limit_down(void)
{
    struct perf_plugins_perf_update this_dom;
    struct perf_plugins_perf_update phy_dom;

    domain_aggregate_malloc(&this_dom, &phy_dom);

    this_dom.level[0] = 5;
    this_dom.max_limit[0] = 10;
    this_dom.min_limit[0] = 1;

    phy_dom.level[0] = 5;
    phy_dom.max_limit[0] = 10;
    phy_dom.min_limit[0] = 0;

    domain_aggregate(&this_dom, &phy_dom);

    TEST_ASSERT_EQUAL(phy_dom.min_limit[0], 1);

    domain_aggregate_free();
}

/*
 * When there are no plugins, only the domain aggregation takes place.
 * For the first logical domain, the storage-and-aggregation will populate the
 * internal tables with the values given.
 */
void utest_perf_plugins_handler_update_no_plugins_first_log_dom(void)
{
    struct perf_plugins_dev_ctx dev_ctx[DVFS_ELEMENT_IDX_COUNT];
    struct fc_perf_update fc_update;
    fwk_id_t dep_id_table[SCMI_PERF_ELEMENT_IDX_COUNT];
    fwk_id_t dep_dom_id;

    memset(&dev_ctx[0], 0, sizeof(dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &dev_ctx[0];

    perf_plugins_ctx.dep_id_table = &dep_id_table[0];

    malloc_dev_ctx_tables(2, &dev_ctx[DVFS_ELEMENT_IDX_0].perf_table);
    malloc_dev_ctx_tables(3, &dev_ctx[DVFS_ELEMENT_IDX_1].perf_table);
    malloc_dev_ctx_tables(
        DVFS_ELEMENT_IDX_COUNT, &perf_plugins_ctx.full_perf_table);

    perf_config.plugins_count = 0;

    dev_ctx[DVFS_ELEMENT_IDX_0].log_dom_count = 1;
    dev_ctx[DVFS_ELEMENT_IDX_1].log_dom_count = 2;

    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_0] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);
    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_1] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 0);
    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_2] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 1);

    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 0);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

    fc_update = (struct fc_perf_update){
        .domain_id = dep_dom_id,
        .level = 5,
        .max_limit = 10,
        .min_limit = 1,
    };

    perf_plugins_handler_update(SCMI_PERF_ELEMENT_IDX_1, &fc_update);

    /* Test internal storage for the logical domain took place */
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[0], 5);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[0], 10);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[0], 1);

    /* Test aggregation for the physical domain took place */
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[2], 5);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[2], 10);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[2], 1);

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        free_dev_ctx_tables(&dev_ctx[i].perf_table);
    }
    free_dev_ctx_tables(&perf_plugins_ctx.full_perf_table);

    perf_config.plugins_count = PERF_PLUGIN_IDX_COUNT;
}

/*
 * When there are no plugins, only the domain aggregation takes place.
 * For the first logical domain, the storage-and-aggregation will populate the
 * internal tables with the values given.
 */
void utest_perf_plugins_handler_update_no_plugins_last_log_dom(void)
{
    struct perf_plugins_dev_ctx dev_ctx[DVFS_ELEMENT_IDX_COUNT];
    struct fc_perf_update fc_update;
    fwk_id_t dep_id_table[SCMI_PERF_ELEMENT_IDX_COUNT];
    fwk_id_t dep_dom_id;

    memset(&dev_ctx[0], 0, sizeof(dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &dev_ctx[0];

    perf_plugins_ctx.dep_id_table = &dep_id_table[0];

    malloc_dev_ctx_tables(2, &dev_ctx[DVFS_ELEMENT_IDX_0].perf_table);
    malloc_dev_ctx_tables(3, &dev_ctx[DVFS_ELEMENT_IDX_1].perf_table);
    malloc_dev_ctx_tables(
        DVFS_ELEMENT_IDX_COUNT, &perf_plugins_ctx.full_perf_table);

    perf_config.plugins_count = 0;

    dev_ctx[DVFS_ELEMENT_IDX_0].log_dom_count = 1;
    dev_ctx[DVFS_ELEMENT_IDX_1].log_dom_count = 2;

    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_0] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);
    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_1] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 0);
    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_2] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 1);

    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 1);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 1);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 1);

    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[0] = 5;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[0] = 10;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[0] = 1;

    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[2] = 5;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[2] = 10;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[2] = 1;

    fc_update = (struct fc_perf_update){
        .domain_id = dep_dom_id,
        .level = 6,
        .max_limit = 11,
        .min_limit = 2,
    };

    perf_plugins_handler_update(SCMI_PERF_ELEMENT_IDX_2, &fc_update);

    /* Test internal storage for the other logical domain did not change */
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[0], 5);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[0], 10);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[0], 1);

    /* Test internal storage for the logical domain took place */
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[1], 6);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[1], 11);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[1], 2);

    /* Test aggregation for the physical domain took place */
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[2], 6);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[2], 10);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[2], 2);

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        free_dev_ctx_tables(&dev_ctx[i].perf_table);
    }
    free_dev_ctx_tables(&perf_plugins_ctx.full_perf_table);

    perf_config.plugins_count = PERF_PLUGIN_IDX_COUNT;
}

/*
 * Where there are plugins, there can be logical domains.
 * In this case, the plugins-handler will wait until the last logical domain has
 * been read before calling the plugins.
 */
void utest_perf_plugins_handler_update_no_last_log_domain(void)
{
    struct fc_perf_update fc_update;
    fwk_id_t dep_dom_id;
    fwk_id_t dep_id_table[DVFS_ELEMENT_IDX_COUNT];

    memset(
        &test_dev_ctx[0], 0, sizeof(test_dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &test_dev_ctx[0];
    test_dev_ctx[0].log_dom_count = PERF_PLUGIN_IDX_COUNT + 1;
    perf_plugins_ctx.dep_id_table = &dep_id_table[0];

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        malloc_dev_ctx_tables(2, &test_dev_ctx[i].perf_table);
    }
    malloc_dev_ctx_tables(2, &perf_plugins_ctx.full_perf_table);

    perf_config.plugins_count = PERF_PLUGIN_IDX_COUNT;
    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);
    perf_plugins_ctx.dep_id_table[0] = dep_dom_id;

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

    fc_update = (struct fc_perf_update){
        .domain_id = dep_dom_id,
        .level = 0,
        .max_limit = 0,
        .min_limit = 0,
    };

    plugin_update_num_calls = 0;

    perf_plugins_handler_update(0, &fc_update);

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        free_dev_ctx_tables(&test_dev_ctx[i].perf_table);
    }
    free_dev_ctx_tables(&perf_plugins_ctx.full_perf_table);
}

static int perf_plugin_update_success_cback(
    struct perf_plugins_perf_update *data,
    int cmock_num_calls)
{
    fwk_id_t dep_dom_id =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_SCMI_PERF, DVFS_ELEMENT_IDX_0, 0);

    /* Test that the input from the caller is available to the plugin */
    TEST_ASSERT_EQUAL(data->domain_id.value, dep_dom_id.value);
    TEST_ASSERT_EQUAL(data->level[0], 500);
    TEST_ASSERT_EQUAL(data->max_limit[0], 1000);
    TEST_ASSERT_EQUAL(data->min_limit[0], 100);

    /* Test that the tables are correclty assigned */
    TEST_ASSERT_EQUAL(data->level, &test_dev_ctx[0].perf_table.level[1]);
    TEST_ASSERT_EQUAL(
        data->max_limit, &test_dev_ctx[0].perf_table.max_limit[1]);
    TEST_ASSERT_EQUAL(
        data->min_limit, &test_dev_ctx[0].perf_table.min_limit[1]);

    /* Pretend to override adjusted values */
    data->adj_max_limit[0] = 999;
    data->adj_min_limit[0] = 101;

    TEST_ASSERT_EQUAL(cmock_num_calls, 0);

    return FWK_SUCCESS;
}

/*
 * Here we test the `update` function for one single physical domain.
 * The caller provides input data and the plugin sets the adjusted values.
 */
void utest_perf_plugins_handler_update_phy_domain_success(void)
{
    struct fc_perf_update fc_update;
    fwk_id_t dep_dom_id;
    fwk_id_t dep_id_table[DVFS_ELEMENT_IDX_COUNT];

    memset(
        &test_dev_ctx[0], 0, sizeof(test_dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &test_dev_ctx[0];
    test_dev_ctx[0].log_dom_count = 1;
    perf_plugins_ctx.dep_id_table = &dep_id_table[0];

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        malloc_dev_ctx_tables(2, &test_dev_ctx[i].perf_table);

        test_dev_ctx[i].perf_table.domain_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, i);
    }
    malloc_dev_ctx_tables(2, &perf_plugins_ctx.full_perf_table);

    perf_config.plugins_count = PERF_PLUGIN_IDX_COUNT;
    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);
    perf_plugins_ctx.dep_id_table[0] = dep_dom_id;

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_0);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_0);
    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

    fwk_id_get_element_idx_ExpectAndReturn(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0), 0);

    fc_update = (struct fc_perf_update){
        .domain_id = dep_dom_id,
        .level = 500,
        .max_limit = 1000,
        .min_limit = 100,
    };

    perf_plugins_ctx.plugins_api_table[0] = &test_perf_plugins_api;
    plugin_update_Stub(perf_plugin_update_success_cback);

    perf_plugins_handler_update(0, &fc_update);

    /*
     * Test that the plugin's changes have been reflected in the internal
     * storage.
     */
    TEST_ASSERT_EQUAL(test_dev_ctx[0].lmax, 999);
    TEST_ASSERT_EQUAL(test_dev_ctx[0].lmin, 101);

    plugin_update_Stub(NULL);

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        free_dev_ctx_tables(&test_dev_ctx[i].perf_table);
    }
    free_dev_ctx_tables(&perf_plugins_ctx.full_perf_table);
}

/*
 * Below we test the `update` function for the first logical domain.
 * No plugins are called, but internal data is set to initial predefined values.
 */
void utest_perf_plugins_handler_update_log_domains_first_log_dom(void)
{
    struct perf_plugins_dev_ctx dev_ctx[DVFS_ELEMENT_IDX_COUNT];
    struct fc_perf_update fc_update;
    fwk_id_t dep_id_table[SCMI_PERF_ELEMENT_IDX_COUNT];
    fwk_id_t dep_dom_id;

    memset(&dev_ctx[0], 0, sizeof(dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &dev_ctx[0];

    perf_plugins_ctx.dep_id_table = &dep_id_table[0];

    malloc_dev_ctx_tables(2, &dev_ctx[DVFS_ELEMENT_IDX_0].perf_table);
    malloc_dev_ctx_tables(3, &dev_ctx[DVFS_ELEMENT_IDX_1].perf_table);
    malloc_dev_ctx_tables(
        DVFS_ELEMENT_IDX_COUNT, &perf_plugins_ctx.full_perf_table);

    dev_ctx[DVFS_ELEMENT_IDX_0].log_dom_count = 1;
    dev_ctx[DVFS_ELEMENT_IDX_1].log_dom_count = 2;

    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_0] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);
    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_1] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 0);
    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_2] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 1);

    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 0);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

    fc_update.level = 5;
    fc_update.max_limit = 10;
    fc_update.min_limit = 1;
    fc_update.domain_id = dep_dom_id;

    plugins_table[PERF_PLUGIN_IDX_0].dom_type = PERF_PLUGIN_DOM_TYPE_LOGICAL;

    perf_plugins_handler_update(SCMI_PERF_ELEMENT_IDX_1, &fc_update);

    /* Test internal storage for the logical domain took place */
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[0], 5);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[0], 10);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[0], 1);

    /* Test aggregation for the physical domain took place */
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[2], 5);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[2], 10);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[2], 1);

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        free_dev_ctx_tables(&dev_ctx[i].perf_table);
    }
    free_dev_ctx_tables(&perf_plugins_ctx.full_perf_table);

    plugins_table[PERF_PLUGIN_IDX_0].dom_type = PERF_PLUGIN_DOM_TYPE_PHYSICAL;
}

static int perf_plugin_update_success_log_doms_cback(
    struct perf_plugins_perf_update *data,
    int cmock_num_calls)
{
    TEST_ASSERT_EQUAL(data->level[0], 5);
    TEST_ASSERT_EQUAL(data->level[1], 6);
    TEST_ASSERT_EQUAL(data->max_limit[0], 10);
    TEST_ASSERT_EQUAL(data->max_limit[1], 9);
    TEST_ASSERT_EQUAL(data->min_limit[0], 3);
    TEST_ASSERT_EQUAL(data->min_limit[1], 4);

    return FWK_SUCCESS;
}

/*
 * Below we test the `update` function for the last logical domain.
 * The plugin is called, and aggregated values are verified in the callback.
 */
void utest_perf_plugins_handler_update_log_domains_last_log_dom(void)
{
    struct perf_plugins_dev_ctx dev_ctx[DVFS_ELEMENT_IDX_COUNT];
    struct fc_perf_update fc_update;
    fwk_id_t dep_id_table[SCMI_PERF_ELEMENT_IDX_COUNT];
    fwk_id_t dep_dom_id;
    struct perf_plugins_api *plugins_api_table[1];

    memset(&dev_ctx[0], 0, sizeof(dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &dev_ctx[0];

    perf_plugins_ctx.dep_id_table = &dep_id_table[0];

    malloc_dev_ctx_tables(2, &dev_ctx[DVFS_ELEMENT_IDX_0].perf_table);
    malloc_dev_ctx_tables(3, &dev_ctx[DVFS_ELEMENT_IDX_1].perf_table);
    malloc_dev_ctx_tables(
        DVFS_ELEMENT_IDX_COUNT, &perf_plugins_ctx.full_perf_table);

    dev_ctx[DVFS_ELEMENT_IDX_0].log_dom_count = 1;
    dev_ctx[DVFS_ELEMENT_IDX_1].log_dom_count = 2;

    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_0] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);
    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_1] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 0);
    perf_plugins_ctx.dep_id_table[SCMI_PERF_ELEMENT_IDX_2] =
        FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 1);

    dev_ctx[DVFS_ELEMENT_IDX_0].perf_table.domain_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0);
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.domain_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1);

    /* Prepare internal storage as the first logical domain had run already */
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[0] = 5;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[0] = 10;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[0] = 3;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.adj_max_limit[0] = 10;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.adj_min_limit[0] = 3;

    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[2] = 5;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[2] = 10;
    dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[2] = 3;

    dev_ctx[DVFS_ELEMENT_IDX_1].lmax = UINT32_MAX;
    dev_ctx[DVFS_ELEMENT_IDX_1].lmin = 0;

    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1, 1);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 1);
    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 1);

    /* assign_data */
    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, DVFS_ELEMENT_IDX_1);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 1);

    /* sync_level_limits */
    fwk_id_get_element_idx_ExpectAndReturn(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_1), 1);

    fc_update.level = 6;
    fc_update.max_limit = 9;
    fc_update.min_limit = 4;
    fc_update.domain_id = dep_dom_id;

    plugins_table[PERF_PLUGIN_IDX_0].dom_type = PERF_PLUGIN_DOM_TYPE_LOGICAL;

    plugins_api_table[0] = &test_perf_plugins_api;
    perf_plugins_ctx.plugins_api_table = &plugins_api_table[0];
    perf_config.plugins_count = PERF_PLUGIN_IDX_COUNT;

    plugin_update_Stub(perf_plugin_update_success_log_doms_cback);

    perf_plugins_handler_update(SCMI_PERF_ELEMENT_IDX_2, &fc_update);

    /* Test internal storage for the logical domain took place */
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[1], 6);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[1], 9);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[1], 4);

    /* Test aggregation for the physical domain took place */
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.level[2], 6);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.max_limit[2], 9);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].perf_table.min_limit[2], 4);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].max, 6);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].lmax, 9);
    TEST_ASSERT_EQUAL(dev_ctx[DVFS_ELEMENT_IDX_1].lmin, 4);

    plugin_update_Stub(NULL);

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        free_dev_ctx_tables(&dev_ctx[i].perf_table);
    }
    free_dev_ctx_tables(&perf_plugins_ctx.full_perf_table);

    plugins_table[PERF_PLUGIN_IDX_0].dom_type = PERF_PLUGIN_DOM_TYPE_PHYSICAL;
}

static int perf_plugin_update_type_full_cback(
    struct perf_plugins_perf_update *data,
    int cmock_num_calls)
{
    plugin_update_num_calls++;

    TEST_ASSERT_EQUAL(perf_plugins_ctx.full_perf_table.level, data->level);
    TEST_ASSERT_EQUAL(
        perf_plugins_ctx.full_perf_table.max_limit, data->max_limit);
    TEST_ASSERT_EQUAL(
        perf_plugins_ctx.full_perf_table.min_limit, data->min_limit);
    TEST_ASSERT_EQUAL(
        perf_plugins_ctx.full_perf_table.adj_max_limit, data->adj_max_limit);
    TEST_ASSERT_EQUAL(
        perf_plugins_ctx.full_perf_table.adj_min_limit, data->adj_min_limit);

    TEST_ASSERT_EQUAL(perf_plugins_ctx.full_perf_table.level[0], 500);
    TEST_ASSERT_EQUAL(perf_plugins_ctx.full_perf_table.level[1], 501);
    TEST_ASSERT_EQUAL(perf_plugins_ctx.full_perf_table.max_limit[0], 1000);
    TEST_ASSERT_EQUAL(perf_plugins_ctx.full_perf_table.max_limit[1], 1001);
    TEST_ASSERT_EQUAL(perf_plugins_ctx.full_perf_table.min_limit[0], 100);
    TEST_ASSERT_EQUAL(perf_plugins_ctx.full_perf_table.min_limit[1], 101);

    data->adj_max_limit[0] = 2000;
    data->adj_max_limit[1] = 2001;

    data->adj_min_limit[0] = 200;
    data->adj_min_limit[1] = 201;

    return FWK_SUCCESS;
}

/*
 * Below we test the `update` function for a plugin whose view if FULL.
 * This means that the plugin is able to see the whole spectrum of physical
 * domains.
 * Since references are passed to the plugins, we test if the pointers provided
 * to it are correct.
 */
void utest_perf_plugins_handler_update_type_full_domains(void)
{
    struct fc_perf_update fc_update;
    fwk_id_t dep_dom_id;
    unsigned int dep_dom_elem_idx;
    fwk_id_t dep_id_table[DVFS_ELEMENT_IDX_COUNT];
    struct perf_plugins_api *plugins_api_table[1];

    memset(
        &test_dev_ctx[0], 0, sizeof(test_dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &test_dev_ctx[0];
    perf_config.perf_doms_count = DVFS_ELEMENT_IDX_COUNT;

    perf_plugins_ctx.dep_id_table = &dep_id_table[0];

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        malloc_dev_ctx_tables(2, &test_dev_ctx[i].perf_table);

        test_dev_ctx[i].perf_table.domain_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, i);

        test_dev_ctx[i].log_dom_count = 1;
        perf_plugins_ctx.dep_id_table[i] =
            FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, i, 0);
    }

    malloc_dev_ctx_tables(
        DVFS_ELEMENT_IDX_COUNT, &perf_plugins_ctx.full_perf_table);

    perf_config.plugins_count = PERF_PLUGIN_IDX_COUNT;

    plugins_api_table[0] = &test_perf_plugins_api;
    perf_plugins_ctx.plugins_api_table = &plugins_api_table[0];

    plugins_table[PERF_PLUGIN_IDX_0].dom_type = PERF_PLUGIN_DOM_TYPE_FULL;

    plugin_update_Stub(perf_plugin_update_type_full_cback);
    plugin_update_num_calls = 0;

    for (size_t j = 0; j < DVFS_ELEMENT_IDX_COUNT; j++) {
        dep_dom_id = perf_plugins_ctx.dep_id_table[j];
        dep_dom_elem_idx = dep_dom_id.element.element_idx;

        if (j == 0) {
            fwk_id_get_element_idx_ExpectAndReturn(
                dep_dom_id, dep_dom_elem_idx);
            fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);
            fwk_id_get_element_idx_ExpectAndReturn(
                dep_dom_id, dep_dom_elem_idx);

            fwk_id_get_element_idx_ExpectAndReturn(
                dep_dom_id, dep_dom_elem_idx);
            fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

            fc_update = (struct fc_perf_update){
                .domain_id = dep_dom_id,
                .level = 500,
                .max_limit = 1000,
                .min_limit = 100,
            };
        }
        if (j == 1) {
            fwk_id_get_element_idx_ExpectAndReturn(
                dep_dom_id, dep_dom_elem_idx);
            fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);
            fwk_id_get_element_idx_ExpectAndReturn(
                dep_dom_id, dep_dom_elem_idx);

            fwk_id_get_element_idx_ExpectAndReturn(
                dep_dom_id, dep_dom_elem_idx);
            fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

            fwk_id_get_element_idx_ExpectAndReturn(
                dep_dom_id, dep_dom_elem_idx);

            fwk_id_get_element_idx_ExpectAndReturn(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 0), 0);
            fwk_id_get_element_idx_ExpectAndReturn(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, 1), 1);

            fc_update = (struct fc_perf_update){
                .domain_id = dep_dom_id,
                .level = 501,
                .max_limit = 1001,
                .min_limit = 101,
            };
        }

        perf_plugins_handler_update(j, &fc_update);
    }

    TEST_ASSERT_EQUAL(plugin_update_num_calls, 1);

    TEST_ASSERT_EQUAL(test_dev_ctx[0].lmax, 2000);
    TEST_ASSERT_EQUAL(test_dev_ctx[1].lmax, 2001);
    TEST_ASSERT_EQUAL(test_dev_ctx[0].lmin, 200);
    TEST_ASSERT_EQUAL(test_dev_ctx[1].lmin, 201);

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        free_dev_ctx_tables(&test_dev_ctx[i].perf_table);
    }
    free_dev_ctx_tables(&perf_plugins_ctx.full_perf_table);

    /* Restore */
    plugin_update_Stub(NULL);
    plugins_table[PERF_PLUGIN_IDX_0].dom_type = PERF_PLUGIN_DOM_TYPE_PHYSICAL;
}

/*
 * The 'get' function is supposed to run once all the information from different
 * sources (plugins, fast-channels) have been combined.
 * We test that values stored internally are correctly returned (physical
 * domains case).
 */
void utest_perf_plugins_handler_get_phy_domains(void)
{
    struct fc_perf_update fc_update;
    fwk_id_t dep_dom_id;
    fwk_id_t dep_id_table[DVFS_ELEMENT_IDX_COUNT];

    memset(
        &test_dev_ctx[0], 0, sizeof(test_dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &test_dev_ctx[0];
    test_dev_ctx[0].log_dom_count = 1;
    perf_plugins_ctx.dep_id_table = &dep_id_table[0];

    for (size_t i = 0; i < DVFS_ELEMENT_IDX_COUNT; i++) {
        test_dev_ctx[i].perf_table.domain_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, i);
    }

    perf_config.plugins_count = PERF_PLUGIN_IDX_COUNT;
    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);
    perf_plugins_ctx.dep_id_table[0] = dep_dom_id;

    test_dev_ctx[0].max = 500;
    test_dev_ctx[0].lmax = 1000;
    test_dev_ctx[0].lmin = 100;

    fc_update = (struct fc_perf_update){
        .domain_id = dep_dom_id,
        .level = 1,
        .max_limit = 2,
        .min_limit = 3,
        .adj_max_limit = 4,
        .adj_min_limit = 5,
    };

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, 0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, 0);

    perf_plugins_handler_get(0, &fc_update);

    TEST_ASSERT_EQUAL(fc_update.level, 500);
    TEST_ASSERT_EQUAL(fc_update.adj_max_limit, 1000);
    TEST_ASSERT_EQUAL(fc_update.adj_min_limit, 100);
}

/*
 * The 'get' function is supposed to run once all the information from different
 * sources (plugins, fast-channels) have been combined.
 * We test that values stored internally are correctly returned (logical
 * domains case).
 */
void utest_perf_plugins_handler_get_log_domains(void)
{
    struct fc_perf_update fc_update;
    fwk_id_t dep_dom_id;

    memset(
        &test_dev_ctx[0], 0, sizeof(test_dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &test_dev_ctx[0];
    test_dev_ctx[0].log_dom_count = 2;

    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);

    test_dev_ctx[0].max = 500;
    test_dev_ctx[0].lmax = 1000;
    test_dev_ctx[0].lmin = 100;

    fc_update = (struct fc_perf_update){
        .domain_id = dep_dom_id,
        .level = 1,
        .max_limit = 2,
        .min_limit = 3,
        .adj_max_limit = 4,
        .adj_min_limit = 5,
    };

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, 0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(dep_dom_id, 0);

    perf_plugins_handler_get(0, &fc_update);

    TEST_ASSERT_EQUAL(fc_update.level, 500);
    TEST_ASSERT_EQUAL(fc_update.adj_max_limit, 1000);
    TEST_ASSERT_EQUAL(fc_update.adj_min_limit, 100);
}

void utest_plugin_set_limits_null_data(void)
{
    int status;

    status = plugin_set_limits(NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

/*
 * Test check validity of limits: max_limit > min_limit
 */
void utest_plugin_set_limits_invalid_limits(void)
{
    int status;
    fwk_id_t dep_dom_id, dvfs_id;

    memset(
        &test_dev_ctx[0], 0, sizeof(test_dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &test_dev_ctx[0];
    test_dev_ctx[0].log_dom_count = 1;

    test_dev_ctx[0].max = 500;
    test_dev_ctx[0].lmax = 1000;
    test_dev_ctx[0].lmin = 100;

    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);
    dvfs_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0);

    struct plugin_limits_req req = (struct plugin_limits_req){
        .domain_id = dep_dom_id,
        .max_limit = 50,
        .min_limit = 5000,
    };

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    status = plugin_set_limits(&req);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

/*
 * Test 'set_limits' function when required limits are same as currently in use.
 */
void utest_plugin_set_limits_valid_limits(void)
{
    int status;
    fwk_id_t dep_dom_id, dvfs_id;

    memset(
        &test_dev_ctx[0], 0, sizeof(test_dev_ctx[0]) * DVFS_ELEMENT_IDX_COUNT);
    perf_plugins_ctx.dev_ctx = &test_dev_ctx[0];
    test_dev_ctx[0].log_dom_count = 1;

    test_dev_ctx[0].max = 500;
    test_dev_ctx[0].lmax = 1000;
    test_dev_ctx[0].lmin = 100;

    dep_dom_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0, 0);
    dvfs_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_0);

    struct plugin_limits_req req = (struct plugin_limits_req){
        .domain_id = dep_dom_id,
        .max_limit = 999,
        .min_limit = 101,
    };

    struct scmi_perf_domain_ctx domain0_ctx;
    scmi_perf_ctx.domain_ctx_table = &domain0_ctx;

    domain0_ctx.level_limits.maximum = 999;
    domain0_ctx.level_limits.minimum = 101;

    fwk_id_get_element_idx_ExpectAndReturn(dep_dom_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    status = plugin_set_limits(&req);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

#endif /* BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER */

int scmi_perf_test_main(void)
{
    UNITY_BEGIN();

#ifndef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
#    ifndef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    RUN_TEST(utest_scmi_perf_message_handler_bad_message_id);
    RUN_TEST(utest_scmi_perf_message_handler_not_found);
#    endif

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

#else
    RUN_TEST(utest_perf_plugins_handler_init_fail_no_dvfs_doms);
    RUN_TEST(utest_perf_plugins_handler_init_success);

    RUN_TEST(utest_perf_plugins_handler_bind_success_no_plugins);
    RUN_TEST(utest_perf_plugins_handler_bind_success);
    RUN_TEST(utest_perf_plugins_handler_bind_fail);

    RUN_TEST(utest_perf_plugins_handler_process_bind_request_success);

    RUN_TEST(utest_perf_plugins_handler_report_null_api);
    RUN_TEST(utest_perf_plugins_handler_report_plugin_success);
    RUN_TEST(utest_perf_plugins_handler_report_plugin_fail);

    RUN_TEST(utest_domain_aggregate_test_policy_level_up);
    RUN_TEST(utest_domain_aggregate_test_policy_level_down);
    RUN_TEST(utest_domain_aggregate_test_policy_max_limit_up);
    RUN_TEST(utest_domain_aggregate_test_policy_max_limit_down);
    RUN_TEST(utest_domain_aggregate_test_policy_min_limit_up);
    RUN_TEST(utest_domain_aggregate_test_policy_min_limit_down);

    RUN_TEST(utest_perf_plugins_handler_update_no_plugins_first_log_dom);
    RUN_TEST(utest_perf_plugins_handler_update_no_plugins_last_log_dom);
    RUN_TEST(utest_perf_plugins_handler_update_no_last_log_domain);

    RUN_TEST(utest_perf_plugins_handler_update_phy_domain_success);
    RUN_TEST(utest_perf_plugins_handler_update_log_domains_first_log_dom);
    RUN_TEST(utest_perf_plugins_handler_update_log_domains_last_log_dom);
    RUN_TEST(utest_perf_plugins_handler_update_type_full_domains);

    RUN_TEST(utest_perf_plugins_handler_get_phy_domains);
    RUN_TEST(utest_perf_plugins_handler_get_log_domains);

    RUN_TEST(utest_plugin_set_limits_null_data);
    RUN_TEST(utest_plugin_set_limits_invalid_limits);
    RUN_TEST(utest_plugin_set_limits_valid_limits);

    RUN_TEST(utest_perf_eval_performance_invalid_limits);
    RUN_TEST(utest_perf_eval_performance_unchanged_limits);
    RUN_TEST(utest_perf_eval_performance_unchanged_limits_level_up);
    RUN_TEST(utest_perf_eval_performance_unchanged_limits_level_down);

    RUN_TEST(utest_perf_eval_performance_new_limits);
    RUN_TEST(utest_perf_eval_performance_new_limits_level_up);
    RUN_TEST(utest_perf_eval_performance_new_limits_level_down);
#endif

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return scmi_perf_test_main();
}
#endif
