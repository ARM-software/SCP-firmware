/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fwk_interrupt.h"
#include "fwk_module_idx.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_interrupt.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockmgi.h>
#include <Mocksmcf_data.h>

#include UNIT_TEST_SRC

#include "smcf_interface_unit_test.h"

void setUp(void)
{
    config.sample_type = 0;
}

void tearDown(void)
{
}

struct smcf_element_ctx ctx_table[MGI_DOMAIN_COUNT];

void utest_smcf_init(void)
{
    int status;

    fwk_mm_calloc_ExpectAnyArgsAndReturn(ctx_table);

    status =
        smcf_mod_init(fwk_module_id_smcf, MGI_DOMAIN_COUNT, (void *)&config);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_element_init(void)
{
    int status;
    struct smcf_element_ctx *element_ctx;

    fwk_id_get_element_idx_ExpectAndReturn(mgi_0_id, MGI_IDX_0);

    mgi_get_num_of_monitors_ExpectAnyArgsAndReturn(MGI0_MLI_COUNT);
    mgi_set_sample_type_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mgi_number_of_data_values_per_monitor_ExpectAnyArgsAndReturn(1);
    mgi_monitor_data_width_ExpectAnyArgsAndReturn(32);
    mgi_is_data_packed_ExpectAnyArgsAndReturn(false);
    mgi_interrupt_source_unmask_Ignore();
    mgi_enable_all_monitor_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    smcf_data_set_data_address_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    fwk_id_get_element_idx_ExpectAndReturn(mgi_0_id, MGI_IDX_0);
    element_ctx = get_domain_ctx(mgi_0_id);
    fwk_interrupt_set_isr_param_ExpectAndReturn(
        config.irq,
        smcf_interrupt_handlers,
        (uintptr_t)element_ctx,
        FWK_SUCCESS);

    status = smcf_element_init(mgi_0_id, MGI0_MLI_COUNT, (const void *)&config);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_init_sample_type_error(void)
{
    int status;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(MGI_IDX_0);
    mgi_get_num_of_monitors_ExpectAnyArgsAndReturn(MGI0_MLI_COUNT);

    /* Set wrong sample type */
    config.sample_type = SMCF_SAMPLE_TYPE_COUNT;
    status = smcf_element_init(mgi_0_id, MGI0_MLI_COUNT, (const void *)&config);

    TEST_ASSERT_EQUAL(FWK_E_PANIC, status);
}

void utest_smcf_mli_config_mode_error_mli_id(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(false);
    status = smcf_mli_config_mode_validate(mli_id, 0, 0);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_mli_config_mode_error_num_mode_registers(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    mgi_get_number_of_mode_registers_ExpectAnyArgsAndReturn(0);

    status = smcf_mli_config_mode_validate(mli_id, 0, 0);

    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_smcf_mli_config_mode_error_mode_register_index(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    mgi_get_number_of_mode_registers_ExpectAnyArgsAndReturn(MODE_REG_COUNT);

    status = smcf_mli_config_mode_validate(mli_id, 0, MODE_REG_COUNT + 1);

    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_smcf_mli_config_mode_error_mode_value(void)
{
    uint32_t mode_value = UINT32_MAX;
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    mgi_get_number_of_mode_registers_ExpectAnyArgsAndReturn(MODE_REG_COUNT);
    mgi_get_number_of_bits_in_mode_registers_ExpectAnyArgsAndReturn(31);

    status = smcf_mli_config_mode_validate(mli_id, mode_value, 0);

    TEST_ASSERT_EQUAL(FWK_E_DATA, status);
}

void utest_smcf_mli_config_mode_correct_mode_value(void)
{
    uint32_t mode_value = UINT32_MAX;
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    mgi_get_number_of_mode_registers_ExpectAnyArgsAndReturn(MODE_REG_COUNT);
    mgi_get_number_of_bits_in_mode_registers_ExpectAnyArgsAndReturn(32);

    status = smcf_mli_config_mode_validate(mli_id, mode_value, 0);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_mli_config_mode_set_value_error_program(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(0);
    mgi_enable_program_mode_ExpectAnyArgsAndReturn(FWK_E_RANGE);

    status = smcf_mli_config_mode_set(mli_id, 0, 0);
    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_smcf_mli_config_mode_set_value_error_setting(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(0);
    mgi_enable_program_mode_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mgi_set_monitor_mode_ExpectAnyArgsAndReturn(FWK_E_RANGE);
    mgi_disable_program_mode_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = smcf_mli_config_mode_set(mli_id, 0, 0);
    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_smcf_mli_config_mode(void)
{
    fwk_id_t mli_id;
    int status;

    /* Mocks for smcf_mli_config_mode_validate */
    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    mgi_get_number_of_mode_registers_ExpectAnyArgsAndReturn(MODE_REG_COUNT);
    mgi_get_number_of_bits_in_mode_registers_ExpectAnyArgsAndReturn(32);

    /* Mocks for smcf_mli_config_mode_set */
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);
    fwk_id_get_sub_element_idx_ExpectAnyArgsAndReturn(0);
    mgi_enable_program_mode_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mgi_set_monitor_mode_ExpectAnyArgsAndReturn(FWK_SUCCESS);
    mgi_disable_program_mode_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = smcf_mli_config_mode(mli_id, 0, 0);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_process_bind_request_api_out_of_range(void)
{
    fwk_id_t smcf_api_id_out_of_range =
        FWK_ID_API_INIT(FWK_MODULE_IDX_SMCF, MOD_SMCF_API_IDX_COUNT);
    int status;

    fwk_id_get_api_idx_ExpectAnyArgsAndReturn(MOD_SMCF_API_IDX_COUNT);

    status = smcf_process_bind_request(
        fwk_module_id_sensor,
        fwk_module_id_smcf,
        smcf_api_id_out_of_range,
        NULL);

    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_smcf_process_bind_request(void)
{
    fwk_id_t smcf_api_id =
        FWK_ID_API_INIT(FWK_MODULE_IDX_SMCF, MOD_SMCF_API_IDX_DATA);
    const struct smcf_data_api *data_api_ptr = NULL;
    int status;

    fwk_id_get_api_idx_ExpectAnyArgsAndReturn(MOD_SMCF_API_IDX_DATA);

    status = smcf_process_bind_request(
        fwk_module_id_sensor,
        fwk_module_id_smcf,
        smcf_api_id,
        (const void **)&data_api_ptr);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_NOT_NULL(data_api_ptr);
    TEST_ASSERT_EQUAL(&data_api, data_api_ptr);
    TEST_ASSERT_EQUAL(
        &data_api.start_data_sampling, &data_api_ptr->start_data_sampling);
    TEST_ASSERT_EQUAL(&data_api.get_data, &data_api_ptr->get_data);
}

void utest_smcf_mgi_data_sample_bad_id(void)
{
    static const fwk_id_t mgi_bad_id;
    int status;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(MGI_DOMAIN_COUNT);

    status = smcf_start_data_sample(mgi_bad_id);
    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_mgi_data_sample(void)
{
    static const fwk_id_t mgi_id;
    int status;

    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(MGI_IDX_0);
    mgi_enable_sample_ExpectAnyArgs();

    status = smcf_start_data_sample(mgi_id);
    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_mgi_data_zero_buffer_size(void)
{
    struct mod_smcf_buffer tag_buffer;
    struct mod_smcf_buffer data_buffer = {
        .size = 0,
        .ptr = NULL,
    };
    static const fwk_id_t mgi_id;
    int status;

    status = smcf_get_element_data(mgi_id, data_buffer, tag_buffer);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_mgi_data_null_buffer(void)
{
    struct mod_smcf_buffer tag_buffer;
    struct mod_smcf_buffer data_buffer = {
        .size = 4,
        .ptr = NULL,
    };
    static const fwk_id_t mgi_id;
    int status;

    status = smcf_get_element_data(mgi_id, data_buffer, tag_buffer);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_mgi_data_get_id_not_sub_element(void)
{
    uint32_t array[4];
    struct mod_smcf_buffer tag_buffer;
    struct mod_smcf_buffer data_buffer = {
        .size = 4,
        .ptr = array,
    };
    static const fwk_id_t mgi_bad_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(false);

    status = smcf_get_element_data(mgi_bad_id, data_buffer, tag_buffer);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_mgi_data_get_bad_id(void)
{
    uint32_t array[4];
    struct mod_smcf_buffer tag_buffer;
    struct mod_smcf_buffer data_buffer = {
        .size = 4,
        .ptr = array,
    };
    static const fwk_id_t mgi_bad_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(MGI_DOMAIN_COUNT);

    status = smcf_get_element_data(mgi_bad_id, data_buffer, tag_buffer);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_interrupt_handlers_no_source_trigged(void)
{
    struct smcf_element_ctx mgi_ctx;

    mgi_is_the_source_triggered_the_interrupt_IgnoreAndReturn(false);

    smcf_interrupt_handlers((uintptr_t)&mgi_ctx);
}

void utest_smcf_interrupt_handlers_monitor_enable_request_complete_event(void)
{
    struct smcf_element_ctx mgi_ctx;
    uint32_t interrupt_source;

    for (interrupt_source = 0; interrupt_source < SMCF_MGI_IRQ_SOURCE_MAX;
         interrupt_source++) {
        if (interrupt_source == SMCF_MGI_IRQ_SOURCE_MON_EN) {
            mgi_is_the_source_triggered_the_interrupt_ExpectAndReturn(
                mgi_ctx.mgi, interrupt_source, true);
            mgi_interrupt_source_clear_Expect(mgi_ctx.mgi, interrupt_source);
        } else {
            mgi_is_the_source_triggered_the_interrupt_ExpectAndReturn(
                mgi_ctx.mgi, interrupt_source, false);
        }
    }

    smcf_interrupt_handlers((uintptr_t)&mgi_ctx);
}

void utest_smcf_interrupt_handlers_sample_data_set_complete_event(void)
{
    struct smcf_element_ctx mgi_ctx;
    uint32_t interrupt_source;

    for (interrupt_source = 0; interrupt_source < SMCF_MGI_IRQ_SOURCE_MAX;
         interrupt_source++) {
        if (interrupt_source == SMCF_MGI_IRQ_SOURCE_SMP_CMP) {
            mgi_is_the_source_triggered_the_interrupt_ExpectAndReturn(
                mgi_ctx.mgi, interrupt_source, true);
            mgi_interrupt_source_clear_Expect(mgi_ctx.mgi, interrupt_source);
        } else {
            mgi_is_the_source_triggered_the_interrupt_ExpectAndReturn(
                mgi_ctx.mgi, interrupt_source, false);
        }
    }

    smcf_interrupt_handlers((uintptr_t)&mgi_ctx);
}

void utest_smcf_process_bind_request_data_api(void)
{
    struct smcf_data_api *api;
    fwk_id_t source_id;
    fwk_id_t target_id;
    fwk_id_t api_id =
        FWK_ID_API_INIT(FWK_MODULE_IDX_SMCF, MOD_SMCF_API_IDX_DATA);
    int status;

    fwk_id_get_api_idx_ExpectAnyArgsAndReturn(MOD_SMCF_API_IDX_DATA);

    status = smcf_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL_PTR(&data_api, api);
}

void utest_smcf_process_bind_request_control_api(void)
{
    struct smcf_control_api *api;
    fwk_id_t source_id;
    fwk_id_t target_id;
    fwk_id_t api_id =
        FWK_ID_API_INIT(FWK_MODULE_IDX_SMCF, MOD_SMCF_API_IDX_CONTROL);
    int status;

    fwk_id_get_api_idx_ExpectAnyArgsAndReturn(MOD_SMCF_API_IDX_CONTROL);

    status = smcf_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL_PTR(&control_api, api);
}

void utest_smcf_process_bind_request_interrupt_api(void)
{
    struct smcf_mgi_interrupt_api *api;
    fwk_id_t source_id;
    fwk_id_t target_id;
    fwk_id_t api_id =
        FWK_ID_API_INIT(FWK_MODULE_IDX_SMCF, MOD_SMCF_API_IDX_INTERRUPT);
    int status;

    fwk_id_get_api_idx_ExpectAnyArgsAndReturn(MOD_SMCF_API_IDX_INTERRUPT);
    fwk_id_is_type_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAnyArgsAndReturn(0);

    status = smcf_process_bind_request(
        source_id, target_id, api_id, (const void **)&api);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
    TEST_ASSERT_EQUAL_PTR(&smcf_interrupt_api, api);
}

void utest_smcf_mli_enable_error_mli_id(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(false);
    status = smcf_mli_enable(mli_id);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_mli_enable_num_of_monitor_error(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(mli_id, 0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(mli_id, 0);
    mgi_enable_monitor_ExpectAnyArgsAndReturn(FWK_E_RANGE);

    status = smcf_mli_enable(mli_id);

    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_smcf_mli_enable_success(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(mli_id, 0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(mli_id, 0);
    mgi_enable_monitor_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = smcf_mli_enable(mli_id);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

void utest_smcf_mli_disable_error_mli_id(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(false);
    status = smcf_mli_disable(mli_id);

    TEST_ASSERT_EQUAL(FWK_E_PARAM, status);
}

void utest_smcf_mli_disable_num_of_monitor_error(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(mli_id, 0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(mli_id, 0);
    mgi_disable_monitor_ExpectAnyArgsAndReturn(FWK_E_RANGE);

    status = smcf_mli_disable(mli_id);

    TEST_ASSERT_EQUAL(FWK_E_RANGE, status);
}

void utest_smcf_mli_disable_success(void)
{
    fwk_id_t mli_id;
    int status;

    fwk_module_is_valid_sub_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_get_element_idx_ExpectAndReturn(mli_id, 0);
    fwk_id_get_sub_element_idx_ExpectAndReturn(mli_id, 0);
    mgi_disable_monitor_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = smcf_mli_disable(mli_id);

    TEST_ASSERT_EQUAL(FWK_SUCCESS, status);
}

int mod_smcf_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(utest_smcf_init);
    RUN_TEST(utest_smcf_element_init);
    RUN_TEST(utest_smcf_init_sample_type_error);
    RUN_TEST(utest_smcf_mli_config_mode_error_mli_id);
    RUN_TEST(utest_smcf_mli_config_mode_error_num_mode_registers);
    RUN_TEST(utest_smcf_mli_config_mode_error_mode_register_index);
    RUN_TEST(utest_smcf_mli_config_mode_error_mode_value);
    RUN_TEST(utest_smcf_mli_config_mode_correct_mode_value);
    RUN_TEST(utest_smcf_mli_config_mode_set_value_error_program);
    RUN_TEST(utest_smcf_mli_config_mode_set_value_error_setting);
    RUN_TEST(utest_smcf_mli_config_mode);
    RUN_TEST(utest_smcf_process_bind_request_api_out_of_range);
    RUN_TEST(utest_smcf_process_bind_request);
    RUN_TEST(utest_smcf_mgi_data_sample_bad_id);
    RUN_TEST(utest_smcf_mgi_data_sample);
    RUN_TEST(utest_smcf_mgi_data_get_id_not_sub_element);
    RUN_TEST(utest_smcf_mgi_data_get_bad_id);
    RUN_TEST(utest_smcf_mgi_data_zero_buffer_size);
    RUN_TEST(utest_smcf_mgi_data_null_buffer);
    RUN_TEST(utest_smcf_interrupt_handlers_no_source_trigged);
    RUN_TEST(
        utest_smcf_interrupt_handlers_monitor_enable_request_complete_event);
    RUN_TEST(utest_smcf_interrupt_handlers_sample_data_set_complete_event);
    RUN_TEST(utest_smcf_process_bind_request_data_api);
    RUN_TEST(utest_smcf_process_bind_request_control_api);
    RUN_TEST(utest_smcf_process_bind_request_interrupt_api);
    RUN_TEST(utest_smcf_mli_enable_error_mli_id);
    RUN_TEST(utest_smcf_mli_enable_num_of_monitor_error);
    RUN_TEST(utest_smcf_mli_enable_success);
    RUN_TEST(utest_smcf_mli_disable_error_mli_id);
    RUN_TEST(utest_smcf_mli_disable_num_of_monitor_error);
    RUN_TEST(utest_smcf_mli_disable_success);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return mod_smcf_test_main();
}
#endif
