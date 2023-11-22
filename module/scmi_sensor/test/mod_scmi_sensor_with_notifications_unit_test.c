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

static int scmi_sensor_driver_get_agent_count_success(unsigned int *agent_count)
{
    return FWK_SUCCESS;
}

static int scmi_sensor_driver_get_agent_count_fail(unsigned int *agent_count)
{
    return FWK_E_PARAM;
}

static int scmi_sensor_driver_notification_init(
    unsigned int protocol_id,
    unsigned int agent_count,
    unsigned int element_count,
    unsigned int operation_count)
{
    return FWK_SUCCESS;
}

static struct mod_scmi_from_protocol_api scmi_driver_api = {
    .get_agent_count = scmi_sensor_driver_get_agent_count_success,
};

static struct mod_scmi_notification_api scmi_sensor_notification_api = {
    .scmi_notification_init = scmi_sensor_driver_notification_init,
};

void setUp(void)
{
}

void tearDown(void)
{
}
void utest_scmi_sensor_bind_start_fail(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SCMI_SENSOR_FAKE_INDEX_0);

    scmi_driver_api.get_agent_count = scmi_sensor_driver_get_agent_count_fail;

    scmi_sensor_ctx.scmi_api = &scmi_driver_api;
    scmi_sensor_ctx.scmi_notification_api = &scmi_sensor_notification_api;
    scmi_sensor_ctx.agent_count = 1;

    status = scmi_sensor_start(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_E_PARAM);
}

void utest_scmi_sensor_bind_start_success(void)
{
    int status;

    fwk_id_t elem_id =
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SENSOR, SCMI_SENSOR_FAKE_INDEX_0);

    scmi_driver_api.get_agent_count =
        scmi_sensor_driver_get_agent_count_success;

    scmi_sensor_ctx.scmi_api = &scmi_driver_api;
    scmi_sensor_ctx.scmi_notification_api = &scmi_sensor_notification_api;
    scmi_sensor_ctx.agent_count = 1;

    status = scmi_sensor_start(elem_id);

    TEST_ASSERT_EQUAL(status, FWK_SUCCESS);
}

int sensor_test_main(void)
{
    UNITY_BEGIN();

    RUN_TEST(utest_scmi_sensor_bind_start_fail);
    RUN_TEST(utest_scmi_sensor_bind_start_success);

    return UNITY_END();
}

#if !defined(TEST_ON_TARGET)
int main(void)
{
    return sensor_test_main();
}
#endif
