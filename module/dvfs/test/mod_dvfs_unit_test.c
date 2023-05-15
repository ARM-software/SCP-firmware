/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
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

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC

void setUp(void)
{
}

void tearDown(void)
{
}

void utest_dvfs_count_opps(void)
{
    size_t expected_opps_elements = 1;
    size_t opps_count;

    struct mod_dvfs_opp opps[2] = {
        {
            .level = 1,
            .voltage = 2,
            .frequency = 3,
            .power = 4,
        },
        { 0 },
    };

    opps_count = count_opps(&opps[0]);

    TEST_ASSERT_EQUAL(expected_opps_elements, opps_count);
}

void utest_dvfs_get_domain_ctx(void)
{
    fwk_id_t dvfs_id = { 0 };
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_ctx *return_ctx;

    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    return_ctx = get_domain_ctx(dvfs_id);

    TEST_ASSERT_EQUAL(&dvfs_domain_ctx[0], return_ctx);
}

void utest_dvfs_get_domain_ctx_invalid_domain_id(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_ctx *return_ctx;

    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 2);

    return_ctx = get_domain_ctx(dvfs_id);

    TEST_ASSERT_EQUAL(NULL, return_ctx);
}

void utest_dvfs_get_opp_for_level_with_existing_level(void)
{
    struct mod_dvfs_domain_config config;
    struct mod_dvfs_opp opps;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx = {
        .opp_count = 1,
    };
    const struct mod_dvfs_opp *return_opp;

    uint32_t level = 50;

    opps.level = 50;
    config.opps = &opps;
    dvfs_domain_ctx.config = &config;

    return_opp = get_opp_for_level(&dvfs_domain_ctx, level);
    TEST_ASSERT_EQUAL(&opps, return_opp);
}

void utest_dvfs_get_opp_for_level_with_non_existing_level(void)
{
    struct mod_dvfs_domain_config config;
    struct mod_dvfs_opp opps;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx = {
        .opp_count = 1,
    };

    const struct mod_dvfs_opp *return_opp;

    uint32_t level = 30;

    opps.level = 50;
    config.opps = &opps;
    dvfs_domain_ctx.config = &config;

    return_opp = get_opp_for_level(&dvfs_domain_ctx, level);

    TEST_ASSERT_EQUAL(NULL, return_opp);
}

void utest_dvfs_get_opp_for_voltage_with_existing_voltage(void)
{
    struct mod_dvfs_domain_config config;
    struct mod_dvfs_opp opps;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx = {
        .opp_count = 1,
    };

    const struct mod_dvfs_opp *return_opp;

    uint32_t voltage = 50;

    opps.voltage = 50;
    config.opps = &opps;
    dvfs_domain_ctx.config = &config;

    return_opp = get_opp_for_voltage(&dvfs_domain_ctx, voltage);

    TEST_ASSERT_EQUAL(&opps, return_opp);
}

void utest_dvfs_get_opp_for_voltage_with_none_existing_voltage(void)
{
    struct mod_dvfs_domain_config config;
    struct mod_dvfs_opp opps;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx = {
        .opp_count = 1,
    };

    const struct mod_dvfs_opp *return_opp;

    uint32_t voltage = 30;

    opps.voltage = 50;
    config.opps = &opps;
    dvfs_domain_ctx.config = &config;

    return_opp = get_opp_for_voltage(&dvfs_domain_ctx, voltage);

    TEST_ASSERT_EQUAL(NULL, return_opp);
}

void utest_dvfs_cleanup_request(void)
{
    /* Initiate ctx with random values in random chosen fields */
    struct mod_dvfs_domain_ctx ctx = {
            .pending_request = {
                .new_opp.level = 20,
            },
            .request = {
                .new_opp.voltage = 30,
            },
            .state = DVFS_DOMAIN_GET_OPP,
        };

    dvfs_cleanup_request(&ctx);

    TEST_ASSERT_EQUAL(0, ctx.pending_request.new_opp.level);
    TEST_ASSERT_EQUAL(0, ctx.request.new_opp.voltage);
    TEST_ASSERT_EQUAL(DVFS_DOMAIN_STATE_IDLE, ctx.state);
}

void utest_dvfs_get_sustained_opp_invalid_domain_id(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_opp sustained_opp;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_config config;
    int return_sustained_opp_status;

    /* configured to call get_domain_id and return sustained_idx */
    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 2);

    /* set opp_count less than sustained_idx to return FWK_E_PARAM */
    dvfs_domain_ctx[0].opp_count = 0;

    dvfs_domain_ctx[0].config = &config;

    return_sustained_opp_status =
        dvfs_get_sustained_opp(dvfs_id, &sustained_opp);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_sustained_opp_status);
}

void utest_dvfs_get_sustained_opp_sustained_idx_greater_opp_count(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_opp sustained_opp;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_config config;
    int return_sustained_opp_status;

    /* configured to call get_domain_id and return sustained_idx */
    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    /* set opp_count less than sustained_idx to return FWK_E_PARAM */
    dvfs_domain_ctx[0].opp_count = 0;
    config.sustained_idx = 1;

    dvfs_domain_ctx[0].config = &config;

    return_sustained_opp_status =
        dvfs_get_sustained_opp(dvfs_id, &sustained_opp);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_sustained_opp_status);
}

void utest_dvfs_get_sustained_opp_valid_domain_and_sustained_idx(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_opp sustained_opp;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_config config;
    struct mod_dvfs_opp opps[2];
    int return_sustained_opp_status;

    /* configured to call get_domain_id and return sustained_idx */
    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    /* set opp_count less than sustained_idx to return FWK_E_PARAM */
    dvfs_domain_ctx[0].opp_count = 1;
    config.sustained_idx = 0;
    config.opps = &opps[0];

    opps[0].level = 11;
    opps[0].voltage = 22;
    opps[0].frequency = 33;
    opps[0].power = 44;

    dvfs_domain_ctx[0].config = &config;

    return_sustained_opp_status =
        dvfs_get_sustained_opp(dvfs_id, &sustained_opp);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, return_sustained_opp_status);

    TEST_ASSERT_EQUAL(sustained_opp.level, 11);
    TEST_ASSERT_EQUAL(sustained_opp.voltage, 22);
    TEST_ASSERT_EQUAL(sustained_opp.frequency, 33);
    TEST_ASSERT_EQUAL(sustained_opp.power, 44);
}

void utest_dvfs_get_nth_opp_null_opp_pointer(void)
{
    fwk_id_t dvfs_id;
    /* The index has no effect */
    size_t index = 9;
    int return_nth_opp;

    return_nth_opp = dvfs_get_nth_opp(dvfs_id, index, NULL);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_nth_opp);
}

void utest_dvfs_get_nth_opp_invalid_dvfs_id(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_opp sustained_opp;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_config config;
    int return_nth_opp;

    /* The index has no effect */
    size_t index = 4;

    /* configured to call get_domain_id and return sustained_idx */
    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 2);

    /* set opp_count less than sustained_idx to return FWK_E_PARAM */
    dvfs_domain_ctx[0].opp_count = 0;
    config.sustained_idx = 1;

    dvfs_domain_ctx[0].config = &config;

    return_nth_opp = dvfs_get_nth_opp(dvfs_id, index, &sustained_opp);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_nth_opp);
}

void utest_dvfs_get_nth_opp_index_greater_than_opp_count(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_opp sustained_opp;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_config config;
    struct mod_dvfs_opp opps[2];
    int return_nth_opp;

    size_t index = 1;

    /* configured to call get_domain_id and return sustained_idx */
    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    /* set opp_count less than sustained_idx to return FWK_E_PARAM */
    dvfs_domain_ctx[0].opp_count = 1;
    config.sustained_idx = 0;
    config.opps = &opps[0];

    dvfs_domain_ctx[0].config = &config;

    return_nth_opp = dvfs_get_nth_opp(dvfs_id, index, &sustained_opp);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_nth_opp);
}

void utest_dvfs_get_nth_opp(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_opp sustained_opp;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_config config;
    struct mod_dvfs_opp opps[2];
    int return_nth_opp;

    size_t index = 0;

    /* configured to call get_domain_id and return sustained_idx */
    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    /* set opp_count less than sustained_idx to return FWK_E_PARAM */
    dvfs_domain_ctx[0].opp_count = 1;
    config.sustained_idx = 0;
    config.opps = &opps[0];

    opps[0].level = 11;
    opps[0].voltage = 22;
    opps[0].frequency = 33;
    opps[0].power = 44;

    dvfs_domain_ctx[0].config = &config;

    return_nth_opp = dvfs_get_nth_opp(dvfs_id, index, &sustained_opp);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, return_nth_opp);

    TEST_ASSERT_EQUAL(sustained_opp.level, 11);
    TEST_ASSERT_EQUAL(sustained_opp.voltage, 22);
    TEST_ASSERT_EQUAL(sustained_opp.frequency, 33);
    TEST_ASSERT_EQUAL(sustained_opp.power, 44);
}

void utest_dvfs_get_level_id_invalid_dvfs_id(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    int return_level_id;

    uint32_t level = 1;
    size_t level_id = 2;

    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 2);

    return_level_id = dvfs_get_level_id(dvfs_id, level, &level_id);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_level_id);
}

void utest_dvfs_get_level_id_opp_level_matches_level(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_config config;
    struct mod_dvfs_opp opps[2];
    int return_level_id;

    uint32_t level = 1;
    size_t level_id = 2;

    /* configured to call get_domain_id and return sustained_idx */
    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    /* set opp_count less than sustained_idx to return FWK_E_PARAM */
    dvfs_domain_ctx[0].opp_count = 1;

    /* set the opp.level equal to the passed level to enforce the code
     * to return FWK_SUCCESS
     */
    opps[0].level = level;
    config.opps = &opps[0];

    dvfs_domain_ctx[0].config = &config;

    return_level_id = dvfs_get_level_id(dvfs_id, level, &level_id);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, return_level_id);
    TEST_ASSERT_EQUAL(0, level_id);
}

void utest_dvfs_get_level_id_level_not_found(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_config config;
    struct mod_dvfs_opp opps[2];
    int return_level_id;

    uint32_t level = 1;
    size_t level_id = 2;

    /* configured to call get_domain_id and return sustained_idx */
    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    /* set opp_count less than sustained_idx to return FWK_E_PARAM */
    dvfs_domain_ctx[0].opp_count = 1;

    /* set the opp.level equal to the passed level to enforce the code
     * to return FWK_SUCCESS
     */
    opps[0].level = 3;
    config.opps = &opps[0];

    dvfs_domain_ctx[0].config = &config;

    return_level_id = dvfs_get_level_id(dvfs_id, level, &level_id);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_level_id);
}

void utest_dvfs_get_opp_count_null_opp_count(void)
{
    fwk_id_t dvfs_id;
    int return_opp_count;

    return_opp_count = dvfs_get_opp_count(dvfs_id, NULL);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_opp_count);
}

void utest_dvfs_get_opp_count_invalid_dvfs_id(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    int return_opp_count;

    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;
    size_t opp_count;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 2);

    return_opp_count = dvfs_get_opp_count(dvfs_id, &opp_count);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_opp_count);
}

void utest_dvfs_get_opp_count(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    size_t opp_count = 1;
    int return_opp_count;

    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    dvfs_domain_ctx[0].opp_count = 3;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    return_opp_count = dvfs_get_opp_count(dvfs_id, &opp_count);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, return_opp_count);
    TEST_ASSERT_EQUAL(3, opp_count);
}

void utest_dvfs_get_latency_null_latency(void)
{
    fwk_id_t dvfs_id;
    int return_latency;

    return_latency = dvfs_get_latency(dvfs_id, NULL);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_latency);
}

void utest_dvfs_get_latency_invalid_dvfs_id(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    int return_latency;

    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;
    uint16_t latency;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 2);

    return_latency = dvfs_get_latency(dvfs_id, &latency);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, return_latency);
}

void utest_dvfs_get_latency(void)
{
    fwk_id_t dvfs_id;
    struct mod_dvfs_domain_ctx dvfs_domain_ctx[1];
    struct mod_dvfs_domain_config config;
    int return_latency;
    uint16_t latency = 6;

    dvfs_ctx.dvfs_domain_element_count = 1;

    dvfs_ctx.domain_ctx = &dvfs_domain_ctx;

    config.latency = 3;
    dvfs_domain_ctx[0].config = &config;

    fwk_id_get_element_idx_ExpectAndReturn(dvfs_id, 0);

    return_latency = dvfs_get_latency(dvfs_id, &latency);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, return_latency);
    TEST_ASSERT_EQUAL(3, latency);
}

int dvfs_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_dvfs_count_opps);

    RUN_TEST(utest_dvfs_get_domain_ctx);
    RUN_TEST(utest_dvfs_get_domain_ctx_invalid_domain_id);

    RUN_TEST(utest_dvfs_get_opp_for_level_with_existing_level);
    RUN_TEST(utest_dvfs_get_opp_for_level_with_non_existing_level);

    RUN_TEST(utest_dvfs_get_opp_for_voltage_with_existing_voltage);
    RUN_TEST(utest_dvfs_get_opp_for_voltage_with_none_existing_voltage);

    RUN_TEST(utest_dvfs_cleanup_request);

    RUN_TEST(utest_dvfs_get_sustained_opp_invalid_domain_id);
    RUN_TEST(utest_dvfs_get_sustained_opp_valid_domain_and_sustained_idx);
    RUN_TEST(utest_dvfs_get_sustained_opp_sustained_idx_greater_opp_count);

    RUN_TEST(utest_dvfs_get_nth_opp_null_opp_pointer);
    RUN_TEST(utest_dvfs_get_nth_opp);
    RUN_TEST(utest_dvfs_get_nth_opp_invalid_dvfs_id);
    RUN_TEST(utest_dvfs_get_nth_opp_index_greater_than_opp_count);

    RUN_TEST(utest_dvfs_get_level_id_invalid_dvfs_id);
    RUN_TEST(utest_dvfs_get_level_id_opp_level_matches_level);
    RUN_TEST(utest_dvfs_get_level_id_level_not_found);

    RUN_TEST(utest_dvfs_get_opp_count_null_opp_count);
    RUN_TEST(utest_dvfs_get_opp_count_invalid_dvfs_id);
    RUN_TEST(utest_dvfs_get_opp_count);

    RUN_TEST(utest_dvfs_get_latency_null_latency);
    RUN_TEST(utest_dvfs_get_latency_invalid_dvfs_id);
    RUN_TEST(utest_dvfs_get_latency);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return dvfs_test_main();
}
#endif
