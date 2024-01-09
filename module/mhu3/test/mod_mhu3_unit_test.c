/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mhu3.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_core.h>
#include <Mockfwk_interrupt.h>
#include <Mockfwk_module.h>

#include <internal/mhu3.h>

#include <mod_mhu3.h>

#include UNIT_TEST_SRC

/* Some invalid values */
#define MHU3_TEST_INVALID_VALUE     0xFFFF
#define MHU3_TEST_INVALID_API_VALUE 0xF

/*!
 * For unit tests, there is no real MHUv3 hardware.
 * Below is the allocated memory that will be accessed by unit test
 * functions as a MHUv3 hardware, therefore unit tests must pre-load desired
 * values in the memory before calling unit test functions.
 */
struct mhu3_pbx_reg *fake_device_1_pbx_base;
struct mhu3_pbx_pdbcw_reg *fake_device_1_pdbcw;
struct mhu3_mbx_reg *fake_device_1_mbx_base;
struct mhu3_mbx_mdbcw_reg *fake_device_1_mdbcw;

void setUp(void)
{
}

void tearDown(void)
{
}

/*!
 * \brief mhu3 unit test: mhu3_raise_interrupt(), wrong channel type.
 *
 *  \details Handle case in mhu3_raise_interrupt where it detects unexpected
 *      channel type only valid value is doorbell channel type.
 */
void test_mhu3_raise_interrupt_wrong_channel_type(void)
{
    int status;
    enum mod_mhu3_channel_type prev;

    struct mhu3_device_ctx *device_ctx;
    struct mod_mhu3_channel_config *channel;

    device_ctx = &mhu3_ctx.device_ctx_table[FAKE_DEVICE_1_CHANNEL_DBCH_0];
    channel = &(device_ctx->config->channels[FAKE_DEVICE_1_CHANNEL_DBCH_0]);

    prev = channel->type;

    /* Force wrong channel type */
    channel->type = MHU3_TEST_INVALID_VALUE;

    fwk_id_t ch_id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_DBCH_0);

    status = mhu3_raise_interrupt(ch_id);
    TEST_ASSERT(status == FWK_E_PARAM);

    channel->type = prev;
}

/*!
 * \brief mhu3 unit test: mhu3_raise_interrupt(), valid case.
 *
 *  \details Handle case in mhu3_raise_interrupt raises interrupt
 *       without any failures.
 */
void test_mhu3_raise_interrupt_valid_case(void)
{
    int status;

    fwk_id_t ch_id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_DBCH_0_IDX);

    status = mhu3_raise_interrupt(ch_id);
    TEST_ASSERT(status == FWK_SUCCESS);
}

/*!
 * \brief mhu3 unit test: is_channel_free(), busy case.
 *
 *  \details Checking for channel availability.
 *      Expects to receive a `false` return value indicating
 *      that the channel is busy.
 */
void test_is_channel_free_busy_case(void)
{
    int status;
    struct channel_status_data channel_stat;
    uint32_t st_reg, pbx_flag_pos;

    /* Force PBX channel flag position to 1 */
    pbx_flag_pos = 1;
    /* Force channel status to busy */
    st_reg = 0x1u << pbx_flag_pos;

    channel_stat.st_reg = &st_reg;
    channel_stat.position = 0x1u << pbx_flag_pos;

    status = is_channel_free(&channel_stat);
    TEST_ASSERT(status == false);
}

/*!
 * \brief mhu3 unit test: is_channel_free(), available case.
 *
 *  \details Checking for channel availability.
 *      Expects to receive a `true` return value indicating
 *      that the channel is free.
 */
void test_is_channel_free_available_case(void)
{
    int status;
    struct channel_status_data channel_stat;
    uint32_t st_reg, pbx_flag_pos;

    /* Force PBX channel flag position to 1 */
    pbx_flag_pos = 1;
    /* Force channel status to free */
    st_reg = 0x0u << pbx_flag_pos;

    channel_stat.st_reg = &st_reg;
    channel_stat.position = 0x1u << pbx_flag_pos;

    status = is_channel_free(&channel_stat);
    TEST_ASSERT(status == true);
}

/*!
 * \brief mhu3 unit test: mhu3_bind(), round 0, successful case.
 *
 *  \details Handle case where fwk_module_bind is successful
 */
void test_mhu3_bind_round_0_success(void)
{
    int status;
    fwk_id_t device_id = { 0 };

    status = mhu3_bind(device_id, 0);
    TEST_ASSERT(status == FWK_SUCCESS);
}

/*!
 * \brief mhu3 unit test: mhu3_bind(), round 1, successful case.
 *
 *  \details Handle case where fwk_module_bind is successful
 */
void test_mhu3_bind_round_1_success(void)
{
    int status;
    struct mhu3_device_ctx *device_ctx;
    struct mhu3_channel_ctx *channel_ctx;

    device_ctx = &mhu3_ctx.device_ctx_table[MHU3_DEVICE_IDX_DEVICE_1];
    channel_ctx =
        &device_ctx->channel_ctx_table[FAKE_DEVICE_1_CHANNEL_DBCH_0_IDX];
    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    channel_ctx->transport_id_bound = true;
    fwk_id_t device_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_MHU3, MHU3_DEVICE_IDX_DEVICE_1);

    status = mhu3_bind(device_id, 1);
    TEST_ASSERT(status == FWK_SUCCESS);

    channel_ctx->transport_id_bound = false;
}

/*!
 * \brief mhu3 unit test: mhu3_bind(), failure case.
 *
 *  \details Handle case where fwk_module_bind fails
 */
void test_mhu3_bind_fail(void)
{
    int status;
    struct mhu3_device_ctx *device_ctx;
    struct mhu3_channel_ctx *channel_ctx;

    device_ctx = &mhu3_ctx.device_ctx_table[MHU3_DEVICE_IDX_DEVICE_1];
    channel_ctx =
        &device_ctx->channel_ctx_table[FAKE_DEVICE_1_CHANNEL_DBCH_0_IDX];

    /* Force fwk_module_bind to fail */
    fwk_module_bind_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    channel_ctx->transport_id_bound = true;
    fwk_id_t device_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_MHU3, MHU3_DEVICE_IDX_DEVICE_1);

    status = mhu3_bind(device_id, 1);
    channel_ctx->transport_id_bound = false;
    TEST_ASSERT(status == FWK_E_PARAM);
}

/*!
 * \brief mhu3 unit test: mhu3_process_bind_request(), failure bind request
 *     for invalid api_id
 *
 *  \details Handle case where mhu3_process_bind_request fails because
 *      binding is requested for invalid api_id
 */
void test_mhu3_process_bind_request_fail_invalid_api_id(void)
{
    int status;
    fwk_id_t source_id;
    fwk_id_t target_id;
    fwk_id_t api_id;
    const void *api;

    source_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_TRANSPORT, 0, 0);
    target_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_MHU3, 0, 0);

    /* Send invalid api_id */
    api_id = FWK_ID_API(FWK_MODULE_IDX_MHU3, MOD_MHU3_API_IDX_COUNT);
    status = mhu3_process_bind_request(source_id, target_id, api_id, &api);
    TEST_ASSERT(status == FWK_E_PARAM);
}

/*!
 * \brief mhu3 unit test: mhu3_process_bind_request(), successful
 *
 *  \details Handle case where mhu3_process_bind_request succeeds
 *
 */
void test_mhu3_process_bind_request_success(void)
{
    int status;
    fwk_id_t source_id;
    fwk_id_t target_id;
    fwk_id_t api_id;
    const void *api;

    source_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_TRANSPORT, 0, 0);
    target_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_MHU3, 0, 0);

    api_id = FWK_ID_API(FWK_MODULE_IDX_MHU3, MOD_MHU3_API_IDX_TRANSPORT_DRIVER);
    status = mhu3_process_bind_request(source_id, target_id, api_id, &api);
    TEST_ASSERT(status == FWK_SUCCESS);
}
/*!
 * \brief mhu3 unit test: mhu3_process_bind_request(), failure bind request
 *     for non sub element
 *
 *  \details Handle case where mhu3_process_bind_request fails because
 *      binding is requested for by non-subelement type.
 */
void test_mhu3_process_bind_request_fail_non_sub_element(void)
{
    int status;
    fwk_id_t source_id;
    fwk_id_t target_id;
    fwk_id_t api_id;
    const void *api;

    source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_TRANSPORT, 0);
    target_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MHU3, 0);
    api_id = FWK_ID_API(FWK_MODULE_IDX_MHU3, MOD_MHU3_API_IDX_TRANSPORT_DRIVER);
    status = mhu3_process_bind_request(source_id, target_id, api_id, &api);

    TEST_ASSERT(status == FWK_E_ACCESS);
}

void test_mhu3_start_id_module_success(void)
{
    int status;
    fwk_id_t id;

    id = FWK_ID_MODULE(FWK_MODULE_IDX_MHU3);
    status = mhu3_start(id);
    TEST_ASSERT(status == FWK_SUCCESS);
}

void test_mhu3_start_id_element_success(void)
{
    int status;
    fwk_id_t id;

    fwk_interrupt_set_isr_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fwk_interrupt_enable_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MHU3, MHU3_DEVICE_IDX_DEVICE_1);
    status = mhu3_start(id);
    TEST_ASSERT(status == FWK_SUCCESS);
}

void test_mhu3_start_id_element_set_isr_fails(void)
{
    int status;
    fwk_id_t id;

    fwk_interrupt_set_isr_ExpectAnyArgsAndReturn(FWK_E_PARAM);
    id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MHU3, MHU3_DEVICE_IDX_DEVICE_1);
    status = mhu3_start(id);
    TEST_ASSERT(status == FWK_E_PARAM);
}

void test_mhu3_start_id_interrupt_enable_fails(void)
{
    int status;
    fwk_id_t id;

    fwk_interrupt_set_isr_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    fwk_interrupt_enable_ExpectAnyArgsAndReturn(FWK_E_STATE);
    id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MHU3, MHU3_DEVICE_IDX_DEVICE_1);
    status = mhu3_start(id);
    TEST_ASSERT(status == FWK_E_STATE);
}

void test_mhu3_get_fch_address_invalid_sub_element(void)
{
    int status;
    fwk_id_t id = { 0 };
    struct mod_transport_fast_channel_addr fch_addr;

    /*
     * mhu3_get_fch_address expects a valid id which has a subelement.
     * check if it returns an FWK_E_PARAM if it gets an id without
     * field subelement
     */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(false);
    status = mhu3_get_fch_address(id, &fch_addr);
    TEST_ASSERT(status == FWK_E_PARAM);
}

void test_mhu3_get_fch_interrupt_type_invalid_sub_element(void)
{
    int status;
    fwk_id_t id = { 0 };
    enum mod_transport_fch_interrupt_type fch_interrupt_type;

    /*
     * mhu3_get_fch_interrupt_type expects a valid id which has a subelement.
     * check if it returns an FWK_E_PARAM if it gets an id without
     * field subelement
     */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(false);
    status = mhu3_get_fch_interrupt_type(id, &fch_interrupt_type);
    TEST_ASSERT(status == FWK_E_PARAM);
}

void test_mhu3_get_fch_address_invalid_channel_type(void)
{
    int status;
    fwk_id_t id = { 0 };
    struct mod_transport_fast_channel_addr fch_addr;

    /*
     * mhu3_get_fch_address should return FWK_E_PARAM if first parameter fch_id
     * is of wrong type that is other than the fast channel type
     */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_DBCH_0_IDX); /*
                                            * Note. no
                                            * FAKE_DEVICE_1_CHANNEL_DBCH_0_IDX
                                            * is not a fast channel
                                            */

    status = mhu3_get_fch_address(id, &fch_addr);
    TEST_ASSERT(status == FWK_E_PARAM);
}

void test_mhu3_get_fch_interrupt_type_invalid_channel_type(void)
{
    int status;
    fwk_id_t id = { 0 };
    enum mod_transport_fch_interrupt_type fch_interrupt_type;

    /*
     * mhu3_get_fch_interrupt_type should return FWK_E_PARAM if first parameter
     * fch_id is of wrong type that is other than the fast channel type
     */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_DBCH_0_IDX); /*
                                            * Note. no
                                            * FAKE_DEVICE_1_CHANNEL_DBCH_0_IDX
                                            * is not a fast channel
                                            */

    status = mhu3_get_fch_interrupt_type(id, &fch_interrupt_type);
    TEST_ASSERT(status == FWK_E_PARAM);
}

void test_mhu3_get_fch_address_null_fch_addr(void)
{
    int status;
    fwk_id_t id = { 0 };
    struct mod_transport_fast_channel_addr *null_fch_addr = NULL;

    /*
     * mhu3_get_fch_address should return FWK_E_PARAM if second parameter
     * fch_addr is NULL.
     */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_FCH_0_IN_IDX);

    status = mhu3_get_fch_address(id, null_fch_addr);
    TEST_ASSERT(status == FWK_E_PARAM);
}

void test_mhu3_get_fch_interrupt_null_fch_interrupt_type(void)
{
    int status;
    fwk_id_t id = { 0 };
    enum mod_transport_fch_interrupt_type *null_fch_interrupt_type = NULL;

    /*
     * mhu3_get_fch_interrupt_type should return FWK_E_PARAM if second parameter
     * null_fch_interrupt_type is NULL.
     */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_FCH_0_IN_IDX);

    status = mhu3_get_fch_interrupt_type(id, null_fch_interrupt_type);
    TEST_ASSERT(status == FWK_E_PARAM);
}

void test_mhu3_get_fch_address_valid_fch_dir_in(void)
{
    int status;
    fwk_id_t id = { 0 };
    struct mod_transport_fast_channel_addr fch_addr;

    /* Success case, send valid id, address and fast channel direction (in) */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_FCH_0_IN_IDX);

    status = mhu3_get_fch_address(id, &fch_addr);
    TEST_ASSERT(status == FWK_SUCCESS);
}

void test_mhu3_get_fch_address_valid_fch_dir_out(void)
{
    int status;
    fwk_id_t id;
    struct mod_transport_fast_channel_addr fch_addr;

    /* Success case, send valid id, address and fast channel direction (out) */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_FCH_0_OUT_IDX);

    status = mhu3_get_fch_address(id, &fch_addr);
    TEST_ASSERT(status == FWK_SUCCESS);
}

void test_mhu3_get_fch_interrupt_type_valid(void)
{
    int status;
    fwk_id_t id = { 0 };
    enum mod_transport_fch_interrupt_type fch_interrupt_type;

    /* Success case, send valid id, non-null parameter fch_interrupt_type*/
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_FCH_0_IN_IDX);

    status = mhu3_get_fch_interrupt_type(id, &fch_interrupt_type);
    TEST_ASSERT(status == FWK_SUCCESS);
    TEST_ASSERT(fch_interrupt_type == MOD_TRANSPORT_FCH_INTERRUPT_TYPE_HW);
}

void fch_callback_test(uintptr_t param)
{
}
void test_mhu3_fch_register_callback_valid(void)
{
    int status;
    fwk_id_t id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_FCH_0_OUT_IDX);

    /* All valid params, expect success */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    status =
        mhu3_fch_register_callback(id, (uintptr_t)&status, fch_callback_test);
    TEST_ASSERT(status == FWK_SUCCESS);
}

void test_mhu3_fch_register_callback_invalid_sub_element_id(void)
{
    int status;
    fwk_id_t id = { 0 };

    /* Force fwk_module_is_valid_sub_element_id to return false */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(false);
    status =
        mhu3_fch_register_callback(id, (uintptr_t)&status, fch_callback_test);
    TEST_ASSERT(status == FWK_E_PARAM);
}

void test_mhu3_fch_register_callback_null_param(void)
{
    int status;
    fwk_id_t id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_FCH_0_OUT_IDX);

    /* Check if it returns FWK_E_PARAM if 'param' is NULL */
    status = mhu3_fch_register_callback(id, (uintptr_t)NULL, fch_callback_test);
    TEST_ASSERT(status == FWK_E_PARAM);
}

void test_mhu3_fch_register_callback_null_callback_addr(void)
{
    int status;
    fwk_id_t id = FWK_ID_SUB_ELEMENT_INIT(
        FWK_MODULE_IDX_MHU3,
        MHU3_DEVICE_IDX_DEVICE_1,
        FAKE_DEVICE_1_CHANNEL_FCH_0_OUT_IDX);

    /* Check if it returns FWK_E_PARAM if callback address is NULL */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    status = mhu3_fch_register_callback(id, (uintptr_t)&status, NULL);
    TEST_ASSERT(status == FWK_E_PARAM);
}

/* Helper function to setup values for unit tests */
static int mhu3_fake_init(void)
{
    int status;
    fwk_id_t module_id;
    fwk_id_t device_id;
    unsigned int sub_element_count;

    uint8_t *tmp_pbx_base;
    uint8_t *tmp_mbx_base;
    uint32_t *feat_spt0;

    struct mod_mhu3_device_config *device_config;

    device_config =
        (struct mod_mhu3_device_config *)element_table[MHU3_DEVICE_IDX_DEVICE_1]
            .data;

    /* Allocate some memory for MHU fake device1 PBX/MBX */
    tmp_pbx_base = aligned_alloc(
        sizeof(unsigned long),
        sizeof(struct mhu3_pbx_reg) + MHU3_PBX_PDBCW_PAGE_OFFSET +
            sizeof(struct mhu3_pbx_pdbcw_reg));
    tmp_mbx_base = aligned_alloc(
        sizeof(unsigned long),
        sizeof(struct mhu3_mbx_reg) + MHU3_MBX_MDBCW_PAGE_OFFSET +
            sizeof(struct mhu3_mbx_mdbcw_reg));

    fake_device_1_pbx_base = (struct mhu3_pbx_reg *)tmp_pbx_base;
    fake_device_1_mbx_base = (struct mhu3_mbx_reg *)tmp_mbx_base;

    fake_device_1_pdbcw = (struct mhu3_pbx_pdbcw_reg
                               *)(tmp_pbx_base + MHU3_PBX_PDBCW_PAGE_OFFSET);
    fake_device_1_mdbcw = (struct mhu3_mbx_mdbcw_reg
                               *)(tmp_mbx_base + MHU3_MBX_MDBCW_PAGE_OFFSET);

    feat_spt0 = (uint32_t *)(&fake_device_1_mbx_base->MBX_FEAT_SPT0);
    *feat_spt0 |= (1U << MHU3_FEAT_SPT0_FCE_SPT_BITSTART);
    feat_spt0 = (uint32_t *)(&fake_device_1_pbx_base->PBX_FEAT_SPT0);
    *feat_spt0 |= (1U << MHU3_FEAT_SPT0_FCE_SPT_BITSTART);

    device_config->in = (uintptr_t)fake_device_1_mbx_base;
    device_config->out = (uintptr_t)fake_device_1_pbx_base;

    module_id = FWK_ID_MODULE(FWK_MODULE_IDX_MHU3);
    status = mhu3_init(module_id, MHU3_DEVICE_IDX_COUNT, NULL);
    if (status != FWK_SUCCESS) {
        printf("[MHU3 UT] Can not execute test cases mhu3_init failed\n");
        return -1;
    }

    device_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MHU3, MHU3_DEVICE_IDX_DEVICE_1);
    sub_element_count = FAKE_DEVICE_1_NUM_CH;
    status = mhu3_device_init(device_id, sub_element_count, device_config);
    if (status != FWK_SUCCESS) {
        printf(
            "[MHU3 UT] Can not execute test cases mhu3_device_init failed\n");
        return -1;
    }

    return 0;
}

int mhu3_test_main(void)
{
    int status;

    status = mhu3_fake_init();
    if (status != 0) {
        return status;
    }

    UNITY_BEGIN();
    RUN_TEST(test_mhu3_raise_interrupt_wrong_channel_type);
    RUN_TEST(test_mhu3_raise_interrupt_valid_case);
    RUN_TEST(test_is_channel_free_busy_case);
    RUN_TEST(test_is_channel_free_available_case);
    RUN_TEST(test_mhu3_bind_round_0_success);
    RUN_TEST(test_mhu3_bind_round_1_success);
    RUN_TEST(test_mhu3_bind_fail);
    RUN_TEST(test_mhu3_process_bind_request_success);
    RUN_TEST(test_mhu3_process_bind_request_fail_invalid_api_id);
    RUN_TEST(test_mhu3_process_bind_request_fail_non_sub_element);
    RUN_TEST(test_mhu3_start_id_module_success);
    RUN_TEST(test_mhu3_start_id_element_success);
    RUN_TEST(test_mhu3_start_id_element_set_isr_fails);
    RUN_TEST(test_mhu3_start_id_interrupt_enable_fails);
    RUN_TEST(test_mhu3_get_fch_address_invalid_sub_element);
    RUN_TEST(test_mhu3_get_fch_address_invalid_channel_type);
    RUN_TEST(test_mhu3_get_fch_address_null_fch_addr);
    RUN_TEST(test_mhu3_get_fch_address_valid_fch_dir_in);
    RUN_TEST(test_mhu3_get_fch_address_valid_fch_dir_out);
    RUN_TEST(test_mhu3_get_fch_interrupt_type_invalid_sub_element);
    RUN_TEST(test_mhu3_get_fch_interrupt_type_invalid_channel_type);
    RUN_TEST(test_mhu3_get_fch_interrupt_null_fch_interrupt_type);
    RUN_TEST(test_mhu3_get_fch_interrupt_type_valid);
    RUN_TEST(test_mhu3_fch_register_callback_valid);
    RUN_TEST(test_mhu3_fch_register_callback_invalid_sub_element_id);
    RUN_TEST(test_mhu3_fch_register_callback_null_param);
    RUN_TEST(test_mhu3_fch_register_callback_null_callback_addr);

    return UNITY_END();
}

int main(void)
{
    return mhu3_test_main();
}
