/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockmod_thermal_mgmt_extra.h>

#include <internal/Mockfwk_core_internal.h>

#include <mod_sensor.h>

#include <fwk_element.h>
#include <fwk_macros.h>

#include UNIT_TEST_SRC
#include "config_thermal_mgmt.h"

void setUp(void)
{
    unsigned int dev_idx, actor_idx;
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    struct mod_thermal_mgmt_actor_ctx *actor_ctx;
    struct mod_thermal_mgmt_dev_config *config;

    /* Initialize module context */
    mod_ctx.dev_ctx_table = dev_ctx_table;
    mod_ctx.dev_ctx_count = MOD_THERMAL_MGMT_DOM_COUNT;

    /* Set default values for configuration structure */
    memcpy(
        dev_config_table,
        dev_config_default_table,
        sizeof(*dev_config_table) * MOD_THERMAL_MGMT_DOM_COUNT);

    for (dev_idx = 0; dev_idx < MOD_THERMAL_MGMT_DOM_COUNT; dev_idx++) {
        dev_ctx = &mod_ctx.dev_ctx_table[dev_idx];
        memset(dev_ctx, 0, sizeof(*dev_ctx));

        config = (struct mod_thermal_mgmt_dev_config *)
                     thermal_mgmt_domains_elem_table[dev_idx]
                         .data;

        /* Configure device context */
        dev_ctx->config = config;
        dev_ctx->id = FWK_ID_ELEMENT(FWK_MODULE_IDX_THERMAL_MGMT, dev_idx);
        dev_ctx->thermal_allocatable_power = (uint32_t)dev_ctx->config->tdp;
        dev_ctx->actor_ctx_table = &actor_ctx_table[dev_idx][0];
        dev_ctx->thermal_protection_api = &thermal_protection_api;
        dev_ctx->sensor_api = &sensor_api;

        /* Configure actors context */
        for (actor_idx = 0; actor_idx < dev_ctx->config->thermal_actors_count;
             actor_idx++) {
            actor_ctx = &dev_ctx->actor_ctx_table[actor_idx];
            memset(actor_ctx, 0, sizeof(*actor_ctx));

            actor_ctx->config = &(config->thermal_actors_table[actor_idx]);
        }
    }
}

void tearDown(void)
{
}

void test_thermal_mgmt_init(void)
{
    int status;

    /* Reset module context configuration */
    memset(&mod_ctx, 0, sizeof(mod_ctx));

    fwk_mm_calloc_ExpectAndReturn(
        MOD_THERMAL_MGMT_DOM_COUNT,
        sizeof(struct mod_thermal_mgmt_dev_ctx),
        dev_ctx_table);

    status = thermal_mgmt_init(
        fwk_module_id_thermal_mgmt, MOD_THERMAL_MGMT_DOM_COUNT, NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(mod_ctx.dev_ctx_table, dev_ctx_table);
}

void test_thermal_mgmt_dev_init_error_null_ptr(void)
{
    int status;

    status = thermal_mgmt_dev_init(fwk_module_id_thermal_mgmt, 0, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_thermal_mgmt_dev_init_error_no_config(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    int status;

    dev_config_table[0].thermal_actors_count = 0;
    dev_config_table[0].temp_protection = NULL;
    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);

    status = thermal_mgmt_dev_init(element_id, 0, &dev_config_table[0]);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_thermal_mgmt_dev_init_error_invalid_dvfs_domain(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    int status;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    fwk_mm_calloc_ExpectAnyArgsAndReturn(actor_table_domain[0]);
    fwk_module_is_valid_element_id_ExpectAnyArgsAndReturn(false);

    status = thermal_mgmt_dev_init(element_id, 0, &dev_config_table[0]);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_thermal_mgmt_dev_init_error_invalid_driver(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    int status;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    fwk_mm_calloc_ExpectAnyArgsAndReturn(actor_table_domain[0]);
    fwk_module_is_valid_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_type_is_valid_ExpectAndReturn(
        actor_table_domain[0][0].driver_id, false);

    status = thermal_mgmt_dev_init(element_id, 0, &dev_config_table[0]);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    fwk_mm_calloc_ExpectAnyArgsAndReturn(actor_table_domain[0]);
    fwk_module_is_valid_element_id_ExpectAnyArgsAndReturn(true);
    fwk_id_type_is_valid_ExpectAndReturn(
        actor_table_domain[0][0].driver_id, true);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);

    status = thermal_mgmt_dev_init(element_id, 0, &dev_config_table[0]);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_thermal_mgmt_dev_init_success(void)
{
    unsigned int status, dev_idx, actor_idx;
    fwk_id_t element_id;
    struct mod_thermal_mgmt_dev_config *config;
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;

    for (dev_idx = 0; dev_idx < MOD_THERMAL_MGMT_DOM_COUNT; dev_idx++) {
        element_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_THERMAL_MGMT, dev_idx);
        dev_ctx = &mod_ctx.dev_ctx_table[dev_idx];
        config = &dev_config_table[dev_idx];
        /* Reset module context configuration */
        memset(dev_ctx, 0, sizeof(*dev_ctx));

        fwk_id_get_element_idx_ExpectAndReturn(element_id, dev_idx);
        fwk_mm_calloc_ExpectAndReturn(
            config->thermal_actors_count,
            sizeof(struct mod_thermal_mgmt_actor_ctx),
            actor_table_domain[dev_idx]);
        for (actor_idx = 0; actor_idx < config->thermal_actors_count;
             actor_idx++) {
            fwk_module_is_valid_element_id_ExpectAnyArgsAndReturn(true);
            fwk_id_type_is_valid_ExpectAnyArgsAndReturn(true);
            fwk_id_is_equal_ExpectAnyArgsAndReturn(false);
        }

        status = thermal_mgmt_dev_init(element_id, dev_idx, config);
        TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
        TEST_ASSERT_EQUAL(
            dev_ctx->actor_ctx_table, actor_table_domain[dev_idx]);
    }
}

void test_thermal_mgmt_bind_round_1(void)
{
    int status;
    status = thermal_mgmt_bind(fwk_module_id_thermal_mgmt, 1);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_thermal_mgmt_bind_module(void)
{
    int status;

    fwk_id_is_type_ExpectAndReturn(
        fwk_module_id_thermal_mgmt, FWK_ID_TYPE_MODULE, true);

    status = thermal_mgmt_bind(fwk_module_id_thermal_mgmt, 0);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_thermal_mgmt_bind_fail(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    int status;
    unsigned int i, j;

    fwk_id_is_type_IgnoreAndReturn(false);
    fwk_id_get_element_idx_IgnoreAndReturn(0);
    fwk_id_get_module_idx_IgnoreAndReturn(FWK_MODULE_IDX_FAKE_POWER_MODEL);

    /*
     * There are 4 binds requests on the module bind function. It is going to
     * tested when each of them fail for this reason it has to be tested 4
     * times.
     */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < i; j++) {
            fwk_module_bind_ExpectAnyArgsAndReturn(FWK_SUCCESS);
        }
        fwk_module_bind_ExpectAnyArgsAndReturn(FWK_E_PARAM);

        status = thermal_mgmt_bind(element_id, 0);
        TEST_ASSERT_EQUAL(status, FWK_E_PANIC);
    }

    fwk_id_is_type_StopIgnore();
    fwk_id_get_element_idx_StopIgnore();
    fwk_id_get_module_idx_StopIgnore();
}

void test_thermal_mgmt_bind_success(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    struct mod_thermal_mgmt_actor_ctx *actor_ctx;
    int status;

    dev_ctx = &mod_ctx.dev_ctx_table[0];
    actor_ctx = &dev_ctx->actor_ctx_table[0];

    fwk_id_is_type_ExpectAndReturn(element_id, FWK_ID_TYPE_MODULE, false);
    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    fwk_module_bind_ExpectAndReturn(
        dev_ctx->config->sensor_id,
        mod_sensor_api_id_sensor,
        &dev_ctx->sensor_api,
        FWK_SUCCESS);
    fwk_module_bind_ExpectAndReturn(
        dev_ctx->config->temp_protection->driver_id,
        dev_ctx->config->temp_protection->driver_api_id,
        &dev_ctx->thermal_protection_api,
        FWK_SUCCESS);
    fwk_id_get_module_idx_ExpectAndReturn(
        dev_ctx->config->driver_api_id, FWK_MODULE_IDX_FAKE_POWER_MODEL);
    fwk_module_bind_ExpectAndReturn(
        fwk_module_id_fake_power_model,
        dev_ctx->config->driver_api_id,
        &dev_ctx->driver_api,
        FWK_SUCCESS);
    fwk_module_bind_ExpectAndReturn(
        actor_ctx->config->activity_factor->driver_id,
        actor_ctx->config->activity_factor->driver_api_id,
        &actor_ctx->activity_api,
        FWK_SUCCESS);
    status = thermal_mgmt_bind(element_id, 0);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_thermal_mgmt_process_bind_request(void)
{
    struct perf_plugins_api *api;
    int status;

    status = thermal_mgmt_process_bind_request(
        FWK_ID_NONE, FWK_ID_NONE, FWK_ID_NONE, (const void **)&api);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(api, &mod_thermal_perf_plugins_api);
}

#if THERMAL_HAS_ASYNC_SENSORS
void test_thermal_mgmt_process_event_invalid(void)
{
    int status;
    struct fwk_event event = {
        .source_id = FWK_ID_NONE,
        .target_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0),
        .id = FWK_ID_NONE,
    };

    fwk_id_get_element_idx_ExpectAndReturn(event.target_id, 0);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);

    status = thermal_mgmt_process_event(&event, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_thermal_mgmt_process_event_read_temp_pending(void)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;
    struct fwk_event event = {
        .source_id = FWK_ID_NONE,
        .target_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0),
        .id = FWK_ID_NONE,
    };
    dev_ctx = &mod_ctx.dev_ctx_table[0];

    fwk_id_get_element_idx_ExpectAndReturn(event.target_id, 0);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    mod_sensor_get_data_ExpectAndReturn(
        dev_ctx->config->sensor_id, &dev_ctx->sensor_data, FWK_PENDING);
    status = thermal_mgmt_process_event(&event, NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void test_thermal_mgmt_process_event_read_temp_success(void)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;
    struct fwk_event event = {
        .source_id = FWK_ID_NONE,
        .target_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0),
        .id = FWK_ID_NONE,
    };
    dev_ctx = &mod_ctx.dev_ctx_table[0];

    /*
     * Setting the current temperature to a random value (20) and giving a
     * different value to the read temperature (30) will result in an update on
     * `cur_temp`
     */
    dev_ctx->cur_temp = 20;
    dev_ctx->sensor_data.value = 30;

    fwk_id_get_element_idx_ExpectAndReturn(event.target_id, 0);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);
    mod_sensor_get_data_ExpectAndReturn(
        dev_ctx->config->sensor_id, &dev_ctx->sensor_data, FWK_SUCCESS);
    status = thermal_mgmt_process_event(&event, NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(dev_ctx->cur_temp, dev_ctx->sensor_data.value);
}

void test_thermal_mgmt_process_event_read_request_fail(void)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;
    struct fwk_event event = {
        .source_id = FWK_ID_NONE,
        .target_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0),
        .id = FWK_ID_NONE,
    };
    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->sensor_data.status = FWK_E_DEVICE;

    fwk_id_get_element_idx_ExpectAndReturn(event.target_id, 0);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);

    status = thermal_mgmt_process_event(&event, NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_DEVICE);
}

void test_thermal_mgmt_process_event_read_request_success(void)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;
    struct fwk_event event = {
        .source_id = FWK_ID_NONE,
        .target_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0),
        .id = FWK_ID_NONE,
    };
    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->sensor_data.status = FWK_SUCCESS;
    dev_ctx->cur_temp = 20;
    dev_ctx->sensor_data.value = 30;

    fwk_id_get_element_idx_ExpectAndReturn(event.target_id, 0);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(false);
    fwk_id_is_equal_ExpectAnyArgsAndReturn(true);

    status = thermal_mgmt_process_event(&event, NULL);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(dev_ctx->cur_temp, dev_ctx->sensor_data.value);
}
#endif

void test_thermal_mgmt_thermal_protection(void)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->cur_temp = 71;

    fwk_module_get_element_name_ExpectAndReturn(
        dev_ctx->id, thermal_mgmt_domains_elem_table[0].name);
    mod_thermal_mgmt_protection_api_warning_Expect(
        dev_ctx->config->temp_protection->driver_id, dev_ctx->id);

    thermal_protection(dev_ctx);

    dev_ctx->cur_temp = 81;

    fwk_module_get_element_name_ExpectAndReturn(
        dev_ctx->id, thermal_mgmt_domains_elem_table[0].name);
    mod_thermal_mgmt_protection_api_critical_Expect(
        dev_ctx->config->temp_protection->driver_id, dev_ctx->id);

    thermal_protection(dev_ctx);
}

#if THERMAL_HAS_ASYNC_SENSORS
void test_thermal_mgmt_read_temperature_async(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    int status;

    __fwk_put_event_light_ExpectAnyArgsAndReturn(FWK_SUCCESS);

    status = read_temperature(element_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}
#endif

void test_thermal_mgmt_read_temperature_not_async_error(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;

    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->sensor_data.status = FWK_SUCCESS;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    mod_sensor_get_data_ExpectAndReturn(
        dev_ctx->config->sensor_id, &dev_ctx->sensor_data, FWK_PENDING);

    status = read_temperature(element_id);
    TEST_ASSERT_EQUAL(status, FWK_PENDING);
    TEST_ASSERT_EQUAL(dev_ctx->cur_temp, dev_ctx->sensor_data.value);
}

void test_thermal_mgmt_read_temperature_not_async_success(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;

    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->sensor_data.status = FWK_SUCCESS;

    /*
     * Setting the current temperature to a random value (20) and giving a
     * different value to the read temperature (30) will result in an update on
     * `cur_temp`
     */
    dev_ctx->cur_temp = 20;
    dev_ctx->sensor_data.value = 30;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    mod_sensor_get_data_ExpectAndReturn(
        dev_ctx->config->sensor_id, &dev_ctx->sensor_data, FWK_SUCCESS);

    status = read_temperature(element_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(dev_ctx->cur_temp, dev_ctx->sensor_data.value);
}

void test_thermal_mgmt_pi_control_switched_off(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;

    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->cur_temp = 20;
    dev_ctx->config->pi_controller.switch_on_temperature = 31;
    dev_ctx->integral_error = 10;
    dev_ctx->thermal_allocatable_power = 0;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);

    pi_control(element_id);
    TEST_ASSERT_EQUAL(dev_ctx->integral_error, 0);
    TEST_ASSERT_EQUAL(
        dev_ctx->thermal_allocatable_power,
        (uint32_t)dev_ctx->config->cold_state_power);
}

void test_thermal_mgmt_pi_control_anti_windup(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;

    dev_ctx = &mod_ctx.dev_ctx_table[0];

    /*
     * Set a current temperature below the control temperature and make the
     * integral error overpass the cut off value.
     */
    dev_ctx->cur_temp = 45;
    dev_ctx->config->pi_controller.control_temperature = 50;
    dev_ctx->config->pi_controller.switch_on_temperature = 40;
    dev_ctx->integral_error = 10;
    dev_ctx->thermal_allocatable_power = 0;
    dev_ctx->config->pi_controller.integral_cutoff = 4;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    pi_control(element_id);
    TEST_ASSERT_EQUAL(dev_ctx->integral_error, 10);

    dev_ctx->config->pi_controller.integral_cutoff = 10;
    dev_ctx->config->pi_controller.integral_max = 14;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    pi_control(element_id);
    TEST_ASSERT_EQUAL(dev_ctx->integral_error, 10);

    dev_ctx->config->pi_controller.integral_max = 100;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    pi_control(element_id);
    TEST_ASSERT_EQUAL(dev_ctx->integral_error, 15);
}

void test_thermal_mgmt_pi_control_positive_allocatable_power(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int32_t allocatable_power, k_i, k_p, err, terr;

    dev_ctx = &mod_ctx.dev_ctx_table[0];

    /*
     * Set a current temperature below the control temperature and check if the
     * allocatable power was properly calculated.
     */
    dev_ctx->cur_temp = 45;
    dev_ctx->config->pi_controller.control_temperature = 50;
    dev_ctx->config->pi_controller.switch_on_temperature = 40;
    dev_ctx->integral_error = 10;
    dev_ctx->thermal_allocatable_power = 0;
    dev_ctx->config->pi_controller.integral_cutoff = 10;
    dev_ctx->config->pi_controller.integral_max = 100;
    k_i = dev_ctx->config->pi_controller.k_integral = 1;
    k_p = dev_ctx->config->pi_controller.k_p_undershoot = 2;
    err =
        dev_ctx->config->pi_controller.control_temperature - dev_ctx->cur_temp;
    terr = err + dev_ctx->integral_error;
    allocatable_power = k_p * err + k_i * terr + dev_ctx->config->tdp;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    pi_control(element_id);
    TEST_ASSERT_EQUAL(dev_ctx->thermal_allocatable_power, allocatable_power);
}

void test_thermal_mgmt_pi_control_negative_allocatable_power(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;

    dev_ctx = &mod_ctx.dev_ctx_table[0];

    /*
     * Set a current temperature above the control temperature and check if the
     * allocatable power was properly set to 0.
     */
    dev_ctx->cur_temp = 55;
    dev_ctx->config->pi_controller.control_temperature = 50;
    dev_ctx->config->pi_controller.switch_on_temperature = 40;
    dev_ctx->integral_error = 10;
    dev_ctx->thermal_allocatable_power = 0;
    dev_ctx->config->pi_controller.integral_cutoff = 10;
    dev_ctx->config->pi_controller.integral_max = 100;
    dev_ctx->config->pi_controller.k_integral = 1;
    dev_ctx->config->pi_controller.k_p_overshoot = 200;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    pi_control(element_id);
    TEST_ASSERT_EQUAL(dev_ctx->thermal_allocatable_power, 0);
}

void test_thermal_mgmt_control_update_fail_last_reading(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;

    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->tick_counter = 2;
    dev_ctx->control_needs_update = true;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);

    status = control_update(element_id);
    TEST_ASSERT_EQUAL(status, FWK_E_PANIC);
}

void test_thermal_mgmt_control_update_fail_last_invalid_read(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;

    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->tick_counter = 2;
    dev_ctx->control_needs_update = false;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);

#if THERMAL_HAS_ASYNC_SENSORS
    __fwk_put_event_light_ExpectAnyArgsAndReturn(FWK_E_PARAM);
#else
    mod_sensor_get_data_ExpectAnyArgsAndReturn(FWK_E_PARAM);
#endif

    status = control_update(element_id);
    TEST_ASSERT_EQUAL(status, FWK_E_DEVICE);
}

void test_thermal_mgmt_control_update_last_success(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;

    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->control_needs_update = true;
    dev_ctx->tot_spare_power = 100;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);

    status = control_update(element_id);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(dev_ctx->control_needs_update, false);
    TEST_ASSERT_EQUAL(dev_ctx->tot_spare_power, 0);
}

void test_thermal_mgmt_thermal_update_fail(void)
{
    fwk_id_t element_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, 0);
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;

    dev_ctx = &mod_ctx.dev_ctx_table[0];
    dev_ctx->tick_counter = 2;
    dev_ctx->control_needs_update = true;

    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);
    fwk_id_get_element_idx_ExpectAndReturn(element_id, 0);

    status = thermal_update(NULL);
    TEST_ASSERT_EQUAL(status, FWK_E_PANIC);
}

void test_thermal_mgmt_thermal_update_success(void)
{
    int status;
    unsigned int i;
    struct perf_plugins_perf_update data;
    uint32_t level, adj_max_limit;

    level = 5;
    adj_max_limit = 10;
    data.level = &level;
    data.adj_max_limit = &adj_max_limit;

    for (i = 0; i < mod_ctx.dev_ctx_count; i++) {
        fwk_id_t element_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_THERMAL_MGMT, i);
        struct mod_thermal_mgmt_dev_ctx *dev_ctx;

        dev_ctx = &mod_ctx.dev_ctx_table[i];
        dev_ctx->tick_counter = 0;
        dev_ctx->control_needs_update = false;

        fwk_id_get_element_idx_ExpectAndReturn(element_id, i);
        fwk_id_get_element_idx_ExpectAndReturn(element_id, i);
        distribute_power_Expect(dev_ctx->id, data.level, data.adj_max_limit);
    }

    status = thermal_update(&data);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int mod_thermal_mgmt_test_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_thermal_mgmt_init);
    RUN_TEST(test_thermal_mgmt_dev_init_error_null_ptr);
    RUN_TEST(test_thermal_mgmt_dev_init_error_no_config);
    RUN_TEST(test_thermal_mgmt_dev_init_error_invalid_dvfs_domain);
    RUN_TEST(test_thermal_mgmt_dev_init_error_invalid_driver);
    RUN_TEST(test_thermal_mgmt_dev_init_success);
    RUN_TEST(test_thermal_mgmt_bind_round_1);
    RUN_TEST(test_thermal_mgmt_bind_module);
    RUN_TEST(test_thermal_mgmt_bind_fail);
    RUN_TEST(test_thermal_mgmt_bind_success);
#if THERMAL_HAS_ASYNC_SENSORS
    RUN_TEST(test_thermal_mgmt_process_bind_request);
    RUN_TEST(test_thermal_mgmt_process_event_invalid);
    RUN_TEST(test_thermal_mgmt_process_event_read_temp_pending);
    RUN_TEST(test_thermal_mgmt_process_event_read_temp_success);
    RUN_TEST(test_thermal_mgmt_process_event_read_request_fail);
    RUN_TEST(test_thermal_mgmt_process_event_read_request_success);
#endif

    RUN_TEST(test_thermal_mgmt_thermal_protection);

#if THERMAL_HAS_ASYNC_SENSORS
    RUN_TEST(test_thermal_mgmt_read_temperature_async);
#else
    RUN_TEST(test_thermal_mgmt_read_temperature_not_async_error);
    RUN_TEST(test_thermal_mgmt_read_temperature_not_async_success);
#endif

    RUN_TEST(test_thermal_mgmt_pi_control_switched_off);
    RUN_TEST(test_thermal_mgmt_pi_control_anti_windup);
    RUN_TEST(test_thermal_mgmt_pi_control_positive_allocatable_power);
    RUN_TEST(test_thermal_mgmt_pi_control_negative_allocatable_power);
    RUN_TEST(test_thermal_mgmt_control_update_fail_last_reading);
    RUN_TEST(test_thermal_mgmt_control_update_fail_last_invalid_read);
    RUN_TEST(test_thermal_mgmt_control_update_last_success);
    RUN_TEST(test_thermal_mgmt_thermal_update_fail);
    RUN_TEST(test_thermal_mgmt_thermal_update_success);

    return UNITY_END();
}

int main(void)
{
    return mod_thermal_mgmt_test_main();
}
