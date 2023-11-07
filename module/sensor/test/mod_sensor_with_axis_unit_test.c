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

#define AXIS_COUNT_HIGH 1

struct sensor_dev_ctx *sensor_get_ctx(fwk_id_t id)
{
    return &sensor_dev_context[SENSOR_FAKE_INDEX_0];
}

static int sensor_driver_get_value(fwk_id_t id, mod_sensor_value_t *value)
{
    return FWK_SUCCESS;
}

static int sensor_driver_get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    return FWK_SUCCESS;
}

static int sensor_driver_get_axis_info(
    fwk_id_t id,
    uint32_t axis,
    struct mod_sensor_axis_info *info)
{
    info->type = MOD_SENSOR_TYPE_DEGREES_C;
    return FWK_SUCCESS;
}

static struct mod_sensor_driver_api sensor_driver_api = {
    .get_value = sensor_driver_get_value,
    .get_info = sensor_driver_get_info,
#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    .get_axis_info = sensor_driver_get_axis_info,
#endif
};

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
}

void tearDown(void)
{
}

void test_sensor_get_axis_info_null_config(void)
{
    int status;

    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, 0);

    status = sensor_get_axis_info(elem_id, 0, NULL);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_sensor_get_axis_info_axis_too_high(void)
{
    int status = FWK_SUCCESS;

    struct mod_sensor_axis_info returned_axis_info;

    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, 0);

    status =
        sensor_get_axis_info(elem_id, AXIS_COUNT_HIGH, &returned_axis_info);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void test_sensor_get_axis_info(void)
{
    int status = FWK_SUCCESS;

    struct mod_sensor_axis_info returned_axis_info;

#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    sensor_dev_context[SENSOR_FAKE_INDEX_0].driver_api = &sensor_driver_api;
#endif

    fwk_id_t elem_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, 0);

    status = sensor_get_axis_info(elem_id, 0, &returned_axis_info);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
    TEST_ASSERT_EQUAL(returned_axis_info.type, MOD_SENSOR_TYPE_DEGREES_C);
}

int sensor_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_sensor_get_axis_info_null_config);
    RUN_TEST(test_sensor_get_axis_info_axis_too_high);
    RUN_TEST(test_sensor_get_axis_info);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return sensor_test_main();
}
#endif
