/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_mm.h>
#include <Mockfwk_string.h>

#include <mod_resource_perms.h>

#include UNIT_TEST_SRC

#define SCMI_FLAGS_ALLOWED MOD_RES_PERMS_ACCESS_DENIED
#define SCMI_FLAGS_DENIED  MOD_RES_PERMS_ACCESS_ALLOWED

#define CHECK_BIT(val, pos) (val | (1U << (MOD_RES_PERMS_RESOURCE_BIT(pos))))

void setUp(void)
{
}

void tearDown(void)
{
}

/*!
 * \brief Power Domain Protocol Testing.
 */
void utest_set_permissions_null_agent(void)
{
    struct mod_res_agent_permission agent_permissions;

    uint32_t agent_id = 0;
    uint32_t resource_id = 0;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    agent_permissions.scmi_pd_perms = NULL;
    resources_perms_ctx.agent_permissions = &agent_permissions;

    result = set_agent_resource_pd_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
}

void utest_set_permissions_resource_id_greater_than_pd_count(void)
{
    uint32_t agent_id = 0;
    uint32_t resource_id = 3;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    resources_perms_ctx.pd_count = 2;

    result = set_agent_resource_pd_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, result);
}

mod_res_perms_t backup_scmi_pd_perms[4];
mod_res_perms_t scmi_pd_perms[4] = { 16, 17, 18, 19 };
void utest_set_pd_permissions(void)
{
    uint32_t agent_id = 1;
    uint32_t resource_id = 2;
    resources_perms_ctx.pd_count = 3;

    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    struct mod_res_resource_perms_config config;

    int result;

    config.pd_cmd_count = 4;
    config.pd_resource_count = 1;

    resources_perms_ctx.config = &config;
    resources_perms_ctx.agent_count = 2;
    resources_perms_ctx.agent_permissions->scmi_pd_perms = scmi_pd_perms;
    resources_perms_backup.scmi_pd_perms = NULL;

    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            resources_perms_ctx.config->pd_cmd_count *
            resources_perms_ctx.config->pd_resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_pd_perms);
    fwk_str_memcpy_ExpectAnyArgs();

    result = set_agent_resource_pd_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_pd_perms[0], resource_id),
        resources_perms_ctx.agent_permissions->scmi_pd_perms[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_pd_perms[1], resource_id),
        resources_perms_ctx.agent_permissions->scmi_pd_perms[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_pd_perms[2], resource_id),
        resources_perms_ctx.agent_permissions->scmi_pd_perms[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_pd_perms[3], resource_id),
        resources_perms_ctx.agent_permissions->scmi_pd_perms[3]);
}

/*!
 * \brief Perf Protocol Testing.
 */
void utest_set_perf_permissions_null_agent(void)
{
    struct mod_res_agent_permission agent_permissions;

    uint32_t agent_id = 0;
    uint32_t resource_id = 0;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    agent_permissions.scmi_perf_perms = NULL;
    resources_perms_ctx.agent_permissions = &agent_permissions;

    result = set_agent_resource_perf_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
}

void utest_set_perf_permissions_resource_id_greater_than_perf_count(void)
{
    uint32_t agent_id = 0;
    uint32_t resource_id = 3;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    resources_perms_ctx.perf_count = 2;

    result = set_agent_resource_perf_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, result);
}

mod_res_perms_t backup_scmi_perf_perms[9];
mod_res_perms_t scmi_perf_perms[9] = { 16, 17, 18, 19, 24, 25, 26, 27, 32 };
void utest_set_perf_permissions(void)
{
    uint32_t agent_id = 1;
    uint32_t resource_id = 2;
    resources_perms_ctx.perf_count = 3;

    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    struct mod_res_resource_perms_config config;

    int result;

    config.perf_cmd_count = 9;
    config.perf_resource_count = 1;

    resources_perms_ctx.config = &config;
    resources_perms_ctx.agent_count = 1;
    resources_perms_ctx.agent_permissions->scmi_perf_perms = scmi_perf_perms;
    resources_perms_backup.scmi_perf_perms = NULL;

    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            resources_perms_ctx.config->perf_cmd_count *
            resources_perms_ctx.config->perf_resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_perf_perms);
    fwk_str_memcpy_ExpectAnyArgs();

    result = set_agent_resource_perf_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[0], resource_id),
        resources_perms_ctx.agent_permissions->scmi_perf_perms[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[1], resource_id),
        resources_perms_ctx.agent_permissions->scmi_perf_perms[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[2], resource_id),
        resources_perms_ctx.agent_permissions->scmi_perf_perms[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[3], resource_id),
        resources_perms_ctx.agent_permissions->scmi_perf_perms[3]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[4], resource_id),
        resources_perms_ctx.agent_permissions->scmi_perf_perms[4]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[5], resource_id),
        resources_perms_ctx.agent_permissions->scmi_perf_perms[5]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[6], resource_id),
        resources_perms_ctx.agent_permissions->scmi_perf_perms[6]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[7], resource_id),
        resources_perms_ctx.agent_permissions->scmi_perf_perms[7]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[8], resource_id),
        resources_perms_ctx.agent_permissions->scmi_perf_perms[8]);
}

/*!
 * \brief Clock Protocol Testing.
 */
void utest_set_clock_permissions_null_agent(void)
{
    struct mod_res_agent_permission agent_permissions;

    uint32_t agent_id = 0;
    uint32_t resource_id = 0;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    agent_permissions.scmi_clock_perms = NULL;
    resources_perms_ctx.agent_permissions = &agent_permissions;

    result = set_agent_resource_clock_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
}

void utest_set_clock_permissions_resource_id_greater_than_clock_count(void)
{
    uint32_t agent_id = 0;
    uint32_t resource_id = 3;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    resources_perms_ctx.clock_count = 2;

    result = set_agent_resource_clock_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, result);
}

mod_res_perms_t backup_scmi_clock_perms[5];
mod_res_perms_t scmi_clock_perms[5] = { 16, 17, 18, 19, 24 };
void utest_set_clock_permissions(void)
{
    uint32_t agent_id = 1;
    uint32_t resource_id = 2;
    resources_perms_ctx.clock_count = 3;

    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    struct mod_res_resource_perms_config config;

    int result;

    config.clock_cmd_count = 5;
    config.clock_resource_count = 1;

    resources_perms_ctx.config = &config;
    resources_perms_ctx.agent_count = 1;
    resources_perms_ctx.agent_permissions->scmi_clock_perms = scmi_clock_perms;
    resources_perms_backup.scmi_clock_perms = NULL;

    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            resources_perms_ctx.config->clock_cmd_count *
            resources_perms_ctx.config->clock_resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_clock_perms);
    fwk_str_memcpy_ExpectAnyArgs();

    result = set_agent_resource_clock_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[0], resource_id),
        resources_perms_ctx.agent_permissions->scmi_clock_perms[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[1], resource_id),
        resources_perms_ctx.agent_permissions->scmi_clock_perms[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[2], resource_id),
        resources_perms_ctx.agent_permissions->scmi_clock_perms[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[3], resource_id),
        resources_perms_ctx.agent_permissions->scmi_clock_perms[3]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[4], resource_id),
        resources_perms_ctx.agent_permissions->scmi_clock_perms[4]);
}

/*!
 * \brief Voltd Protocol Testing.
 */
void utest_set_voltd_permissions_null_agent(void)
{
    struct mod_res_agent_permission agent_permissions;

    uint32_t agent_id = 0;
    uint32_t resource_id = 0;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    agent_permissions.scmi_voltd_perms = NULL;
    resources_perms_ctx.agent_permissions = &agent_permissions;

    result = set_agent_resource_voltd_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
}

void utest_set_voltd_permissions_resource_id_greater_than_voltd_count(void)
{
    uint32_t agent_id = 0;
    uint32_t resource_id = 3;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    resources_perms_ctx.voltd_count = 2;

    result = set_agent_resource_voltd_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, result);
}

mod_res_perms_t backup_scmi_voltd_perms[6] = {};
mod_res_perms_t scmi_voltd_perms[6] = { 16, 17, 18, 19, 24, 25 };
void utest_set_voltd_permissions(void)
{
    uint32_t agent_id = 1;
    uint32_t resource_id = 2;
    resources_perms_ctx.voltd_count = 3;

    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    struct mod_res_resource_perms_config config;

    int result;

    config.voltd_cmd_count = 6;
    config.voltd_resource_count = 1;

    resources_perms_ctx.config = &config;
    resources_perms_ctx.agent_count = 1;
    resources_perms_ctx.agent_permissions->scmi_voltd_perms =
        &scmi_voltd_perms[0];
    resources_perms_backup.scmi_voltd_perms = NULL;

    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            resources_perms_ctx.config->voltd_cmd_count *
            resources_perms_ctx.config->voltd_resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_voltd_perms);
    fwk_str_memcpy_ExpectAnyArgs();

    result = set_agent_resource_voltd_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[0], resource_id),
        resources_perms_ctx.agent_permissions->scmi_voltd_perms[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[1], resource_id),
        resources_perms_ctx.agent_permissions->scmi_voltd_perms[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[2], resource_id),
        resources_perms_ctx.agent_permissions->scmi_voltd_perms[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[3], resource_id),
        resources_perms_ctx.agent_permissions->scmi_voltd_perms[3]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[4], resource_id),
        resources_perms_ctx.agent_permissions->scmi_voltd_perms[4]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[5], resource_id),
        resources_perms_ctx.agent_permissions->scmi_voltd_perms[5]);
}

/*!
 * \brief Sensor Protocol Testing.
 */
void utest_set_sensor_permissions_null_agent(void)
{
    struct mod_res_agent_permission agent_permissions;

    uint32_t agent_id = 0;
    uint32_t resource_id = 0;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    agent_permissions.scmi_sensor_perms = NULL;
    resources_perms_ctx.agent_permissions = &agent_permissions;

    result =
        set_agent_resource_sensor_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
}

void utest_set_sensor_permissions_resource_id_greater_than_sensor_count(void)
{
    uint32_t agent_id = 0;
    uint32_t resource_id = 3;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    resources_perms_ctx.sensor_count = 2;

    result =
        set_agent_resource_sensor_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, result);
}

mod_res_perms_t backup_scmi_sensor_perms[4];
mod_res_perms_t scmi_sensor_perms[4] = { 16, 17, 18, 19 };
void utest_set_sensor_permissions(void)
{
    uint32_t agent_id = 1;
    uint32_t resource_id = 2;
    resources_perms_ctx.sensor_count = 3;

    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    struct mod_res_resource_perms_config config;

    int result;

    config.sensor_cmd_count = 4;
    config.sensor_resource_count = 1;

    resources_perms_ctx.config = &config;
    resources_perms_ctx.agent_count = 2;
    resources_perms_ctx.agent_permissions->scmi_sensor_perms =
        scmi_sensor_perms;
    resources_perms_backup.scmi_sensor_perms = NULL;

    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            resources_perms_ctx.config->sensor_cmd_count *
            resources_perms_ctx.config->sensor_resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_sensor_perms);
    fwk_str_memcpy_ExpectAnyArgs();

    result =
        set_agent_resource_sensor_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_sensor_perms[0], resource_id),
        resources_perms_ctx.agent_permissions->scmi_sensor_perms[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_sensor_perms[1], resource_id),
        resources_perms_ctx.agent_permissions->scmi_sensor_perms[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_sensor_perms[2], resource_id),
        resources_perms_ctx.agent_permissions->scmi_sensor_perms[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_sensor_perms[3], resource_id),
        resources_perms_ctx.agent_permissions->scmi_sensor_perms[3]);
}

/*!
 * \brief Reset Domain Protocol Testing.
 */
#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN

void utest_set_reset_permissions_null_agent(void)
{
    struct mod_res_agent_permission agent_permissions;

    uint32_t agent_id = 0;
    uint32_t resource_id = 0;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    agent_permissions.scmi_reset_domain_perms = NULL;
    resources_perms_ctx.agent_permissions = &agent_permissions;

    result = set_agent_resource_reset_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
}

void utest_set_reset_permissions_resource_id_greater_than_reset_count(void)
{
    uint32_t agent_id = 0;
    uint32_t resource_id = 3;
    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    int result;

    resources_perms_ctx.reset_domain_count = 2;

    result = set_agent_resource_reset_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, result);
}

mod_res_perms_t backup_scmi_reset_domain_perms[3];
mod_res_perms_t scmi_reset_domain_perms[3] = { 16, 17, 18 };
void utest_set_reset_permissions(void)
{
    uint32_t agent_id = 1;
    uint32_t resource_id = 2;
    resources_perms_ctx.reset_domain_count = 3;

    enum mod_res_perms_permissions flags = SCMI_FLAGS_DENIED;

    struct mod_res_resource_perms_config config;

    int result;

    config.sensor_cmd_count = 3;
    config.sensor_resource_count = 1;

    resources_perms_ctx.config = &config;
    resources_perms_ctx.agent_count = 2;
    resources_perms_ctx.agent_permissions->scmi_reset_domain_perms =
        scmi_reset_domain_perms;
    resources_perms_backup.scmi_reset_domain_perms = NULL;

    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            resources_perms_ctx.config->reset_domain_cmd_count *
            resources_perms_ctx.config->reset_domain_resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_reset_domain_perms);
    fwk_str_memcpy_ExpectAnyArgs();

    result = set_agent_resource_reset_permissions(agent_id, resource_id, flags);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_reset_domain_perms[0], resource_id),
        resources_perms_ctx.agent_permissions->scmi_reset_domain_perms[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_reset_domain_perms[1], resource_id),
        resources_perms_ctx.agent_permissions->scmi_reset_domain_perms[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_reset_domain_perms[2], resource_id),
        resources_perms_ctx.agent_permissions->scmi_reset_domain_perms[2]);
}
#endif

int resource_perms_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_set_permissions_null_agent);
    RUN_TEST(utest_set_permissions_resource_id_greater_than_pd_count);
    RUN_TEST(utest_set_pd_permissions);

    RUN_TEST(utest_set_perf_permissions_null_agent);
    RUN_TEST(utest_set_perf_permissions_resource_id_greater_than_perf_count);
    RUN_TEST(utest_set_perf_permissions);

    RUN_TEST(utest_set_clock_permissions_null_agent);
    RUN_TEST(utest_set_clock_permissions_resource_id_greater_than_clock_count);
    RUN_TEST(utest_set_clock_permissions);

    RUN_TEST(utest_set_voltd_permissions_null_agent);
    RUN_TEST(utest_set_voltd_permissions_resource_id_greater_than_voltd_count);
    RUN_TEST(utest_set_voltd_permissions);

    RUN_TEST(utest_set_sensor_permissions_null_agent);
    RUN_TEST(
        utest_set_sensor_permissions_resource_id_greater_than_sensor_count);
    RUN_TEST(utest_set_sensor_permissions);

#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
    RUN_TEST(utest_set_reset_permissions_null_agent);
    RUN_TEST(utest_set_reset_permissions_resource_id_greater_than_reset_count);
    RUN_TEST(utest_set_reset_permissions);
#endif

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return resource_perms_test_main();
}
#endif
