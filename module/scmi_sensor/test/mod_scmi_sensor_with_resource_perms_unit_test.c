/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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

#include <config_scmi_sensor.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void utest_scmi_sensor_bind_two_binds_succeed(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_FAKE_INDEX_0);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_sensor_ctx.scmi_api,
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR),
        mod_sensor_api_id_sensor,
        &scmi_sensor_ctx.sensor_api,
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &scmi_sensor_ctx.res_perms_api,
        FWK_E_PARAM);

    status = scmi_sensor_bind(elem_id, SCMI_SENSOR_ROUND_0);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_scmi_sensor_bind_three_binds_succeed(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_SENSOR, SCMI_SENSOR_FAKE_INDEX_0);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_sensor_ctx.scmi_api,
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR),
        mod_sensor_api_id_sensor,
        &scmi_sensor_ctx.sensor_api,
        FWK_SUCCESS);

    fwk_module_bind_ExpectAndReturn(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &scmi_sensor_ctx.res_perms_api,
        FWK_SUCCESS);

    status = scmi_sensor_bind(elem_id, SCMI_SENSOR_ROUND_0);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int sensor_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_scmi_sensor_bind_two_binds_succeed);
    RUN_TEST(utest_scmi_sensor_bind_three_binds_succeed);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return sensor_test_main();
}
#endif
