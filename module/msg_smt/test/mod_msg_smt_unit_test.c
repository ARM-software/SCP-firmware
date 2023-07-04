/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "scp_unity.h"
#include "unity.h"

#include <mod_msg_smt.h>
#include <mod_scmi.h>

#include <Mockfwk_id.h>
#include <Mockfwk_module.h>
#include <Mockfwk_mm.h>

#include UNIT_TEST_SRC
#include "config_msg_smt.h"

void setUp(void)
{
}

void tearDown(void)
{
}

struct smt_channel_ctx fake_smt_channel_ctx[FAKE_SCMI_SERVICE_IDX_COUNT];

int fake_signal_message_status = FWK_SUCCESS;

char fake_msg_in[FAKE_MAILBOX_SIZE];
size_t fake_in_len = FAKE_MAILBOX_SIZE;
char fake_msg_out[FAKE_MAILBOX_SIZE];
size_t fake_out_len = FAKE_MAILBOX_SIZE;

/*!
 * \brief test module's init function.
 *
 *  \details Test all possible cases for init.
 */
void test_msg_init_test(void)
{
    int status;
    fwk_id_t module_id;

    module_id = FWK_ID_MODULE(FWK_MODULE_IDX_MSG_SMT);

    /* Element count can't be null */
    status = msg_init(module_id, 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Fail to allocate module cxt */
    fwk_mm_calloc_ExpectAndReturn(
        1,
        sizeof(struct smt_channel_ctx),
        NULL);
    status = msg_init(module_id, FAKE_SCMI_SERVICE_IDX_COUNT, NULL);
    TEST_ASSERT_EQUAL(FWK_E_NOMEM, status);

    /* Allocate module context */
    fwk_mm_calloc_ExpectAndReturn(
        1,
        sizeof(struct smt_channel_ctx),
        fake_smt_channel_ctx);
    status = msg_init(module_id, FAKE_SCMI_SERVICE_IDX_COUNT, NULL);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's element_init function.
 *
 *  \details Test all possible cases for element_init.
 */
void test_msg_element_init_test(void)
{
    int status;
    fwk_id_t device_id;
    unsigned int sub_element_count;
    enum mod_msg_smt_channel_type saved_type;
    struct mod_msg_smt_channel_config *device_config;
    size_t saved_size;

    /* Must provide config data */
    device_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    sub_element_count = 0;
    device_config = NULL;
    status = msg_channel_init(device_id, sub_element_count, device_config);
    TEST_ASSERT_EQUAL(FWK_E_DATA, status);

    /* Set fake config data */
    device_config =
        (struct mod_msg_smt_channel_config *)smt_element_table[FAKE_SCMI_SERVICE_IDX_OSPM_0]
            .data;

    /* Save current type */
    saved_type = device_config->type;

    /* Wrong type */
    /* Set invalid type */
    device_config->type = MOD_MSG_SMT_CHANNEL_TYPE_COUNT;
    status = msg_channel_init(device_id, sub_element_count, device_config);
    TEST_ASSERT_EQUAL(FWK_E_DATA, status);

    /* Restore valid type */
    device_config->type = saved_type;

    /* Save current size */
    saved_size = device_config->mailbox_size;

    /* Set invalid mailbox size */
    device_config->mailbox_size = 0;
    status = msg_channel_init(device_id, sub_element_count, device_config);
    TEST_ASSERT_EQUAL(FWK_E_DATA, status);

    /* Restore valid size */
    device_config->mailbox_size = saved_size;

    /* Invalide device id (out of range) */
    device_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_COUNT);
    fwk_id_get_element_idx_ExpectAndReturn(device_id, 1);
    status = msg_channel_init(device_id, sub_element_count, device_config);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Successfully init module */
    device_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(device_id, 0);
    status = msg_channel_init(device_id, sub_element_count, device_config);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's bind fucntion for round 0.
 *
 *  \details Test bind for round 0 and others except for round 1 when it needs
 *  additional information from bind_request of other modules.
 */
void test_msg_bind_test_round0(void)
{
    int status;
    unsigned int i;
    fwk_id_t module_id;
    struct smt_channel_ctx *channel_ctx;

    /*
     * Bind module always succeed because there is nothing to do ...
     * Take the opportunity to test all possible rounds in one shot because
     * it's a nop,
     */
    module_id = FWK_ID_MODULE(FWK_MODULE_IDX_MSG_SMT);
    for (i = 0; i <= FWK_MODULE_BIND_ROUND_MAX; i++) {
        fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, true);
        status = msg_bind(module_id, i);
        TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    }

    /* Then bind something else than an element always fails */
    module_id = FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0, 0);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, false);
    status = msg_bind(module_id, 0);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Bind an element out of range */
    module_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_COUNT);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(module_id, FAKE_SCMI_SERVICE_IDX_COUNT);
    status = msg_bind(module_id, 0);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Fail to bind an element round 0 */
    module_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(module_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_module_bind_ExpectAndReturn(
        channel_ctx->config->driver_id,
        channel_ctx->config->driver_api_id,
        &channel_ctx->driver_api,
        FWK_E_PARAM);
    status = msg_bind(module_id, 0);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Bind an element round 0 */
    module_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(module_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_module_bind_ExpectAndReturn(
        channel_ctx->config->driver_id,
        channel_ctx->config->driver_api_id,
        &channel_ctx->driver_api,
        FWK_SUCCESS);
    status = msg_bind(module_id, 0);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's bind_request function.
 *
 *  \details Test all possible cases for bind_request function
 */
void test_msg_bind_request_test(void)
{
    int status;
    fwk_id_t source_id;
    fwk_id_t target_id;
    fwk_id_t api_id;
    void *test_driver_input_api;
    struct smt_channel_ctx *channel_ctx;

    source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT, MOD_MSG_SMT_API_IDX_COUNT));

    /* Bind to something else than an element always fails */
    target_id = FWK_ID_MODULE(FWK_MODULE_IDX_MSG_SMT);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, false);
    status = msg_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);

    /* Bind to an out of range element */
    target_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_COUNT);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_COUNT);
    status = msg_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);

    /* Bind to an out of range interface */
    target_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_MSG_SMT_API_IDX_COUNT);
    status = msg_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Bind to scmi interface */
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT, MOD_MSG_SMT_API_IDX_SCMI_TRANSPORT));
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_MSG_SMT_API_IDX_SCMI_TRANSPORT);
    status = msg_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&smt_mod_scmi_to_transport_api, test_driver_input_api);

    /* Bind with wrong source id (has been set during bind r0) */
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT, MOD_MSG_SMT_API_IDX_DRIVER_INPUT));
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_MSG_SMT_API_IDX_DRIVER_INPUT);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_id_get_module_idx_ExpectAndReturn(channel_ctx->driver_id, FWK_MODULE_IDX_OPTEE_MBX);
    fwk_id_get_module_idx_ExpectAndReturn(source_id, FWK_MODULE_IDX_SCMI);
    status = msg_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);

    /* Bind to input driver interface */
    source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT, MOD_MSG_SMT_API_IDX_DRIVER_INPUT));
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_MSG_SMT_API_IDX_DRIVER_INPUT);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_id_get_module_idx_ExpectAndReturn(channel_ctx->driver_id, FWK_MODULE_IDX_OPTEE_MBX);
    fwk_id_get_module_idx_ExpectAndReturn(source_id, FWK_MODULE_IDX_OPTEE_MBX);
    fwk_id_get_element_idx_ExpectAndReturn(channel_ctx->driver_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(source_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&driver_input_api, test_driver_input_api);
}

/*!
 * \brief test module's bind function for element during round 1.
 *
 *  \details Test bind of elements for round 1.
 */
void test_msg_bind_test_round1(void)
{
    int status;
    fwk_id_t module_id;
    struct smt_channel_ctx *channel_ctx;

    /* Fail to bind an element round 1 */
    module_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(module_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_module_bind_ExpectAndReturn(
        channel_ctx->scmi_service_id,
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_TRANSPORT),
        &channel_ctx->scmi_api,
        FWK_E_PARAM);
    status = msg_bind(module_id, 1);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Bind an element round 1 */
    module_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(module_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_module_bind_ExpectAndReturn(
        channel_ctx->scmi_service_id,
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_TRANSPORT),
        &channel_ctx->scmi_api,
        FWK_SUCCESS);
    status = msg_bind(module_id, 1);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's start function.
 *
 *  \details Test all cases for start function.
 */
void test_msg_start(void)
{
    int status;
    fwk_id_t module_id;

    /* Start always succeed */
    module_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_start(module_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test early access to signal_message.
 *
 *  \details Test cases for signaling a message before mailbox is ready.
 */
void test_msg_signal_message_early(void)
{
    int status;
    fwk_id_t channel_id;
    char msg_in[FAKE_MAILBOX_SIZE];
    size_t in_len = FAKE_MAILBOX_SIZE;
    char msg_out[FAKE_MAILBOX_SIZE];
    size_t out_len = FAKE_MAILBOX_SIZE;

   /* Mailbox not ready, discard message silently ... */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_signal_message(channel_id, (void *)msg_in, in_len, (void *)msg_out, out_len);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

static int fake_signal_message(fwk_id_t service_id)
{
    return fake_signal_message_status;
}

struct mod_scmi_from_transport_api fake_scmi_api = {
    .signal_error = NULL, /* Not used */
    .signal_message = fake_signal_message,
};

void test_reset_fake_msg(void)
{
    int i;

    for (i = 0; i < FAKE_MAILBOX_SIZE; i++) {
        fake_msg_in[i] = 0xAA;
        fake_msg_out[i] = 0xCC;
    }
}

/*!
 * \brief test all cases of signal_message.
 *
 *  \details Test all remaining cases for signaling a message.
 */
void test_msg_signal_message(void)
{
    int status;
    fwk_id_t channel_id;
    struct smt_channel_ctx *channel_ctx;

    enum mod_msg_smt_channel_type saved_type;
    struct mod_msg_smt_channel_config *device_config;

    /* Init channel context and buffers */
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    channel_ctx->scmi_api = &fake_scmi_api;
    test_reset_fake_msg();

    /* Propagate scmi error return */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fake_signal_message_status = FWK_E_PARAM;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_signal_message(channel_id, (void *)fake_msg_in, fake_in_len, (void *)fake_msg_out, fake_out_len);
    TEST_ASSERT_EQUAL(FWK_E_HANDLER, status);

    device_config =
        (struct mod_msg_smt_channel_config *)smt_element_table[FAKE_SCMI_SERVICE_IDX_OSPM_0]
            .data;

    /* Save current type */
    saved_type = device_config->type;

    /* Set invalid channel type */
    device_config->type = MOD_MSG_SMT_CHANNEL_TYPE_COUNT;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_signal_message(channel_id, (void *)fake_msg_in, fake_in_len, (void *)fake_msg_out, fake_out_len);
    TEST_ASSERT_EQUAL(FWK_E_INIT, status);

    /* Restore valid type */
    device_config->type = saved_type;

    /* Set completer channel type */
    device_config->type = MOD_MSG_SMT_CHANNEL_TYPE_COMPLETER;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_signal_message(channel_id, (void *)fake_msg_in, fake_in_len, (void *)fake_msg_out, fake_out_len);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* Restore valid channel type */
    device_config->type = saved_type;
    /* Clear lock */
    channel_ctx->locked = false;

    /* Propagate scmi error return */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    channel_ctx->scmi_api = &fake_scmi_api;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_signal_message(channel_id, (void *)fake_msg_in, fake_in_len, (void *)fake_msg_out, fake_out_len);
    TEST_ASSERT_EQUAL(FWK_E_HANDLER, status);

    /* Signal message to scmi */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fake_signal_message_status = FWK_SUCCESS;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_signal_message(channel_id, (void *)fake_msg_in, fake_in_len, (void *)fake_msg_out, fake_out_len);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* Try to re signal message to scmi while previous one is pending */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_signal_message(channel_id, (void *)fake_msg_in, fake_in_len, (void *)fake_msg_out, fake_out_len);
    TEST_ASSERT_EQUAL(FWK_E_STATE, status);
}

/*!
 * \brief test get_secure function.
 *
 *  \details Test all cases for get_secure function.
 */
void test_smt_get_secure(void)
{
    int status;
    bool fake_secure;
    fwk_id_t channel_id;

    /* Must prodvide a secure argument */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_secure(channel_id, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Get always false secure state */
    status = smt_get_secure(channel_id, &fake_secure);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(0, fake_secure);
}

/*!
 * \brief test get_max_payload_size function.
 *
 *  \details Test all cases for get_max_payload_size function.
 */
void test_smt_get_max_payload_size(void)
{
    int status;
    size_t fake_size;
    fwk_id_t channel_id;
    struct smt_channel_ctx *channel_ctx;

    /* Must prodvide a size argument */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_max_payload_size(channel_id, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];

    /* Get max payload size */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_max_payload_size(channel_id, &fake_size);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(fake_size, channel_ctx->max_payload_size);
}

/*!
 * \brief test early access to get_message_header function.
 *
 *  \details Test all cases for get_message_header function before
 *  a message has been received.
 */
void test_smt_get_message_header_early(void)
{
    int status;
    uint32_t fake_header;
    fwk_id_t channel_id;

    /* Null header error */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_message_header(channel_id, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* No message available yet */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_message_header(channel_id, &fake_header);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);
}

/*!
 * \brief test get_message_header function.
 *
 *  \details Test all other cases for get_message_header function.
 */
void test_smt_get_message_header(void)
{
    int status;
    uint32_t fake_header;
    fwk_id_t channel_id;

    /* Get message header */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_message_header(channel_id, &fake_header);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(*((uint32_t *)fake_msg_in), fake_header);
}

/*!
 * \brief test early access to get_payload function.
 *
 *  \details Test all cases for get_payload function before
 *  a message has been received.
 */
void test_smt_get_payload_early(void)
{
    int status;
    const void *fake_payload;
    size_t fake_size;
    fwk_id_t channel_id;

    /* Null payload error */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_payload(channel_id, NULL, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* No message available yet */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_payload(channel_id, &fake_payload, &fake_size);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);
}

/*!
 * \brief test get_payload function.
 *
 *  \details Test all remaining cases for get_payload function.
 */
void test_smt_get_payload(void)
{
    int status;
    const void *fake_payload;
    size_t fake_size;
    fwk_id_t channel_id;

    /* Get Payload */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_payload(channel_id, &fake_payload, &fake_size);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(fake_in_len - sizeof(uint32_t), fake_size);
    TEST_ASSERT_EQUAL(&fake_msg_in[sizeof(uint32_t)], fake_payload);

    /* Get Payload without size*/
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_payload(channel_id, &fake_payload, NULL);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&fake_msg_in[sizeof(uint32_t)], fake_payload);
}

/*!
 * \brief test write_payload function.
 *
 *  \details Test all cases for smt_write_payload function.
 */
void test_smt_write_payload(void)
{
    int i, status;
    char fake_payload[FAKE_MAILBOX_SIZE];
    size_t fake_offset, fake_size;
    struct smt_channel_ctx *channel_ctx;
    fwk_id_t channel_id;

    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];

    /* Null Payload */
    fake_offset = 0;
    fake_size = channel_ctx->max_payload_size + 1;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status =  smt_write_payload(channel_id, fake_offset, NULL, fake_size);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Out of range offset: max_payload_size = mailbox_size - header */
    fake_offset = channel_ctx->max_payload_size;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, fake_offset, fake_payload, fake_size);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Out of range size: max_payload_size = mailbox_size - header */
    fake_offset = 0;
    fake_size = channel_ctx->max_payload_size + 1;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, fake_offset, fake_payload, fake_size);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Out of range offset + size */
    fake_offset = 1;
    fake_size = channel_ctx->max_payload_size;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, fake_offset, fake_payload, fake_size);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Init payload buffer */
    for (i = 0; i < FAKE_MAILBOX_SIZE; i++) {
        fake_payload[i] = i;
    }

    /* Fill payload */
    test_reset_fake_msg();
    fake_offset = 0;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, fake_offset, fake_payload, fake_size);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(0, fake_msg_out[sizeof(uint32_t) + fake_offset]);
    TEST_ASSERT_EQUAL(fake_size-1, fake_msg_out[sizeof(uint32_t) + fake_offset + fake_size - 1]);

    /* Fill payload at offset */
    test_reset_fake_msg();
    fake_offset = 17;
    fake_size -= fake_offset;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, fake_offset, fake_payload, fake_size);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(0, fake_msg_out[sizeof(uint32_t) + fake_offset]);
    TEST_ASSERT_EQUAL(fake_size-1, fake_msg_out[sizeof(uint32_t) + fake_offset + fake_size - 1]);
}

static int fake_raise_shm_notification(fwk_id_t channel_id, size_t size)
{
    return FWK_SUCCESS;
}

struct mod_msg_smt_driver_ouput_api fake_mbx_shm_api = {
    .raise_notification = fake_raise_shm_notification,
};

/*!
 * \brief test all cases of respond.
 *
 *  \details Test all cases for responding to a message.
 */
void test_smt_respond(void)
{
    int i, status;
    char fake_payload[FAKE_MAILBOX_SIZE];
    size_t fake_size;
    struct smt_channel_ctx *channel_ctx;
    fwk_id_t channel_id;

    /* Init scmi api */
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    channel_ctx->driver_api = &fake_mbx_shm_api;

    /* Respond with null Payload */
    fake_size = channel_ctx->max_payload_size;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status =  smt_respond(channel_id, NULL, fake_size);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* No message available to write payload */
    fake_size = channel_ctx->max_payload_size;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, 0, fake_payload, fake_size);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);

    /* Signal a new message to scmi */
    fake_signal_message_status = FWK_SUCCESS;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_signal_message(channel_id, (void *)fake_msg_in, fake_in_len, (void *)fake_msg_out, fake_out_len);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

     /* Init payload buffer */
    for (i = 0; i < FAKE_MAILBOX_SIZE; i++) {
        fake_payload[i] = i;
    }

   /* Respond with payload */
    test_reset_fake_msg();
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_respond(channel_id, fake_payload, fake_size);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(0, fake_msg_out[sizeof(uint32_t)]);
    TEST_ASSERT_EQUAL(fake_size-1, fake_msg_out[sizeof(uint32_t) + fake_size - 1]);
}

/*!
 * \brief test all cases of signal_message.
 *
 *  \details Test all cases for signaling a message.
 */
void test_smt_transmit(void)
{
    int i, status;
    char fake_payload[FAKE_MAILBOX_SIZE];
    size_t fake_size;
    struct smt_channel_ctx *channel_ctx;
    fwk_id_t channel_id;

    /* Init scmi api */
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    channel_ctx->driver_api = &fake_mbx_shm_api;

    /* Null Payload */
    fake_size = channel_ctx->max_payload_size;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_MSG_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status =  smt_transmit(channel_id, 0xAA, NULL, fake_size, true);
    TEST_ASSERT_EQUAL(FWK_E_DATA, status);

    /* Init payload buffer */
    for (i = 0; i < FAKE_MAILBOX_SIZE; i++) {
        fake_payload[i] = i;
    }

    /* No message available, return silently */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status =  smt_transmit(channel_id, 0xAA, fake_payload, fake_size, true);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    /* Signal a new message to scmi */
    fake_signal_message_status = FWK_SUCCESS;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = msg_signal_message(channel_id, (void *)fake_msg_in, fake_in_len, (void *)fake_msg_out, fake_out_len);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    /* Transmit async message */
    test_reset_fake_msg();
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status =  smt_transmit(channel_id, 0xAA, fake_payload, fake_size, true);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(0, fake_msg_out[sizeof(uint32_t)]);
    TEST_ASSERT_EQUAL(fake_size-1, fake_msg_out[sizeof(uint32_t) + fake_size - 1]);
}

int msg_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_msg_init_test);

    RUN_TEST(test_msg_signal_message_early);

    RUN_TEST(test_msg_element_init_test);

    RUN_TEST(test_msg_bind_test_round0);

    RUN_TEST(test_msg_bind_request_test);

    RUN_TEST(test_msg_bind_test_round1);

    RUN_TEST(test_msg_start);

    RUN_TEST(test_smt_get_message_header_early);

    RUN_TEST(test_smt_get_payload_early);

    RUN_TEST(test_msg_signal_message);

    RUN_TEST(test_smt_get_secure);

    RUN_TEST(test_smt_get_max_payload_size);

    RUN_TEST(test_smt_get_message_header);

    RUN_TEST(test_smt_get_payload);

    RUN_TEST(test_smt_write_payload);

    RUN_TEST(test_smt_respond);

    RUN_TEST(test_smt_transmit);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return msg_test_main();
}
#endif
