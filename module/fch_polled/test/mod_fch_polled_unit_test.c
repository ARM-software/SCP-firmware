/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_fch_polled.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_core.h>
#include <Mockfwk_id.h>
#include <Mockfwk_interrupt.h>
#include <Mockfwk_module.h>
#include <Mockmod_fch_polled_extra.h>

#include UNIT_TEST_SRC

int fake_fch0_callback_status = FWK_E_STATE;

struct mod_timer_alarm_api mock_alarm_api = {
    .start = fch_polled_extra_alarm_start,
    .stop = fch_polled_extra_alarm_stop,
};

void setUp(void)
{
    unsigned int index;

    fch_polled_ctx.channel_count = FAKE_FCH_POLLED_COUNT;
    fch_polled_ctx.fch_config = &fake_fch_config;
    fch_polled_ctx.fch_alarm_api = &mock_alarm_api;

    for (index = 0; index < FAKE_FCH_POLLED_COUNT; index++) {
        fch_polled_ctx.channel_ctx_table[index].config =
            (struct mod_fch_polled_channel_config *)
                fake_fch_polled_element_table[index]
                    .data;
    }
}

void test_init(void)
{
    fch_polled_ctx.channel_ctx_table = fwk_mm_calloc(
        FAKE_FCH_POLLED_COUNT, sizeof(fch_polled_ctx.channel_ctx_table[0]));
}

void tearDown(void)
{
}

void fake_fch0_callback(uintptr_t param)
{
    fake_fch0_callback_status = FWK_SUCCESS;
    fwk_id_t id = *(fwk_id_t *)param;
    TEST_ASSERT_EQUAL(id.element.element_idx, FAKE_FCH_POLLED_0);
}

/* Test fast_channel_callback() for success case */
void utest_fast_channel_callback()
{
    struct mod_fch_polled_channel_ctx *channel_ctx;
    fake_fch0_callback_status = FWK_E_STATE;

    channel_ctx = &fch_polled_ctx.channel_ctx_table[FAKE_FCH_POLLED_0];

    /* Store the context specific value */
    channel_ctx->param = (uintptr_t)&fake_fch_0;

    /* Store the callback function pointer */
    channel_ctx->fch_callback = fake_fch0_callback;

    fast_channel_alarm_callback((uintptr_t)channel_ctx);

    TEST_ASSERT_EQUAL(fake_fch0_callback_status, FWK_SUCCESS);
}

/* A test callback stub to verify passed millisecond parameter */
int alarm_start_callback(
    fwk_id_t alarm_id,
    unsigned int milliseconds,
    enum mod_timer_alarm_type type,
    void (*callback)(uintptr_t param),
    uintptr_t param,
    int NumCalls)
{
    uint32_t interval_assert_value;

    if (fch_polled_ctx.fch_config->fch_poll_rate < FCH_MIN_POLL_RATE_US) {
        interval_assert_value = (uint32_t)FCH_MIN_POLL_RATE_US / 1000;
    } else {
        interval_assert_value =
            (uint32_t)fch_polled_ctx.fch_config->fch_poll_rate / 1000;
    }

    TEST_ASSERT_EQUAL(interval_assert_value, milliseconds);

    return FWK_SUCCESS;
}

/* Test start_alarm() for success case */
void utest_start_alarm()
{
    int status;

    fch_polled_extra_alarm_start_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fch_polled_extra_alarm_start_AddCallback(alarm_start_callback);

    status = start_alarm(NULL);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*
 *  Test start_alarm() success case with configured poll rate less than
 *  FCH_MIN_POLL_RATE_US.
 */
void utest_start_alarm_less_than_min_poll_rate()
{
    int status;

    fch_polled_extra_alarm_start_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fch_polled_extra_alarm_start_AddCallback(alarm_start_callback);

    fch_polled_ctx.fch_config = &fake_fch_config_2;

    status = start_alarm(NULL);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/* Test mod_fch_polled_get_fch() to get address */
void utest_mod_fch_polled_get_fch_addr()
{
    int status;
    struct mod_transport_fast_channel_addr fch_addr;

    fwk_id_get_element_idx_ExpectAndReturn(fake_fch_0, FAKE_FCH_POLLED_0);

    status = mod_fch_polled_get_fch_address(fake_fch_0, &fch_addr);

    TEST_ASSERT_EQUAL(fch_addr.local_view_address, FCH_ADDRESS_LEVEL_SET(0));
    TEST_ASSERT_EQUAL(
        fch_addr.target_view_address, FCH_ADDRESS_LEVEL_SET_AP(0));
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/* Test mod_fch_polled_get_fch() to get interrupt type */
void utest_mod_fch_polled_get_fch_intr_type()
{
    int status;
    enum mod_transport_fch_interrupt_type fch_interrupt_type;

    status =
        mod_fch_polled_get_fch_interrupt_type(fake_fch_0, &fch_interrupt_type);

    TEST_ASSERT_EQUAL(
        fch_interrupt_type, MOD_TRANSPORT_FCH_INTERRUPT_TYPE_TIMER);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/* Test mod_fch_polled_get_fch_doorbell_info() for success case */
void utest_mod_fch_polled_get_fch_doorbell_info()
{
    int status;
    struct mod_transport_fch_doorbell_info doorbell_info;

    status = mod_fch_polled_get_fch_doorbell_info(fake_fch_0, &doorbell_info);
    TEST_ASSERT_EQUAL(doorbell_info.doorbell_support, false);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/* Test mod_fch_polled_get_fch_rate_limit() for success case */
void utest_mod_fch_polled_get_fch_rate_limit()
{
    int status;
    uint32_t fch_rate_limit;

    status = mod_fch_polled_get_fch_rate_limit(fake_fch_0, &fch_rate_limit);

    TEST_ASSERT_EQUAL(fch_rate_limit, FAKE_RATE_LIMIT);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/* Test mod_fch_polled_register_callback() for success case */
void utest_mod_fch_polled_register_callback()
{
    int status = FWK_E_STATE;
    struct mod_fch_polled_channel_ctx *channel_ctx;

    channel_ctx = &fch_polled_ctx.channel_ctx_table[0];

    fwk_id_get_element_idx_ExpectAndReturn(fake_fch_0, FAKE_FCH_POLLED_0);

    fch_polled_extra_alarm_start_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    status = mod_fch_polled_register_callback(
        fake_fch_0, (uintptr_t)NULL, (void *)0XDEADDEAD);

    TEST_ASSERT_EQUAL(0xDEADDEAD, (uintptr_t)channel_ctx->fch_callback);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/* Test mod_fch_polled_init() for success case */
void utest_mod_fch_polled_init()
{
    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FCH_POLLED);
    int status;

    struct mod_fch_polled_config *config =
        (struct mod_fch_polled_config *)&config_fake_fch_polled.data;

    fwk_id_type_is_valid_ExpectAndReturn(config->fch_alarm_id, true);
    status = mod_fch_polled_init(module_id, 2, &config_fake_fch_polled.data);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/* Test mod_fch_polled_init() for zero elements */
void utest_mod_fch_polled_init_zero_elements()
{
    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FCH_POLLED);
    int status;
    status = mod_fch_polled_init(module_id, 0, NULL);

    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}

/* Test mod_fch_polled_init() for invalid poll rate */
void utest_mod_fch_polled_init_invalid_poll_rate_config()
{
    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FCH_POLLED);
    int status;
    struct mod_fch_polled_config config_invalid_poll_rate = {
        .fch_poll_rate = 0, /* Invalid, must be > 0 */
    };

    status = mod_fch_polled_init(module_id, 2, &config_invalid_poll_rate);

    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}

/* Test mod_fch_polled_init() for invalid alarm_id  */
void utest_mod_fch_polled_init_invalid_alarm_id_config()
{
    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FCH_POLLED);
    int status;
    struct mod_fch_polled_config config_invalid_alarm_id = {
        .fch_poll_rate = __LINE__,
        .fch_alarm_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_COUNT, /* Invalid module id */
            0,
            0),
    };

    fwk_id_type_is_valid_ExpectAndReturn(
        config_invalid_alarm_id.fch_alarm_id, false);

    status = mod_fch_polled_init(module_id, 2, &config_invalid_alarm_id);

    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}

/* Test mod_fch_polled_channel_init() for success case  */
void utest_mod_fch_polled_channel_init()
{
    int status;
    struct mod_fch_polled_channel_config fch_config;
    fch_config.fch_addr.local_view_address = __LINE__; /* random */
    fch_config.fch_addr.target_view_address = __LINE__; /* random */
    fch_config.fch_addr.length = 24;

    fwk_id_get_element_idx_ExpectAndReturn(fake_fch_0, FAKE_FCH_POLLED_0);

    status = mod_fch_polled_channel_init(
        fake_fch_0,
        0, /* unused */
        &fch_config);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/* Test mod_fch_polled_channel_init() for invalid case  */
void utest_mod_fch_polled_channel_init_invalid_data()
{
    int status;
    struct mod_fch_polled_channel_config fch_config;
    fch_config.fch_addr.local_view_address = 0x00000000; /* random */
    fch_config.fch_addr.local_view_address = 0x00000000; /* random */
    fch_config.fch_addr.length = 0;

    fwk_id_get_element_idx_ExpectAndReturn(fake_fch_0, FAKE_FCH_POLLED_0);

    status = mod_fch_polled_channel_init(
        fake_fch_0,
        0, /* unused */
        &fch_config);

    TEST_ASSERT_EQUAL(status, FWK_E_DATA);
}

/*
 * Test mod_fch_polled_bind(), should succeed for a valid module id
 * and round 0
 */
void utest_mod_fch_polled_bind_round_0()
{
    int status;
    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TRANSPORT);

    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, false);

    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = mod_fch_polled_bind(module_id, 0);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*
 * Test mod_fch_polled_bind(), should succeed for round  0 and
 * for a valid element
 */
void utest_mod_fch_polled_bind_round_0_with_valid_element_id()
{
    int status;

    fwk_id_is_type_ExpectAndReturn(fake_fch_0, FWK_ID_TYPE_ELEMENT, true);
    /* Function expected return success without actual bind for element */
    status = mod_fch_polled_bind(fake_fch_0, 0);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*
 * Test mod_fch_polled_bind(), should succeed for round  1
 * as it does not do any action for round 1
 */
void utest_mod_fch_polled_bind_round_1()
{
    int status;
    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TRANSPORT);

    status = mod_fch_polled_bind(module_id, 1);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/* Test mod_fch_polled_bind(), should fail if fwk_module_bind() fails */
void utest_mod_fch_polled_bind_round_0_bind_fail()
{
    int status;
    fwk_id_t module_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TRANSPORT);

    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, false);

    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_E_SUPPORT);

    status = mod_fch_polled_bind(module_id, 0);

    TEST_ASSERT_EQUAL(status, FWK_E_PANIC);
}

/* Test mod_fch_polled_process_bind_request(), all valid parameters */
void utest_mod_fch_polled_process_bind_request()
{
    int status;
    fwk_id_t source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TRANSPORT);
    fwk_id_t target_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FCH_POLLED);
    fwk_id_t api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_TRANSPORT, MOD_FCH_POLLED_API_IDX_TRANSPORT);
    struct mod_transport_driver_api driver_api;

    fwk_id_get_api_idx_ExpectAndReturn(
        api_id, MOD_FCH_POLLED_API_IDX_TRANSPORT);

    status = mod_fch_polled_process_bind_request(
        source_id, target_id, api_id, (void *)&driver_api);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

/*
 * Test mod_fch_polled_process_bind_request(), should fail for
 * invalid api id
 */
void utest_mod_fch_polled_process_bind_request_invalid_api()
{
    int status;
    fwk_id_t source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TRANSPORT);
    fwk_id_t target_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FCH_POLLED);
    fwk_id_t api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_TRANSPORT, MOD_FCH_POLLED_API_IDX_TRANSPORT + 1);
    struct mod_transport_driver_api driver_api;

    fwk_id_get_api_idx_ExpectAndReturn(
        api_id, MOD_FCH_POLLED_API_IDX_TRANSPORT + 1);

    status = mod_fch_polled_process_bind_request(
        source_id, target_id, api_id, (void *)&driver_api);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

int fch_polled_test_main(void)
{
    UNITY_BEGIN();
    test_init();
    RUN_TEST(utest_fast_channel_callback);
    RUN_TEST(utest_start_alarm);
    RUN_TEST(utest_start_alarm_less_than_min_poll_rate);
    RUN_TEST(utest_mod_fch_polled_get_fch_addr);
    RUN_TEST(utest_mod_fch_polled_get_fch_intr_type);
    RUN_TEST(utest_mod_fch_polled_get_fch_doorbell_info);
    RUN_TEST(utest_mod_fch_polled_get_fch_rate_limit);
    RUN_TEST(utest_mod_fch_polled_register_callback);
    RUN_TEST(utest_mod_fch_polled_init);
    RUN_TEST(utest_mod_fch_polled_init_zero_elements);
    RUN_TEST(utest_mod_fch_polled_init_invalid_poll_rate_config);
    RUN_TEST(utest_mod_fch_polled_init_invalid_alarm_id_config);
    RUN_TEST(utest_mod_fch_polled_channel_init);
    RUN_TEST(utest_mod_fch_polled_channel_init_invalid_data);
    RUN_TEST(utest_mod_fch_polled_bind_round_0);
    RUN_TEST(utest_mod_fch_polled_bind_round_0_with_valid_element_id);
    RUN_TEST(utest_mod_fch_polled_bind_round_1);
    RUN_TEST(utest_mod_fch_polled_bind_round_0_bind_fail);
    RUN_TEST(utest_mod_fch_polled_process_bind_request);
    RUN_TEST(utest_mod_fch_polled_process_bind_request_invalid_api);
    return UNITY_END();
}

int main(void)
{
    return fch_polled_test_main();
}
