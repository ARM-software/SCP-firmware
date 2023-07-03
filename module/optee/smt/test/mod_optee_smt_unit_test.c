/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * mod_optee_smt_unit_test.c is comming from the template file
 * This is the minimum file setup you need to get started with Unit Testing
 */

#include "scp_unity.h"
#include "unity.h"

#include <mod_optee_smt.h>
#include <mod_scmi.h>

#include <Mockfwk_id.h>
#include <Mockfwk_module.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_string.h>
#include <Mockmod_optee_smt_extra.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC
#include "config_optee_smt.h"

void setUp(void)
{
}

void tearDown(void)
{
}

struct smt_channel_ctx fake_optee_smt_ctx[FAKE_SCMI_SERVICE_IDX_COUNT];

char mailbox_virt_buffer[FAKE_MAILBOX_SIZE];
char channel_in_buffer[FAKE_MAILBOX_SIZE];
char channel_out_buffer[FAKE_MAILBOX_SIZE];

int fake_signal_message_status = FWK_SUCCESS;

/*!
 * \brief test module's init function.
 *
 *  \details Test all possible cases for init.
 */
void test_optee_smt_init(void)
{
    int status;
    fwk_id_t module_id;

    module_id = FWK_ID_MODULE(FWK_MODULE_IDX_OPTEE_SMT);

    /* Element count can't be null */
    status = mailbox_init(module_id, 0, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Fail to allocate module cxt */
    fwk_mm_calloc_ExpectAndReturn(
        FAKE_SCMI_SERVICE_IDX_COUNT,
        sizeof(struct smt_channel_ctx),
        NULL);
    status = mailbox_init(module_id, FAKE_SCMI_SERVICE_IDX_COUNT, NULL);
    TEST_ASSERT_EQUAL(FWK_E_NOMEM, status);

    /* Allocate module context */
    fwk_mm_calloc_ExpectAndReturn(
        1,
        sizeof(struct smt_channel_ctx),
        fake_optee_smt_ctx);
    status = mailbox_init(module_id, FAKE_SCMI_SERVICE_IDX_COUNT, NULL);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's element_init function.
 *
 *  \details Test all possible cases for element_init.
 */
void test_mailbox_channel_init(void)
{
    int status;
    fwk_id_t channel_id;
    unsigned int slot_count;
    struct mod_optee_smt_channel_config *channel_config;
    enum mod_optee_smt_channel_type saved_type;
    uintptr_t saved_address;
    size_t saved_size;

    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    slot_count = 0;
    channel_config =
        (struct mod_optee_smt_channel_config*)smt_element_table[FAKE_SCMI_SERVICE_IDX_OSPM_0]
            .data;

   /* Save current type */
    saved_type = channel_config->type;

    /* Set invalid type */
    channel_config->type = MOD_OPTEE_SMT_CHANNEL_TYPE_COUNT;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = mailbox_channel_init(channel_id, slot_count, channel_config);
    TEST_ASSERT_EQUAL(FWK_E_DATA, status);

    /* Restore valid type */
    channel_config->type = saved_type;

    /* Save current address */
    saved_address = channel_config->mailbox_address;

    /* Set invalid mailbox address */
    channel_config->mailbox_address = 0;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = mailbox_channel_init(channel_id, slot_count, channel_config);
    TEST_ASSERT_EQUAL(FWK_E_DATA, status);

    /* Restore valid address */
    channel_config->mailbox_address = saved_address;

    /* Save current size */
    saved_size = channel_config->mailbox_size;

    /* Set invalid mailbox size */
    channel_config->mailbox_size = 0;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = mailbox_channel_init(channel_id, slot_count, channel_config);
    TEST_ASSERT_EQUAL(FWK_E_DATA, status);

    /* Restore valid size */
    channel_config->mailbox_size = saved_size;

    /* Fail to get virt address */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    smt_phys_to_virt_ExpectAndReturn(channel_config->mailbox_address,
                                     channel_config->mailbox_size,
                                     channel_config->policies & MOD_SMT_POLICY_SECURE,
                                     (uintptr_t)NULL);
    status = mailbox_channel_init(channel_id, slot_count, channel_config);
    TEST_ASSERT_EQUAL(FWK_E_DATA, status);

    /* Successfully init channel */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    smt_phys_to_virt_ExpectAndReturn(channel_config->mailbox_address,
                                     channel_config->mailbox_size,
                                     channel_config->policies & MOD_SMT_POLICY_SECURE,
                                     (uintptr_t)mailbox_virt_buffer);
    fwk_mm_alloc_ExpectAndReturn(1, channel_config->mailbox_size, channel_in_buffer);
    fwk_mm_alloc_ExpectAndReturn(1, channel_config->mailbox_size, channel_out_buffer);
    status = mailbox_channel_init(channel_id, slot_count, channel_config);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's bind fucntion for round 0.
 *
 *  \details Test bind for round 0 and others except for round 1 when it needs
 *  additional information from bind_request of other modules.
 */
void test_mailbox_smt_bind_round0(void)
{
    int status;
    unsigned int i;
    fwk_id_t bind_id;
    struct smt_channel_ctx *channel_ctx;

    /*
     * Bind module always succeed because there is nothing to do ...
     * Take the opportunity to test all possible rounds in one shot because
     * it's a nop,
     */
    bind_id = FWK_ID_MODULE(FWK_MODULE_IDX_OPTEE_SMT);
    for (i = 0; i <= FWK_MODULE_BIND_ROUND_MAX; i++) {
        fwk_id_is_type_ExpectAndReturn(bind_id, FWK_ID_TYPE_MODULE, true);
        status = mailbox_smt_bind(bind_id, i);
        TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    }

    /* Fail to bind an element round 0 */
    bind_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(bind_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_get_element_idx_ExpectAndReturn(bind_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_module_bind_ExpectAndReturn(
        channel_ctx->config->driver_id,
        channel_ctx->config->driver_api_id,
        &channel_ctx->driver_api,
        FWK_E_PARAM);
    status = mailbox_smt_bind(bind_id, 0);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Succeed to bind an element round 0 */
    bind_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(bind_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_get_element_idx_ExpectAndReturn(bind_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_module_bind_ExpectAndReturn(
        channel_ctx->config->driver_id,
        channel_ctx->config->driver_api_id,
        &channel_ctx->driver_api,
        FWK_SUCCESS);
    status = mailbox_smt_bind(bind_id, 0);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's bind_request function.
 *
 *  \details Test all possible cases for bind_request function
 */
void test_mailbox_smt_process_bind_request(void)
{
    int status;
    fwk_id_t source_id;
    fwk_id_t target_id;
    fwk_id_t api_id;
    void *test_driver_input_api;
    struct smt_channel_ctx *channel_ctx;

    source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_SMT, MOD_OPTEE_SMT_API_IDX_COUNT));

    /* Bind to something else than an element always fails */
    target_id = FWK_ID_MODULE(FWK_MODULE_IDX_OPTEE_SMT);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, false);
    status = mailbox_smt_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Bind to an out of range element */
    target_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_COUNT);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_COUNT);
    status = mailbox_smt_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);

    /* Bind to an out of range interface */
    target_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_OPTEE_SMT_API_IDX_COUNT);
    status = mailbox_smt_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Bind to scmi interface */
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_SMT, MOD_OPTEE_SMT_API_IDX_SCMI_TRANSPORT));
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_OPTEE_SMT_API_IDX_SCMI_TRANSPORT);
    status = mailbox_smt_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&smt_mod_scmi_to_transport_api, test_driver_input_api);

    /* Bind with wrong source id (has been set during bind round0) */
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_SMT, MOD_OPTEE_SMT_API_IDX_DRIVER_INPUT));
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_OPTEE_SMT_API_IDX_DRIVER_INPUT);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_id_get_module_idx_ExpectAndReturn(channel_ctx->driver_id, FWK_MODULE_IDX_OPTEE_MBX);
    fwk_id_get_module_idx_ExpectAndReturn(source_id, FWK_MODULE_IDX_SCMI);
    status = mailbox_smt_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);

    /* Bind to input driver interface */
    source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_MBX, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    api_id = ((fwk_id_t) FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_SMT, MOD_OPTEE_SMT_API_IDX_DRIVER_INPUT));
    fwk_id_is_type_ExpectAndReturn(target_id, FWK_ID_TYPE_ELEMENT, true);
    fwk_id_get_element_idx_ExpectAndReturn(target_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_api_idx_ExpectAndReturn(api_id, MOD_OPTEE_SMT_API_IDX_DRIVER_INPUT);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_id_get_module_idx_ExpectAndReturn(channel_ctx->driver_id, FWK_MODULE_IDX_OPTEE_MBX);
    fwk_id_get_module_idx_ExpectAndReturn(source_id, FWK_MODULE_IDX_OPTEE_MBX);
    fwk_id_get_element_idx_ExpectAndReturn(channel_ctx->driver_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(source_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = mailbox_smt_process_bind_request(source_id, target_id, api_id, (const void **)&test_driver_input_api);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(&driver_input_api, test_driver_input_api);
}

/*!
 * \brief test module's bind function for element during round 1.
 *
 *  \details Test bind of elements for round 1.
 */
void test_mailbox_smt_bind_round1(void)
{
    int status;
    fwk_id_t element_id;
    struct smt_channel_ctx *channel_ctx;

    /* Fail to bind an element round 1 */
    element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(element_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_get_element_idx_ExpectAndReturn(element_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_module_bind_ExpectAndReturn(
        channel_ctx->scmi_service_id,
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_TRANSPORT),
        &channel_ctx->scmi_api,
        FWK_E_PARAM);
    status = mailbox_smt_bind(element_id, 1);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Succeed to bind an element at round 1 */
    element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_is_type_ExpectAndReturn(element_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_get_element_idx_ExpectAndReturn(element_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    fwk_module_bind_ExpectAndReturn(
        channel_ctx->scmi_service_id,
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_TRANSPORT),
        &channel_ctx->scmi_api,
        FWK_SUCCESS);
    status = mailbox_smt_bind(element_id, 1);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test module's start function.
 *
 *  \details Test all cases for start function.
 */
void test_mailbox_start(void)
{
    int status;
    fwk_id_t module_id;

    /* Start always succeed */
    module_id = FWK_ID_MODULE(FWK_MODULE_IDX_OPTEE_SMT);
    status = mailbox_start(module_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test get_secure function.
 *
 *  \details Test all cases for get_secure function.
 */
void test_smt_get_secure(void)
{
    int status;
    fwk_id_t channel_id;
    uint32_t saved_policies;
    bool secure;

    struct mod_optee_smt_channel_config *device_config;

    /* Fail if the pointer is null */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_secure(channel_id, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    device_config =
        (struct mod_optee_smt_channel_config *)smt_element_table[FAKE_SCMI_SERVICE_IDX_OSPM_0]
            .data;

    /* Save policies */
    saved_policies = device_config->policies;

    /* Return secure state of the channel */
    device_config->policies = MOD_SMT_POLICY_SECURE;
    secure = 0xAA;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_secure(channel_id, &secure);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(true, secure);

    /* Restore policies */
    device_config->policies = saved_policies;

    /* Return non secure state of the channel */
    secure = 0xAA;
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_secure(channel_id, &secure);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(false, secure);
}

/*!
 * \brief test get_max_payload_size function.
 *
 *  \details Test all cases for get_max_payload_size function.
 */
void test_smt_get_max_payload_size(void)
{
    int status;
    fwk_id_t channel_id;
    size_t payload_size;
    struct smt_channel_ctx *channel_ctx;

    /* Init channel context */
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];

    /* Fail if the pointer is null */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_max_payload_size(channel_id, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Return payload size */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    payload_size = 0xAAAA;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_max_payload_size(channel_id, &payload_size);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(channel_ctx->max_payload_size, payload_size);
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
    fwk_id_t channel_id;
    uint32_t header;

    /* Fail if the pointer is null */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_message_header(channel_id, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* No message available */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    header = 0x0000;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_message_header(channel_id, &header);
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
    fwk_id_t channel_id;
    uint32_t header;

    /* Return header */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    header = 0x0000;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_message_header(channel_id, &header);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(((struct mod_optee_smt_memory *)(mailbox_virt_buffer))->message_header, header);
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
    fwk_id_t channel_id;
    void *fake_payload;

    /* Fail if the pointer is null */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_payload(channel_id, NULL, NULL);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* No message available */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_payload(channel_id, (const void **)&fake_payload, NULL);
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
    fwk_id_t channel_id;
    void *fake_payload;
    size_t fake_size;

    /* Return payload pointer */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_payload(channel_id, (const void **)&fake_payload, NULL);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(((struct mod_optee_smt_memory *)(channel_in_buffer))->payload, fake_payload);

    /* Return payload pointer and size */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fake_size = 0x0000;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_get_payload(channel_id, (const void **)&fake_payload, &fake_size);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL(((struct mod_optee_smt_memory *)(channel_in_buffer))->payload, fake_payload);
    TEST_ASSERT_EQUAL(((struct mod_optee_smt_memory *)(channel_in_buffer))->length - sizeof(uint32_t), fake_size);

}

void fake_fwk_str_memcpy(void *dest, const void *src, size_t count, int iteration)
{
}

/*!
 * \brief test early access to write_payload function.
 *
 *  \details Test all remaining cases for write_payload function before
 *  a message has been received.
 */
void test_smt_write_payload_early(void)
{
    int status;
    fwk_id_t channel_id;
    char fake_payload[FAKE_MAILBOX_SIZE];
    struct smt_channel_ctx *channel_ctx;

    /* Init channel context */
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];

    /* Fail if the pointer is null */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, 0, NULL, 0);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Fail if the offset is out of range */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, channel_ctx->max_payload_size+1, fake_payload, 0);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Fail if the size is out of range */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, 0, (const void*)fake_payload, channel_ctx->max_payload_size+1);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* Fail if the offset+size is out of range */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, channel_ctx->max_payload_size, (const void*)fake_payload, channel_ctx->max_payload_size);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);

    /* No message available */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_write_payload(channel_id, 0, (const void*)fake_payload, channel_ctx->max_payload_size);
    TEST_ASSERT_EQUAL(FWK_E_ACCESS, status);
}

/*!
 * \brief test write_payload function.
 *
 *  \details Test all remaining cases for write_payload function.
 */
void test_smt_write_payload(void)
{
    int status;
    fwk_id_t channel_id;
    char fake_payload[FAKE_MAILBOX_SIZE];
    struct smt_channel_ctx *channel_ctx;

    /* Init channel context */
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];

    /* write payload */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_str_memcpy_StubWithCallback(fake_fwk_str_memcpy);
    status = smt_write_payload(channel_id, 0, (const void*)fake_payload, channel_ctx->max_payload_size);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test early access to signal_message.
 *
 *  \details Test all cases for signaling a message to a static memory.
 */
void test_smt_signal_message_early(void)
{
    int status;
    fwk_id_t channel_id;

   /* Mailbox not ready, discard message silently ... */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_signal_message(channel_id);
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

static int fake_raise_smt_interrupt(fwk_id_t channel_id)
{
    return FWK_SUCCESS;
}

struct mod_optee_smt_driver_api fake_mbx_smt_api = {
    .raise_interrupt = fake_raise_smt_interrupt,
};

void test_reset_virt_buffer(void)
{
    int i;

    for (i = 0; i < FAKE_MAILBOX_SIZE; i++) {
        mailbox_virt_buffer[i] = 0xAA;
    }
}

void test_reset_in_buffer(void)
{
    int i;

    for (i = 0; i < FAKE_MAILBOX_SIZE; i++) {
        channel_in_buffer[i] = 0;
    }
}

void test_reset_out_buffer(void)
{
    int i;

    for (i = 0; i < FAKE_MAILBOX_SIZE; i++) {
        channel_out_buffer[i] = 0;
    }
}

/*!
 * \brief test all cases of signal_message.
 *
 *  \details Test all cases for signaling a message.
 */
void test_smt_signal_message(void)
{
    int status;
    fwk_id_t channel_id;
    struct smt_channel_ctx *channel_ctx;

    enum mod_optee_smt_channel_type saved_type;
    struct mod_optee_smt_channel_config *device_config;
    struct mod_optee_smt_memory *fake_memory;

    /* Init channel context and buffers */
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    channel_ctx->scmi_api = &fake_scmi_api;
    channel_ctx->driver_api = &fake_mbx_smt_api;
    test_reset_virt_buffer();
    test_reset_in_buffer();
    test_reset_out_buffer();

    device_config =
        (struct mod_optee_smt_channel_config *)smt_element_table[FAKE_SCMI_SERVICE_IDX_OSPM_0]
            .data;

    /* Save current type */
    saved_type = device_config->type;

    /* Set invalid channel type and silently return */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    device_config->type = MOD_OPTEE_SMT_CHANNEL_TYPE_COUNT;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_signal_message(channel_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* Set completer channel type */
    device_config->type = MOD_OPTEE_SMT_CHANNEL_TYPE_COMPLETER;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_signal_message(channel_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* Try to send a new message while one is pending */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_signal_message(channel_id);
    TEST_ASSERT_EQUAL(FWK_E_STATE, status);

    /* Restore valid channel type */
    device_config->type = saved_type;
    /* Clear lock */
    channel_ctx->locked = false;

    /* Check message header correctness: length too small */
    test_reset_virt_buffer();
    test_reset_in_buffer();
    test_reset_out_buffer();
    channel_ctx->scmi_api = &fake_scmi_api;
    fake_memory = (struct mod_optee_smt_memory *)mailbox_virt_buffer;
    fake_memory->length = 0;

    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    /* smt_respond part */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_str_memcpy_StubWithCallback(fake_fwk_str_memcpy);
    status = smt_signal_message(channel_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* Check message header correctness: length too large */
    test_reset_virt_buffer();
    test_reset_in_buffer();
    test_reset_out_buffer();
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fake_memory->length = FAKE_MAILBOX_SIZE;

    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    /* smt_respond part */
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_str_memcpy_StubWithCallback(fake_fwk_str_memcpy);
    status = smt_signal_message(channel_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);

    /* Message is correct, signal upper layer which returns error */
    test_reset_virt_buffer();
    test_reset_in_buffer();
    test_reset_out_buffer();
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fake_memory->length = FAKE_MAILBOX_SIZE - sizeof(struct mod_optee_smt_memory);
    fake_signal_message_status = FWK_E_HANDLER;

    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_signal_message(channel_id);
    TEST_ASSERT_EQUAL(FWK_E_HANDLER, status);

    /* Clear lock */
    channel_ctx->locked = false;

    /* Message is correct, signal upper layer */
    test_reset_virt_buffer();
    test_reset_in_buffer();
    test_reset_out_buffer();
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fake_memory->length = FAKE_MAILBOX_SIZE - sizeof(struct mod_optee_smt_memory);
    fake_signal_message_status = FWK_SUCCESS;

    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status = smt_signal_message(channel_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

/*!
 * \brief test all cases of respond.
 *
 *  \details Test all cases for responding to a message.
 */
void test_smt_respond(void)
{
    int status;
    fwk_id_t channel_id;
    struct smt_channel_ctx *channel_ctx;
    size_t fake_size;

    /* Init channel context and buffers */
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    channel_ctx->driver_api = &fake_mbx_smt_api;
    test_reset_virt_buffer();
    test_reset_in_buffer();
    test_reset_out_buffer();

    /* Successfully foward NULL payload */
    fake_size = 0;
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fwk_str_memcpy_Expect(((struct mod_optee_smt_memory *)(channel_ctx->mailbox_va))->payload, channel_ctx->out->payload, fake_size);
    status = smt_respond(channel_id, NULL, fake_size);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

static int fake_raise_shm_interrupt(fwk_id_t channel_id)
{
    return FWK_SUCCESS;
}

struct mod_optee_smt_driver_api fake_mbx_shm_api = {
    .raise_interrupt = fake_raise_shm_interrupt,
};

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

    /* Init channel context */
    channel_ctx = &smt_ctx.channel_ctx_table[FAKE_SCMI_SERVICE_IDX_OSPM_0];
    channel_ctx->driver_api = &fake_mbx_shm_api;

    /* Null Payload */
    channel_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_OPTEE_SMT, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    fake_size = channel_ctx->max_payload_size;
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
    status = smt_signal_message(channel_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);

    /* Transmit async message */
    test_reset_virt_buffer();
    test_reset_in_buffer();
    test_reset_out_buffer();
    fwk_id_get_element_idx_ExpectAndReturn(channel_id, FAKE_SCMI_SERVICE_IDX_OSPM_0);
    status =  smt_transmit(channel_id, 0xAA, fake_payload, fake_size, true);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int optee_smt_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_optee_smt_init);

    RUN_TEST(test_smt_signal_message_early);

    RUN_TEST(test_mailbox_channel_init);

    RUN_TEST(test_mailbox_smt_bind_round0);

    RUN_TEST(test_mailbox_smt_process_bind_request);

    RUN_TEST(test_mailbox_smt_bind_round1);

    RUN_TEST(test_mailbox_start);

    RUN_TEST(test_smt_get_secure);

    RUN_TEST(test_smt_get_max_payload_size);

    RUN_TEST(test_smt_get_message_header_early);

    RUN_TEST(test_smt_get_payload_early);

    RUN_TEST(test_smt_write_payload_early);

    RUN_TEST(test_smt_signal_message);

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
    return optee_smt_test_main();
}
#endif
