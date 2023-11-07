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

static struct sensor_trip_point_ctx
    sensor_trip_point_context[SENSOR_TRIP_POINT_COUNT];

struct sensor_dev_ctx *sensor_get_ctx(fwk_id_t id)
{
    return &sensor_dev_context[SENSOR_FAKE_INDEX_0];
}

void setUp(void)
{
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

    sensor_dev_context[SENSOR_FAKE_INDEX_0].trip_point_ctx =
        &sensor_trip_point_context[SENSOR_FAKE_INDEX_0];
    sensor_dev_context[SENSOR_FAKE_INDEX_1].trip_point_ctx =
        &sensor_trip_point_context[SENSOR_FAKE_INDEX_1];
}

void tearDown(void)
{
}

void test_sensor_get_timestamp_config_null_config(void)
{
    int status;

    fwk_id_t elem_id_0 =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_is_equal_ExpectAndReturn(elem_id_0, FWK_ID_NONE, false);

    status = sensor_get_timestamp_config(elem_id_0, NULL);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_sensor_get_timestamp_config_no_ts_support(void)
{
    int status;

    fwk_id_t elem_id_0 =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    struct mod_sensor_timestamp_info timestamp;

    fwk_id_is_equal_ExpectAndReturn(elem_id_0, FWK_ID_NONE, false);

    sensor_dev_context[SENSOR_FAKE_INDEX_0].timestamp.timestamp_support = false;

    status = sensor_get_timestamp_config(elem_id_0, &timestamp);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void test_sensor_get_timestamp_config_success(void)
{
    int status;

    fwk_id_t elem_id_0 =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    struct mod_sensor_timestamp_info timestamp;

    memset(&timestamp, 0, sizeof(timestamp));

    fwk_id_is_equal_ExpectAndReturn(elem_id_0, FWK_ID_NONE, false);

    sensor_dev_context[SENSOR_FAKE_INDEX_0].timestamp.timestamp_support = true;
    sensor_dev_context[SENSOR_FAKE_INDEX_0].timestamp.enabled = true;

    status = sensor_get_timestamp_config(elem_id_0, &timestamp);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(timestamp.enabled, true);
}

void test_sensor_set_timestamp_config_null_config(void)
{
    int status;

    fwk_id_t elem_id_0 =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    fwk_id_is_equal_ExpectAndReturn(elem_id_0, FWK_ID_NONE, false);

    status = sensor_set_timestamp_config(elem_id_0, NULL);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_sensor_set_timestamp_config_no_ts_support(void)
{
    int status;

    fwk_id_t elem_id_0 =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    struct mod_sensor_timestamp_info timestamp;

    fwk_id_is_equal_ExpectAndReturn(elem_id_0, FWK_ID_NONE, false);

    sensor_dev_context[SENSOR_FAKE_INDEX_0].timestamp.timestamp_support = false;

    status = sensor_set_timestamp_config(elem_id_0, &timestamp);

    TEST_ASSERT_EQUAL(status, FWK_E_SUPPORT);
}

void test_sensor_set_timestamp_config_success(void)
{
    int status;

    fwk_id_t elem_id_0 =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SENSOR_FAKE_INDEX_0);

    struct mod_sensor_timestamp_info test_timestamp_info;

    memset(&test_timestamp_info, 0, sizeof(test_timestamp_info));

    test_timestamp_info.timestamp_support = true;
    test_timestamp_info.enabled = true;
    test_timestamp_info.exponent = SENSOR_FAKE_EXPONENT_VALUE;

    fwk_id_is_equal_ExpectAndReturn(elem_id_0, FWK_ID_NONE, false);

    sensor_dev_context[SENSOR_FAKE_INDEX_0].timestamp.timestamp_support = true;
    sensor_dev_context[SENSOR_FAKE_INDEX_0].timestamp.enabled = false;
    sensor_dev_context[SENSOR_FAKE_INDEX_0].timestamp.exponent = 0;

    status = sensor_set_timestamp_config(elem_id_0, &test_timestamp_info);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(
        sensor_dev_context[SENSOR_FAKE_INDEX_0].timestamp.enabled,
        test_timestamp_info.enabled);
    TEST_ASSERT_EQUAL(
        sensor_dev_context[SENSOR_FAKE_INDEX_0].timestamp.exponent,
        test_timestamp_info.exponent);
}

int sensor_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_sensor_get_timestamp_config_null_config);
    RUN_TEST(test_sensor_get_timestamp_config_no_ts_support);
    RUN_TEST(test_sensor_get_timestamp_config_success);

    RUN_TEST(test_sensor_set_timestamp_config_null_config);
    RUN_TEST(test_sensor_set_timestamp_config_no_ts_support);
    RUN_TEST(test_sensor_set_timestamp_config_success);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return sensor_test_main();
}
#endif
