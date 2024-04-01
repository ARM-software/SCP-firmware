/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_module.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>
#include <fwk_string.h>

#include UNIT_TEST_SRC
#include "config_sc_pll.h"

#include <internal/sc_pll.h>

#include <mod_clock.h>
#include <mod_sc_pll.h>

#define FAKE_RATE      1466
#define LARGE_REF_RATE 1300 * FWK_MHZ

struct sc_pll_dev_ctx dev_ctx_table[FWK_ELEMENT_IDX_COUNT];

void setUp(void)
{
    int dev_idx;
    struct sc_pll_dev_ctx *dev_ctx;
    module_ctx.dev_count = FWK_ELEMENT_IDX_COUNT;
    module_ctx.dev_ctx_table = dev_ctx_table;

    for (dev_idx = 0; dev_idx < FWK_ELEMENT_IDX_COUNT; dev_idx++) {
        dev_ctx = &dev_ctx_table[dev_idx];
        dev_ctx->config = sc_pll_element_table[dev_idx].data;
    }
}

void tearDown(void)
{
    /* Do Nothing */
}

/*!
 * \brief pll unit test: pll_init(), empty config data.
 *
 *  \details Handle case in pll_init() where empty config data is passed.
 */
void test_pll_init_fail(void)
{
    int status;
    /*  Test init by passing NULL pointer instead of data */
    status = sc_pll_init(fwk_module_id_sc_pll, 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}
/*!
 * \brief pll unit test: pll_init(), empty config data.
 *
 *  \details Handle case in pll_init() where empty config data is passed.
 */
void test_pll_init_success(void)
{
    int status;
    /*  Test init by data */
    status = sc_pll_init(fwk_module_id_sc_pll, 1, config_sc_pll.data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}
/*!
 * \brief pll unit test: pll_element_init(), empty config data.
 *
 *  \details Handle case in pll_element_init() where empty config data is
 *       passed.
 */
void test_pll_device_init_fail(void)
{
    int status;
    status = sc_pll_element_init(fwk_element_id_dummy, 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}
/*!
 * \brief pll unit test: pll_element_init(), empty config data.
 *
 *  \details Handle case in pll_element_init() where empty config data is
 *       passed.
 */
void test_pll_device_init_success(void)
{
    int status;
    fwk_id_get_element_idx_ExpectAndReturn(fwk_element_id_dummy, 0);
    status = sc_pll_element_init(
        fwk_element_id_dummy, 0, sc_pll_element_table[0].data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}
/*!
 * \brief pll unit test: pll_process_bind_request(), invalid bind.
 *
 *  \details Handle case in pll_process_bind_request() where binding is
 *       requested for an element instead of the module.
 */
void test_pll_process_bind_request_fail(void)
{
    int status;
    const void *api;
    fwk_id_t api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SC_PLL, 0);
    fwk_id_is_type_ExpectAnyArgsAndReturn(false);
    status = sc_pll_process_bind_request(
        fwk_module_id_fake_module, fwk_element_id_dummy, api_id, &api);
    TEST_ASSERT_EQUAL(status, FWK_E_ACCESS);
}
/*!
 * \brief pll unit test: pll_process_bind_request().
 *
 *  \details Handle case in pll_process_bind_request() where binding is
 *       requested for an element
 */
void test_pll_process_bind_request_success(void)
{
    int status;
    const void *api;
    fwk_id_t api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SC_PLL, 0);
    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    fwk_id_is_equal_IgnoreAndReturn(true);
    status = sc_pll_process_bind_request(
        fwk_module_id_fake_module, fwk_element_id_dummy, api_id, &api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}
/*!
 * \brief pll unit test: test_pll_set_rate_fail()
 *
 *  \details Handle case in test_pll_set_rate_fail() where setting the
 *  rate with invalid element id
 */
void test_pll_set_rate_fail(void)
{
    int status;

    fwk_module_is_valid_element_id_IgnoreAndReturn(false);
    status = sc_pll_set_rate(
        fwk_element_id_dummy, (FAKE_RATE * FWK_MHZ), MOD_CLOCK_ROUND_MODE_NONE);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}
/*!
 * \brief pll unit test: test_pll_set_rate_success
 *
 *  \details Handle case in test_pll_set_rate_success() where setting
 *  rate with valid element id
 */
void test_pll_set_rate_success(void)
{
    int status;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(fwk_element_id_dummy, 0);
    status = sc_pll_set_rate(
        fwk_element_id_dummy, (FAKE_RATE * FWK_MHZ), MOD_CLOCK_ROUND_MODE_NONE);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}
/*!
 * \brief pll unit test: test_pll_get_rate_fail
 *
 *  \details Handle case in test_pll_get_rate_fail() where
 *  sending invalid element Id
 */
void test_pll_get_rate_fail(void)
{
    int status;
    uint64_t rate;

    fwk_module_is_valid_element_id_IgnoreAndReturn(false);
    status = sc_pll_get_rate(fwk_element_id_dummy, &rate);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}
/*!
 * \brief pll unit test: test_pll_get_rate_success
 *
 *  \details Handle case in test_pll_get_rate_success() where
 *  sending valid element id
 */
void test_pll_get_rate_success(void)
{
    int status;
    uint64_t rate;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(fwk_element_id_dummy, 0);
    status = sc_pll_get_rate(fwk_element_id_dummy, &rate);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}
/*!
 * \brief pll unit test: test_pll_get_state_fail
 *
 *  \details Handle case in test_pll_get_state_fail() with invalid
 *  element ID
 */
void test_pll_get_state_fail(void)
{
    int status;
    enum mod_clock_state state;

    fwk_module_is_valid_element_id_IgnoreAndReturn(false);
    status = sc_pll_get_state(fwk_element_id_dummy, &state);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}
/*!
 * \brief pll unit test: test_pll_get_state_success
 *
 *  \details Handle case in test_pll_get_state_success() with
 *  valid element id
 */
void test_pll_get_state_success(void)
{
    int status;
    enum mod_clock_state state;

    fwk_module_is_valid_element_id_IgnoreAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(fwk_element_id_dummy, 0);
    status = sc_pll_get_state(fwk_element_id_dummy, &state);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*!
 * \brief pll unit test: test_pll_calc_fbdiv_fail
 *
 *  \details Handle case in test_pll_calc_fbdiv_fail() with
 *  invalid rate.
 */
void test_pll_calc_fbdiv_fail(void)
{
    int status;

    status = pll_calc_fbdiv(&dev_ctx_table[0], UINT32_MAX * FWK_MHZ, 1, 1);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

/*!
 * \brief pll unit test: test_pll_calc_fbdiv_success
 *
 *  \details Handle case in test_pll_calc_fbdiv_success() with
 *  valid rate.
 */
void test_pll_calc_fbdiv_success(void)
{
    int status;

    status = pll_calc_fbdiv(&dev_ctx_table[0], (2600 * FWK_MHZ), 1, 1);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*!
 * \brief pll unit test: test_pll_calc_fbdiv_small_config_ref_rate
 *
 *  \details Handle case in test_pll_calc_fbdiv_small_config_ref_rate() with
 *  invalid reference rate. As per specification minimum reference frequency
 *  should be 2MHz.
 */
void test_pll_calc_fbdiv_small_config_ref_rate(void)
{
    int status;
    struct sc_pll_dev_ctx ctx;
    const struct mod_sc_pll_dev_config config = {
        .ref_rate = 1 * FWK_MHZ,
        .control_reg0 = &control_reg0,
        .control_reg1 = &control_reg1,
        .initial_rate = SC_PLL_RATE_CPU_PLL0,
        .dev_param = &dev_param,
    };
    ctx.config = &config;
    status = pll_calc_fbdiv(&ctx, (2600 * FWK_MHZ), 1, 1);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

/*!
 * \brief pll unit test: test_pll_calc_fbdiv_large_config_ref_rate
 *
 *  \details Handle case in test_pll_calc_fbdiv_large_config_ref_rate() with
 *  very large reference rate.
 */
void test_pll_calc_fbdiv_large_config_ref_rate(void)
{
    int status;
    struct sc_pll_dev_ctx ctx;
    const struct mod_sc_pll_dev_config config = {
        .ref_rate = LARGE_REF_RATE,
        .control_reg0 = &control_reg0,
        .control_reg1 = &control_reg1,
        .initial_rate = SC_PLL_RATE_CPU_PLL0,
        .dev_param = &dev_param,
    };
    ctx.config = &config;
    status = pll_calc_fbdiv(&ctx, (2600 * FWK_MHZ), 1, 1);
    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

/*!
 * \brief pll unit test: test_pll_write_success
 *
 *  \details Handle case in test_pll_write_success() for a successful pll lock.
 */
void test_pll_write_success(void)
{
    int status;
    status = pll_write(&dev_ctx_table[0], 1, 1, 1, 52, SC_PLL_RATE_CPU_PLL0);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*!
 * \brief pll unit test: test_pll_write_fail
 *
 *  \details Handle case in test_pll_write_fail() for the pll lock getting
 *  timed-out.
 */
void test_pll_write_fail(void)
{
    int status;
    struct sc_pll_dev_ctx ctx;
    const struct mod_sc_pll_dev_config config = {
        .control_reg0 = &control_reg0,
        .control_reg1 = &control_reg1_timeout,
        .initial_rate = SC_PLL_RATE_CPU_PLL0,
        .ref_rate = CLOCK_RATE_REFCLK,
        .dev_param = &dev_param,
    };
    ctx.config = &config;
    status = pll_write(&ctx, 1, 1, 1, 52, SC_PLL_RATE_CPU_PLL0);
    TEST_ASSERT_EQUAL(status, FWK_E_TIMEOUT);
}

int pll_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_pll_init_fail);
    RUN_TEST(test_pll_init_success);
    RUN_TEST(test_pll_device_init_fail);
    RUN_TEST(test_pll_device_init_success);
    RUN_TEST(test_pll_process_bind_request_fail);
    RUN_TEST(test_pll_process_bind_request_success);
    RUN_TEST(test_pll_set_rate_fail);
    RUN_TEST(test_pll_set_rate_success);
    RUN_TEST(test_pll_get_rate_fail);
    RUN_TEST(test_pll_get_rate_success);
    RUN_TEST(test_pll_get_state_fail);
    RUN_TEST(test_pll_get_state_success);
    RUN_TEST(test_pll_calc_fbdiv_fail);
    RUN_TEST(test_pll_calc_fbdiv_success);
    RUN_TEST(test_pll_calc_fbdiv_small_config_ref_rate);
    RUN_TEST(test_pll_calc_fbdiv_large_config_ref_rate);
    RUN_TEST(test_pll_write_success);
    RUN_TEST(test_pll_write_fail);

    return UNITY_END();
}

int main(void)
{
    return pll_test_main();
}
