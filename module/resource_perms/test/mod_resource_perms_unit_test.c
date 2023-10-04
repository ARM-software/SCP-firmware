
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

#define CLOCK_RESOURCE_CMDS        5
#define PD_RESOURCE_CMDS           4
#define PERF_RESOURCE_CMDS         9
#define RESET_DOMAIN_RESOURCE_CMDS 3
#define VOLTD_RESOURCE_CMDS        8
#define SENSOR_RESOURCE_CMDS       4

#define RESOURCE_CMD0 16
#define RESOURCE_CMD1 17
#define RESOURCE_CMD2 18
#define RESOURCE_CMD3 19
#define RESOURCE_CMD4 24
#define RESOURCE_CMD5 25
#define RESOURCE_CMD6 26
#define RESOURCE_CMD7 27
#define RESOURCE_CMD8 32

mod_res_perms_t backup_scmi_pd_perms[PD_RESOURCE_CMDS];
mod_res_perms_t scmi_pd_perms[PD_RESOURCE_CMDS] = { RESOURCE_CMD0,
                                                    RESOURCE_CMD1,
                                                    RESOURCE_CMD2,
                                                    RESOURCE_CMD3 };

mod_res_perms_t backup_scmi_perf_perms[PERF_RESOURCE_CMDS];
mod_res_perms_t scmi_perf_perms[PERF_RESOURCE_CMDS] = {
    RESOURCE_CMD0, RESOURCE_CMD1, RESOURCE_CMD2, RESOURCE_CMD3, RESOURCE_CMD4,
    RESOURCE_CMD5, RESOURCE_CMD6, RESOURCE_CMD7, RESOURCE_CMD8
};

mod_res_perms_t backup_scmi_clock_perms[CLOCK_RESOURCE_CMDS];
mod_res_perms_t scmi_clock_perms[CLOCK_RESOURCE_CMDS] = { RESOURCE_CMD0,
                                                          RESOURCE_CMD1,
                                                          RESOURCE_CMD2,
                                                          RESOURCE_CMD3,
                                                          RESOURCE_CMD4 };

mod_res_perms_t backup_scmi_voltd_perms[VOLTD_RESOURCE_CMDS] = {};
mod_res_perms_t scmi_voltd_perms[VOLTD_RESOURCE_CMDS] = {
    RESOURCE_CMD0, RESOURCE_CMD1, RESOURCE_CMD2, RESOURCE_CMD3,
    RESOURCE_CMD4, RESOURCE_CMD5, RESOURCE_CMD6, RESOURCE_CMD7
};

mod_res_perms_t backup_scmi_sensor_perms[SENSOR_RESOURCE_CMDS];
mod_res_perms_t scmi_sensor_perms[SENSOR_RESOURCE_CMDS] = { RESOURCE_CMD0,
                                                            RESOURCE_CMD1,
                                                            RESOURCE_CMD2,
                                                            RESOURCE_CMD3 };

#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
mod_res_perms_t backup_scmi_reset_domain_perms[RESET_DOMAIN_RESOURCE_CMDS];
mod_res_perms_t scmi_reset_domain_perms[RESET_DOMAIN_RESOURCE_CMDS] = {
    RESOURCE_CMD0,
    RESOURCE_CMD1,
    RESOURCE_CMD2
};
#endif

void setUp(void)
{
}
void tearDown(void)
{
}

void utest_set_permissions_null_agent(void)
{
    struct agent_resource_permissions_params permissions = {};
    permissions.resource_permission = NULL;
    int result = set_agent_resource_permissions(&permissions);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
}
void utest_set_permissions_resource_id_greater_than_count(void)
{
    struct agent_resource_permissions_params permissions = {};
    struct protocol_permissions_counters config;
    mod_res_perms_t resource_permission[4];
    permissions.agent_id = 0;
    permissions.resource_id = 3;
    int result;
    config.count = 2;
    permissions.counters = &config;
    permissions.resource_permission = resource_permission;
    result = set_agent_resource_permissions(&permissions);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, result);
}
/*!
 * \brief power domain Protocol Testing.
 */
void utest_set_agent_resource_pd_permissions(void)
{
    struct agent_resource_permissions_params pd_permissions = {};
    struct protocol_permissions_counters config;
    int result;
    pd_permissions.counters = &config;
    pd_permissions.resource_permission = scmi_pd_perms;
    pd_permissions.resource_permission_backup = NULL;
    pd_permissions.agent_id = 1;
    pd_permissions.resource_id = 2;
    pd_permissions.protocol_id = MOD_SCMI_PROTOCOL_ID_POWER_DOMAIN;
    pd_permissions.flags = SCMI_FLAGS_DENIED;
    pd_permissions.start_message_idx = MOD_SCMI_PD_POWER_DOMAIN_ATTRIBUTES;
    pd_permissions.end_message_idx = MOD_SCMI_PD_POWER_STATE_NOTIFY;
    config.count = 3;
    config.resource_count = 1;
    config.cmd_count = PD_RESOURCE_CMDS;
    resources_perms_ctx.agent_count = 1;
    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count * pd_permissions.counters->cmd_count *
            pd_permissions.counters->resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_pd_perms);
    fwk_str_memcpy_ExpectAnyArgs();
    result = set_agent_resource_permissions(&pd_permissions);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_pd_perms[0], pd_permissions.resource_id),
        pd_permissions.resource_permission[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_pd_perms[1], pd_permissions.resource_id),
        pd_permissions.resource_permission[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_pd_perms[2], pd_permissions.resource_id),
        pd_permissions.resource_permission[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_pd_perms[3], pd_permissions.resource_id),
        pd_permissions.resource_permission[3]);
}
/*!
 * \brief Perf Protocol Testing.
 */
void utest_set_agent_resource_perf_permissions(void)
{
    struct agent_resource_permissions_params perf_permissions = {};
    struct protocol_permissions_counters config;
    int result;
    perf_permissions.counters = &config;
    perf_permissions.resource_permission = scmi_perf_perms;
    perf_permissions.resource_permission_backup = NULL;
    perf_permissions.agent_id = 1;
    perf_permissions.resource_id = 2;
    perf_permissions.protocol_id = MOD_SCMI_PROTOCOL_ID_PERF;
    perf_permissions.flags = SCMI_FLAGS_DENIED;
    perf_permissions.start_message_idx = MOD_SCMI_PERF_DOMAIN_ATTRIBUTES;
    perf_permissions.end_message_idx = MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL;
    config.count = 3;
    config.resource_count = 1;
    config.cmd_count = PERF_RESOURCE_CMDS;
    resources_perms_ctx.agent_count = 2;
    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count * perf_permissions.counters->cmd_count *
            perf_permissions.counters->resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_perf_perms);
    fwk_str_memcpy_ExpectAnyArgs();
    result = set_agent_resource_permissions(&perf_permissions);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[0], perf_permissions.resource_id),
        perf_permissions.resource_permission[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[1], perf_permissions.resource_id),
        perf_permissions.resource_permission[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[2], perf_permissions.resource_id),
        perf_permissions.resource_permission[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[3], perf_permissions.resource_id),
        perf_permissions.resource_permission[3]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[4], perf_permissions.resource_id),
        perf_permissions.resource_permission[4]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[5], perf_permissions.resource_id),
        perf_permissions.resource_permission[5]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[6], perf_permissions.resource_id),
        perf_permissions.resource_permission[6]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[7], perf_permissions.resource_id),
        perf_permissions.resource_permission[7]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_perf_perms[8], perf_permissions.resource_id),
        perf_permissions.resource_permission[8]);
}
/*!
 * \brief Clock Protocol Testing.
 */
void utest_set_agent_resource_clock_permissions(void)
{
    struct agent_resource_permissions_params clock_permissions = {};
    struct protocol_permissions_counters config;
    int result;
    clock_permissions.counters = &config;
    clock_permissions.resource_permission = scmi_clock_perms;
    clock_permissions.resource_permission_backup = NULL;
    clock_permissions.agent_id = 1;
    clock_permissions.resource_id = 2;
    clock_permissions.protocol_id = MOD_SCMI_PROTOCOL_ID_CLOCK;
    clock_permissions.flags = MOD_RES_PERMS_ACCESS_ALLOWED;
    clock_permissions.start_message_idx = MOD_SCMI_CLOCK_ATTRIBUTES;
    clock_permissions.end_message_idx = MOD_SCMI_CLOCK_CONFIG_SET;
    config.count = 3;
    config.resource_count = 1;
    config.cmd_count = CLOCK_RESOURCE_CMDS;
    resources_perms_ctx.agent_count = 2;
    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            clock_permissions.counters->cmd_count *
            clock_permissions.counters->resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_clock_perms);
    fwk_str_memcpy_ExpectAnyArgs();
    result = set_agent_resource_permissions(&clock_permissions);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[0], clock_permissions.resource_id),
        clock_permissions.resource_permission[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[1], clock_permissions.resource_id),
        clock_permissions.resource_permission[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[2], clock_permissions.resource_id),
        clock_permissions.resource_permission[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[3], clock_permissions.resource_id),
        clock_permissions.resource_permission[3]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_clock_perms[4], clock_permissions.resource_id),
        clock_permissions.resource_permission[4]);
}
/*!
 * \brief Voltd Protocol Testing.
 */
void utest_set_agent_resource_voltd_permissions(void)
{
    struct agent_resource_permissions_params voltd_permissions = {};
    struct protocol_permissions_counters config;
    int result;
    voltd_permissions.counters = &config;
    voltd_permissions.resource_permission = scmi_voltd_perms;
    voltd_permissions.resource_permission_backup = NULL;
    voltd_permissions.agent_id = 1;
    voltd_permissions.resource_id = 2;
    voltd_permissions.protocol_id = MOD_SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN;
    voltd_permissions.flags = MOD_RES_PERMS_ACCESS_ALLOWED;
    voltd_permissions.start_message_idx = MOD_SCMI_VOLTD_DOMAIN_ATTRIBUTES;
    voltd_permissions.end_message_idx = MOD_SCMI_VOLTD_LEVEL_GET;
    config.count = 3;
    config.resource_count = 1;
    config.cmd_count = VOLTD_RESOURCE_CMDS;
    resources_perms_ctx.agent_count = 2;
    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            voltd_permissions.counters->cmd_count *
            voltd_permissions.counters->resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_voltd_perms);
    fwk_str_memcpy_ExpectAnyArgs();
    result = set_agent_resource_permissions(&voltd_permissions);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[0], voltd_permissions.resource_id),
        voltd_permissions.resource_permission[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[1], voltd_permissions.resource_id),
        voltd_permissions.resource_permission[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[2], voltd_permissions.resource_id),
        voltd_permissions.resource_permission[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[3], voltd_permissions.resource_id),
        voltd_permissions.resource_permission[3]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[4], voltd_permissions.resource_id),
        voltd_permissions.resource_permission[4]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_voltd_perms[5], voltd_permissions.resource_id),
        voltd_permissions.resource_permission[5]);
}
/*!
 * \brief Sensor Protocol Testing.
 */
void utest_set_agent_resource_sensor_permissions(void)
{
    struct agent_resource_permissions_params sensor_permissions = {};
    struct protocol_permissions_counters config;
    int result;
    sensor_permissions.counters = &config;
    sensor_permissions.resource_permission = scmi_sensor_perms;
    sensor_permissions.resource_permission_backup = NULL;
    sensor_permissions.agent_id = 1;
    sensor_permissions.resource_id = 2;
    sensor_permissions.protocol_id = MOD_SCMI_PROTOCOL_ID_SENSOR;
    sensor_permissions.flags = MOD_RES_PERMS_ACCESS_ALLOWED;
    sensor_permissions.start_message_idx = MOD_SCMI_SENSOR_DESCRIPTION_GET;
    sensor_permissions.end_message_idx = MOD_SCMI_SENSOR_READING_GET;
    config.count = 3;
    config.resource_count = 1;
    config.cmd_count = SENSOR_RESOURCE_CMDS;
    resources_perms_ctx.agent_count = 2;
    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            sensor_permissions.counters->cmd_count *
            sensor_permissions.counters->resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_sensor_perms);
    fwk_str_memcpy_ExpectAnyArgs();
    result = set_agent_resource_permissions(&sensor_permissions);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_sensor_perms[0], sensor_permissions.resource_id),
        sensor_permissions.resource_permission[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_sensor_perms[1], sensor_permissions.resource_id),
        sensor_permissions.resource_permission[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_sensor_perms[2], sensor_permissions.resource_id),
        sensor_permissions.resource_permission[2]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_sensor_perms[3], sensor_permissions.resource_id),
        sensor_permissions.resource_permission[3]);
}
/*!
 * \brief Reset Domain Protocol Testing.
 */
#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
void utest_set_agent_resource_reset_permissions(void)
{
    struct agent_resource_permissions_params reset_permissions = {};
    struct protocol_permissions_counters config;
    int result;
    reset_permissions.counters = &config;
    reset_permissions.resource_permission = scmi_reset_domain_perms;
    reset_permissions.resource_permission_backup = NULL;
    reset_permissions.agent_id = 1;
    reset_permissions.resource_id = 2;
    reset_permissions.protocol_id = MOD_SCMI_PROTOCOL_ID_RESET_DOMAIN;
    reset_permissions.flags = MOD_RES_PERMS_ACCESS_ALLOWED;
    reset_permissions.start_message_idx = MOD_SCMI_RESET_DOMAIN_ATTRIBUTES;
    reset_permissions.end_message_idx = MOD_SCMI_RESET_NOTIFY;
    config.count = 3;
    config.resource_count = 1;
    config.cmd_count = RESET_DOMAIN_RESOURCE_CMDS;
    resources_perms_ctx.agent_count = 2;
    fwk_mm_alloc_ExpectAndReturn(
        resources_perms_ctx.agent_count *
            reset_permissions.counters->cmd_count *
            reset_permissions.counters->resource_count,
        sizeof(mod_res_perms_t),
        backup_scmi_reset_domain_perms);
    fwk_str_memcpy_ExpectAnyArgs();
    result = set_agent_resource_permissions(&reset_permissions);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, result);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_reset_domain_perms[0], reset_permissions.resource_id),
        reset_permissions.resource_permission[0]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_reset_domain_perms[1], reset_permissions.resource_id),
        reset_permissions.resource_permission[1]);
    TEST_ASSERT_EQUAL(
        CHECK_BIT(scmi_reset_domain_perms[2], reset_permissions.resource_id),
        reset_permissions.resource_permission[2]);
}
#endif
int resource_perms_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(utest_set_permissions_null_agent);
    RUN_TEST(utest_set_permissions_resource_id_greater_than_count);
    RUN_TEST(utest_set_agent_resource_pd_permissions);
    RUN_TEST(utest_set_agent_resource_perf_permissions);
    RUN_TEST(utest_set_agent_resource_clock_permissions);
    RUN_TEST(utest_set_agent_resource_voltd_permissions);
    RUN_TEST(utest_set_agent_resource_sensor_permissions);
#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
    RUN_TEST(utest_set_agent_resource_reset_permissions);
#endif
    return UNITY_END();
}
#if !defined(TEST_ON_TARGET)
int main(void)
{
    return resource_perms_test_main();
}
#endif
