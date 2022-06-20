/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_module.h>

#include <internal/Mockfwk_core_internal.h>

#include <mod_xr77128.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>
#include <fwk_string.h>

#include UNIT_TEST_SRC

#include "config_xr77128.h"

static struct xr77128_dev_ctx dev_ctx[FWK_ELEMENT_IDX_COUNT];
static struct xr77128_channel_ctx chan_ctx[FWK_CHANNEL_IDX_COUNT];

void setUp(void)
{
    int dev_idx, channel_idx;
    struct xr77128_dev_ctx *ctx;
    struct xr77128_channel_ctx *channel_ctx;

    module_ctx.dev_ctx_table = dev_ctx;
    module_ctx.device_count = FWK_ELEMENT_IDX_COUNT;
    for (dev_idx = 0; dev_idx < FWK_ELEMENT_IDX_COUNT; dev_idx++) {
        ctx = &dev_ctx[dev_idx];
        ctx->channel_ctx_table = chan_ctx;
        ctx->config = xr77128_element_table[dev_idx].data;

        for (channel_idx = 0; channel_idx < FWK_CHANNEL_IDX_COUNT;
             channel_idx++) {
            channel_ctx = &chan_ctx[channel_idx];
            channel_ctx->config = &channel_config[channel_idx];
            channel_ctx->is_channel_enabled = channel_ctx->config->enabled;
        }
    }
}

void tearDown(void)
{
}

void test_xr77128_init_fail(void)
{
    int status;

    status = mod_xr77128_init(fwk_module_id_xr77128, 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}

void test_xr77128_init_success(void)
{
    int status;

    status =
        mod_xr77128_init(fwk_module_id_xr77128, FWK_ELEMENT_IDX_COUNT, NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_xr77128_device_init_fail(void)
{
    int status;

    status = mod_xr77128_element_init(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_COUNT, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}

void test_xr77128_device_init_success(void)
{
    int status;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_build_sub_element_id_IgnoreAndReturn(fwk_sub_element_id_dummy);

    status = mod_xr77128_element_init(
        fwk_element_id_dummy,
        FWK_ELEMENT_IDX_COUNT,
        xr77128_element_table[FWK_ELEMENT_IDX_DUMMY].data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_xr77128_process_bind_request_fail(void)
{
    int status;
    const void *api;

    fwk_id_t api_id =
        FWK_ID_API_INIT(FWK_MODULE_IDX_XR77128, MOD_XR77128_API_IDX_PSU);

    fwk_id_is_type_ExpectAnyArgsAndReturn(false);

    status = mod_xr77128_process_bind_request(
        fwk_module_id_psu, fwk_module_id_xr77128, api_id, &api);
    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);
}

void test_xr77128_process_bind_request_success(void)
{
    int status;
    const void *api;

    fwk_id_t api_id =
        FWK_ID_API_INIT(FWK_MODULE_IDX_XR77128, MOD_XR77128_API_IDX_PSU);

    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    fwk_id_is_equal_IgnoreAndReturn(true);

    status = mod_xr77128_process_bind_request(
        fwk_module_id_psu, fwk_module_id_xr77128, api_id, &api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_xr77128_bind_round_1_success(void)
{
    unsigned int round = 1;
    int status;

    status = mod_xr77128_bind(fwk_element_id_dummy, round);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_xr77128_bind_element_fail(void)
{
    unsigned int round = 0;
    int status;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_E_PARAM);

    status = mod_xr77128_bind(fwk_element_id_dummy, round);
    TEST_ASSERT_EQUAL(status, FWK_E_HANDLER);
}

void test_xr77128_bind_element_success(void)
{
    unsigned int round = 0;
    int status;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = mod_xr77128_bind(fwk_element_id_dummy, round);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_xr77128_get_enabled_fail(void)
{
    int status;

    status = xr77128_get_enabled(fwk_element_id_dummy, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_xr77128_get_enabled_success(void)
{
    int status;
    bool enabled;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);

    status = xr77128_get_enabled(fwk_element_id_dummy, &enabled);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_TRUE(enabled);
}

void test_xr77128_set_enabled_event_fail(void)
{
    int status;
    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);
    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_E_PARAM);

    status = xr77128_set_enabled(fwk_element_id_dummy, 1);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_xr77128_set_enabled_request_fail(void)
{
    int status;
    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);
    chan_ctx->psu_request = XR77128_PSU_REQUEST_WAIT_FOR_IDLE;

    status = xr77128_set_enabled(fwk_element_id_dummy, 1);
    TEST_ASSERT_EQUAL(status, FWK_E_BUSY);
}

void test_xr77128_set_enabled_success(void)
{
    int status;
    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);
    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    chan_ctx->psu_request = XR77128_PSU_REQUEST_IDLE;

    status = xr77128_set_enabled(fwk_element_id_dummy, 1);
    TEST_ASSERT_EQUAL(status, FWK_PENDING);
    TEST_ASSERT_EQUAL(chan_ctx->psu_request, XR77128_PSU_REQUEST_SET_ENABLED);
}

void test_xr77128_set_enabled_busy_success(void)
{
    int status;
    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);
    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    chan_ctx->psu_request = XR77128_PSU_REQUEST_IDLE;
    dev_ctx->device_busy = 1;

    status = xr77128_set_enabled(fwk_element_id_dummy, 1);
    TEST_ASSERT_EQUAL(status, FWK_PENDING);
    TEST_ASSERT_EQUAL(
        chan_ctx->saved_psu_request, XR77128_PSU_REQUEST_SET_ENABLED);
    TEST_ASSERT_EQUAL(chan_ctx->psu_request, XR77128_PSU_REQUEST_WAIT_FOR_IDLE);
}

void test_xr77128_get_voltage_fail(void)
{
    int status;

    status = xr77128_get_voltage(fwk_element_id_dummy, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_xr77128_get_voltage_request_fail(void)
{
    int status;
    uint32_t voltage;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);

    status = xr77128_get_voltage(fwk_element_id_dummy, &voltage);
    TEST_ASSERT_EQUAL(status, FWK_E_BUSY);
}

void test_xr77128_get_voltage_event_fail(void)
{
    int status;
    uint32_t voltage;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);
    __fwk_put_event_light_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    chan_ctx->psu_request = XR77128_PSU_REQUEST_IDLE;

    status = xr77128_get_voltage(fwk_element_id_dummy, &voltage);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_xr77128_get_voltage_success(void)
{
    int status;
    uint32_t voltage;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);
    __fwk_put_event_light_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    dev_ctx->device_busy = 0;

    status = xr77128_get_voltage(fwk_element_id_dummy, &voltage);
    TEST_ASSERT_EQUAL(status, FWK_PENDING);
    TEST_ASSERT_EQUAL(chan_ctx->psu_request, XR77128_PSU_REQUEST_GET_VOLTAGE);
}

void test_xr77128_set_voltage_range_fail(void)
{
    int status;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);

    status = xr77128_set_voltage(fwk_element_id_dummy, 1200);
    TEST_ASSERT_EQUAL(status, FWK_E_RANGE);
}

void test_xr77128_set_voltage_request_fail(void)
{
    int status;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);
    chan_ctx->psu_request = XR77128_PSU_REQUEST_WAIT_FOR_IDLE;

    status = xr77128_set_voltage(fwk_element_id_dummy, 1000);
    TEST_ASSERT_EQUAL(status, FWK_E_BUSY);
}

void test_xr77128_set_voltage_event_fail(void)
{
    int status;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);
    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    chan_ctx->psu_request = XR77128_PSU_REQUEST_IDLE;

    status = xr77128_set_voltage(fwk_element_id_dummy, 860);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_xr77128_set_voltage_success(void)
{
    int status;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_module_is_valid_sub_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(FWK_CHANNEL_IDX_DUMMY);
    __fwk_put_event_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fwk_id_get_element_idx_ExpectAndReturn(
        fwk_element_id_dummy, FWK_ELEMENT_IDX_DUMMY);
    dev_ctx->device_busy = 0;

    status = xr77128_set_voltage(fwk_element_id_dummy, 900);
    TEST_ASSERT_EQUAL(status, FWK_PENDING);
    TEST_ASSERT_EQUAL(chan_ctx->psu_request, XR77128_PSU_REQUEST_SET_VOLTAGE);
}

int xr77128_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_xr77128_init_fail);
    RUN_TEST(test_xr77128_init_success);
    RUN_TEST(test_xr77128_device_init_fail);
    RUN_TEST(test_xr77128_device_init_success);
    RUN_TEST(test_xr77128_process_bind_request_fail);
    RUN_TEST(test_xr77128_process_bind_request_success);
    RUN_TEST(test_xr77128_bind_round_1_success);
    RUN_TEST(test_xr77128_bind_element_fail);
    RUN_TEST(test_xr77128_bind_element_success);
    RUN_TEST(test_xr77128_get_enabled_fail);
    RUN_TEST(test_xr77128_get_enabled_success);
    RUN_TEST(test_xr77128_set_enabled_event_fail);
    RUN_TEST(test_xr77128_set_enabled_request_fail);
    RUN_TEST(test_xr77128_set_enabled_success);
    RUN_TEST(test_xr77128_set_enabled_busy_success);
    RUN_TEST(test_xr77128_get_voltage_fail);
    RUN_TEST(test_xr77128_get_voltage_request_fail);
    RUN_TEST(test_xr77128_get_voltage_event_fail);
    RUN_TEST(test_xr77128_get_voltage_success);
    RUN_TEST(test_xr77128_set_voltage_range_fail);
    RUN_TEST(test_xr77128_set_voltage_request_fail);
    RUN_TEST(test_xr77128_set_voltage_event_fail);
    RUN_TEST(test_xr77128_set_voltage_success);

    return UNITY_END();
}

int main(void)
{
    return xr77128_test_main();
}
