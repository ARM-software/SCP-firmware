/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "Mockmod_sensor_with_conditionals_extra.h"
#include "scp_unity.h"
#include "unity.h"

#include <Mockfwk_core.h>
#include <Mockfwk_id.h>
#include <Mockfwk_mm.h>
#include <Mockfwk_module.h>
#include <Mockfwk_string.h>
#include <internal/Mockfwk_core_internal.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_element.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include UNIT_TEST_SRC

#include <config_sensor.h>

void setUp(void)
{
    ctx_table = sensor_dev_context;
    memset(
        sensor_dev_context,
        0,
        SENSOR_ELEMENT_COUNT * sizeof(struct sensor_dev_ctx));

    sensor_dev_context[SENSOR_FAKE_INDEX_0].config =
        (struct mod_sensor_dev_config *)
            sensor_element_table[SENSOR_FAKE_INDEX_0]
                .data;
    sensor_dev_context[SENSOR_FAKE_INDEX_1].config =
        (struct mod_sensor_dev_config *)
            sensor_element_table[SENSOR_FAKE_INDEX_1]
                .data;
}

void tearDown(void)
{
}

void utest_sensor_start_module_id(void)
{
    int status;

    fwk_id_t module_id = FWK_ID_MODULE(FWK_ID_TYPE_MODULE);

    fwk_id_is_type_ExpectAndReturn(module_id, FWK_ID_TYPE_MODULE, true);

    status = sensor_start(module_id);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_start_element_id(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_1);

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_MODULE, false);

    sensor_axis_start_ExpectAndReturn(elem_id, FWK_SUCCESS);

    status = sensor_start(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_start_element_id_sensor_axis_start_returns_error(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_1);

    fwk_id_is_type_ExpectAndReturn(elem_id, FWK_ID_TYPE_MODULE, false);

    sensor_axis_start_ExpectAndReturn(elem_id, FWK_E_DEVICE);

    status = sensor_start(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_E_DEVICE);
}

void utest_sensor_dev_init_success(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_1);

    struct sensor_trip_point_ctx trip_point_context[SENSOR_TRIP_POINT_COUNT];

    fwk_mm_calloc_ExpectAndReturn(
        1, sizeof(struct sensor_trip_point_ctx), (void *)trip_point_context);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_1);

    sensor_timestamp_dev_init_ExpectAndReturn(
        elem_id, &sensor_dev_context[SENSOR_FAKE_INDEX_1], FWK_SUCCESS);

    status = sensor_dev_init(
        elem_id, 0, sensor_element_table[SENSOR_FAKE_INDEX_1].data);

    TEST_ASSERT_EQUAL(
        ctx_table[SENSOR_FAKE_INDEX_1].trip_point_ctx->enabled, true);
    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

void utest_sensor_dev_init_failure(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_1);

    struct sensor_trip_point_ctx trip_point_context[SENSOR_TRIP_POINT_COUNT];

    fwk_mm_calloc_ExpectAndReturn(
        1, sizeof(struct sensor_trip_point_ctx), (void *)trip_point_context);
    fwk_id_get_element_idx_ExpectAndReturn(elem_id, SENSOR_FAKE_INDEX_1);

    sensor_timestamp_dev_init_ExpectAndReturn(
        elem_id, &sensor_dev_context[SENSOR_FAKE_INDEX_1], FWK_E_DEVICE);

    status = sensor_dev_init(
        elem_id, 0, sensor_element_table[SENSOR_FAKE_INDEX_1].data);

    TEST_ASSERT_EQUAL(
        ctx_table[SENSOR_FAKE_INDEX_1].trip_point_ctx->enabled, true);
    TEST_ASSERT_EQUAL(status, FWK_E_DEVICE);
}

int sensor_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_sensor_start_module_id);
    RUN_TEST(utest_sensor_start_element_id);
    RUN_TEST(utest_sensor_start_element_id_sensor_axis_start_returns_error);

    RUN_TEST(utest_sensor_dev_init_success);
    RUN_TEST(utest_sensor_dev_init_failure);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return sensor_test_main();
}
#endif
