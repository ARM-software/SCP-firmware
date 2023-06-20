/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * mod_optee_mbx_unit_test.c is comming from the template file
 * This is the minimum file setup you need to get started with Unit Testing
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_module.h>
#include <Mockfwk_mm.h>
#include <Mockmod_optee_mbx_extra.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC
#include "config_optee_mbx.h"

void setUp(void)
{
}

void tearDown(void)
{
}

struct mbx_device_ctx fake_optee_mbx_ctx[FAKE_SCMI_SERVICE_IDX_COUNT];

/*!
 * \brief test module's init function.
 *
 *  \details Test all possible cases for init.
 */
void test_optee_mbx_init(void)
{
    int status;
    fwk_id_t init_id;

    init_id = FWK_ID_MODULE(FWK_MODULE_IDX_OPTEE_MBX);

    /* Element count can't be null */
    status = mbx_init(init_id, 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Fail to allocate module cxt */
    fwk_mm_calloc_ExpectAndReturn(
        1,
        sizeof(struct mbx_device_ctx),
        NULL);
    status = mbx_init(init_id, FAKE_SCMI_SERVICE_IDX_COUNT, NULL);
    TEST_ASSERT_EQUAL(FWK_E_NOMEM, status);

    /* Allocate module context */
    fwk_mm_calloc_ExpectAndReturn(
        FAKE_SCMI_SERVICE_IDX_COUNT,
        sizeof(struct mbx_device_ctx),
        fake_optee_mbx_ctx);
    status = mbx_init(init_id, FAKE_SCMI_SERVICE_IDX_COUNT, NULL);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(mbx_ctx.device_ctx_table, fake_optee_mbx_ctx);
}

/*!
 * \brief test module's element_init function.
 *
 *  \details Test all possible cases for element_init.
 */
void test_optee_mbx_device_init(void)
{
    int status;
    fwk_id_t device_id;
    struct mod_optee_mbx_channel_config *device_config;

    /* Set fake config data */
    device_config =
        (struct mod_optee_mbx_channel_config *)mbx_element_table[FAKE_SCMI_SERVICE_IDX_OSPM_0]
            .data;

    /* Successfully init element */
    device_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(device_id, 0);
    mutex_init_ExpectAnyArgs();
    status = mbx_device_init(device_id, 0, device_config);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's bind fucntion for round 0.
 *
 *  \details Test bind for round 0 and others except for round 1 when it needs
 *  additional information from bind_request of other modules.
 */
void test_optee_mbx_bind_round0(void)
{
    int status;
    fwk_id_t bind_id;
    unsigned int i;

    /*
     * Bind module always succeed because there is nothing to do ...
     * Take the opportunity to test all possible rounds in one shot because
     * it's a nop.
     */
    bind_id = FWK_ID_MODULE(FWK_MODULE_IDX_OPTEE_MBX);
    for (i = 0; i <= FWK_MODULE_BIND_ROUND_MAX; i++) {
        fwk_id_is_type_ExpectAndReturn(bind_id, FWK_ID_TYPE_MODULE, true);
        status = mbx_bind(bind_id, i);
        TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    }

    /*
     * Similarly, bind something than module at any round except round 1 always
     * succeed !
     */
    bind_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX,
                                 FAKE_SCMI_SERVICE_IDX_OSPM_0, 0);
    fwk_id_is_type_ExpectAndReturn(bind_id, FWK_ID_TYPE_MODULE, false);
    status = mbx_bind(bind_id, 0);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    bind_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX,
                                 FAKE_SCMI_SERVICE_IDX_OSPM_0, 0);
    fwk_id_is_type_ExpectAndReturn(bind_id, FWK_ID_TYPE_MODULE, false);
    status = mbx_bind(bind_id, FWK_MODULE_BIND_ROUND_MAX+1);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's bind_request function.
 *
 *  \details Test all possible cases for bind_request function.
 */
void test_optee_mbx_bind_request(void)
{
    int status;
    fwk_id_t source_id;
    fwk_id_t target_id;
    fwk_id_t api_id;
    void *test_driver_input_api;

    source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MBX, 0));

    /* Bind to something else than an element always fails */
    target_id = FWK_ID_MODULE(FWK_MODULE_IDX_OPTEE_MBX);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, false);
    status = mbx_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);

    /* Bind to an out of range api i.e. != 0 */
    target_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MBX, 1));
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, 1);
    status = mbx_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Bind to an out of range element */
    target_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_CHANNEL_DEVICE_IDX_COUNT);
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MBX, 0));
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_COUNT);
    status = mbx_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Bind to unknown source module */
    target_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_id_get_module_idx_ExpectAndReturn(source_id, FWK_MODULE_IDX_SCMI);
    status = mbx_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_PANIC, status);

    /* Bind to optee msg source module */
    source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_id_get_module_idx_ExpectAndReturn(source_id, FWK_MODULE_IDX_OPTEE_SMT);
    status = mbx_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&mbx_smt_api, test_driver_input_api);

    /* Bind to shared msg source module */
    source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_id_get_module_idx_ExpectAndReturn(source_id, FWK_MODULE_IDX_MSG_SMT);
    status = mbx_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&mbx_shm_api, test_driver_input_api);
}

/*!
 * \brief test module's bind function for element during round 1.
 *
 *  \details Test bind of elements for round 1.
 */
void test_optee_mbx_bind_round1(void)
{
    int status;
    fwk_id_t element_id;
    struct mbx_device_ctx *device_ctx;
    device_ctx =  &mbx_ctx.device_ctx_table[FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0];

    /* Fail to bind at round 1 */
    element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(element_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_get_element_idx_ExpectAndReturn(element_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_module_bind_ExpectAndReturn(
        device_ctx->config->driver_id,
        device_ctx->config->driver_api_id,
        &device_ctx->shmem_api,
        FWK_E_PARAM);
    status = mbx_bind(element_id, 1);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /*  Succeed to bind at round 1 */
    element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(element_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_get_element_idx_ExpectAndReturn(element_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_module_bind_ExpectAndReturn(
        device_ctx->config->driver_id,
        device_ctx->config->driver_api_id,
        &device_ctx->shmem_api,
        FWK_SUCCESS);
    status = mbx_bind(element_id, 1);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(device_ctx->shmem_id.value, device_ctx->config->driver_id.value);
}

/*!
 * \brief test module's start function.
 *
 *  \details Test all cases for start function.
 */
void test_optee_mbx_start(void)
{
    int status;
    fwk_id_t module_id;

    /* Start always succeed */
    module_id = FWK_ID_MODULE(FWK_MODULE_IDX_OPTEE_MBX);
    status = mbx_start(module_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test optee_mbx_get_devices_count function.
 *
 *  \details Test all cases for optee_mbx_get_devices_count function.
 */
void test_optee_mbx_get_devices_count(void)
{
    int status;

    status = optee_mbx_get_devices_count();
    TEST_ASSERT_EQUAL(FAKE_SCMI_CHANNEL_DEVICE_IDX_COUNT, status);
}

/*!
 * \brief test optee_mbx_get_device function.
 *
 *  \details Test all cases for optee_mbx_get_device function.
 */
void test_optee_mbx_get_device(void)
{
    fwk_id_t device_id, returned_id;
    unsigned int id;

    /* Fail to get out of range index */
    id = FAKE_SCMI_CHANNEL_DEVICE_IDX_COUNT;
    device_id = optee_mbx_get_device(id);
    returned_id = (fwk_id_t)FWK_ID_NONE_INIT;
    TEST_ASSERT_EQUAL(returned_id.value, device_id.value);

    /* Succeed to get a device */
    id = FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0;
    device_id = optee_mbx_get_device(id);
    returned_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, id);
    TEST_ASSERT_EQUAL(returned_id.value, device_id.value);
}

static int optee_signal_message(fwk_id_t channel_id)
{
    return FWK_SUCCESS;
}

static struct mod_optee_smt_driver_input_api optee_smt_input_api = {
    .signal_message = optee_signal_message,
};

/*!
 * \brief test optee mbx with static memory.
 *
 *  \details Test all cases for ignaling a message to a static memory.
 */
void test_optee_mbx_use_optee_smt_device(void)
{
    int status;
    fwk_id_t device_id;
    struct mbx_device_ctx *device_ctx;
    device_ctx =  &mbx_ctx.device_ctx_table[FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0];
    device_ctx->shmem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    device_ctx->shmem_api.smt = &optee_smt_input_api;

    /* Signal a message to static memory device */
    device_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(device_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_id_get_module_idx_ExpectAndReturn(device_ctx->shmem_id, FWK_MODULE_IDX_OPTEE_SMT);
    mutex_lock_ExpectAnyArgs();
    optee_mbx_signal_smt_message(device_id);

    /* Get back from static memory devoce */
    fwk_id_get_element_idx_ExpectAndReturn(device_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    mutex_unlock_ExpectAnyArgs();
    status = raise_smt_interrupt(device_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

int msg_signal_message(fwk_id_t channel_id, void *msg_in, size_t in_len, void *msg_out, size_t out_len)
{
    return FWK_SUCCESS;
}

static struct mod_msg_smt_driver_input_api msg_smt_input_api = {
    .signal_message = msg_signal_message,
};

/*!
 * \brief test optee mbx with dynamic memory.
 *
 *  \details Test all cases for signaling a message to a dynamic memory.
 */
void test_optee_mbx_use_msg_smt_device(void)
{
    int status;
    fwk_id_t device_id;
    struct mbx_device_ctx *device_ctx;
    void *msg_in = (void *)0xDEADBEEF, *msg_out = (void *)0xDEADBEEF;
    size_t in_len = 55, out_len = 66;

    device_ctx =  &mbx_ctx.device_ctx_table[FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0];
    device_ctx->shmem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    device_ctx->shmem_api.msg = &msg_smt_input_api;

    /* Signal a message to dynamic memory device */
    device_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(device_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    fwk_id_get_module_idx_ExpectAndReturn(device_ctx->shmem_id, FWK_MODULE_IDX_MSG_SMT);
    mutex_lock_ExpectAnyArgs();
    optee_mbx_signal_msg_message(device_id, msg_in, in_len, msg_out, &out_len);

    /* Get back from dynamic memory device */
    fwk_id_get_element_idx_ExpectAndReturn(device_id, FAKE_SCMI_CHANNEL_DEVICE_IDX_OSPM_0);
    mutex_unlock_ExpectAnyArgs();
    status = raise_shm_notification(device_id, out_len);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

}

int optee_mbx_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_optee_mbx_init);

    RUN_TEST(test_optee_mbx_device_init);

    RUN_TEST(test_optee_mbx_bind_round0);

    RUN_TEST(test_optee_mbx_bind_request);

    RUN_TEST(test_optee_mbx_bind_round1);

    RUN_TEST(test_optee_mbx_start);

    RUN_TEST(test_optee_mbx_get_devices_count);

    RUN_TEST(test_optee_mbx_get_device);

    RUN_TEST(test_optee_mbx_use_optee_smt_device);

    RUN_TEST(test_optee_mbx_use_msg_smt_device);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return optee_mbx_test_main();
}
#endif
