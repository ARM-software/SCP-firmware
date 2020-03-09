/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mock_sensor.h>
#include <mod_sensor.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdint.h>

#define MOCK_SENSOR_ALARM_DELAY_MS  10

static struct mod_timer_alarm_api *alarm_api;
static struct mod_sensor_driver_response_api *driver_response_api;

static void mock_sensor_callback(uintptr_t param)
{
    struct mod_sensor_driver_resp_params response;

    response.status = FWK_SUCCESS;
    response.value = (273 + 25); /* Mimic a comfortable temperature in 'K */

    fwk_id_t sensor_id = fwk_id_build_element_id(fwk_module_id_sensor,
                                                 (unsigned int)param);
    driver_response_api->reading_complete(sensor_id, &response);

    return;
}

/*
 * Module API
 */

static int get_value(fwk_id_t id, uint64_t *value)
{
    unsigned int sensor_hal_idx;
    const struct mod_mock_sensor_dev_config *config;

    config = fwk_module_get_data(id);

    sensor_hal_idx = fwk_id_get_element_idx(config->sensor_hal_id);
    status = alarm_api->start(config->alarm_id,
                              MOCK_SENSOR_ALARM_DELAY_MS,
                              MOD_TIMER_ALARM_TYPE_ONCE,
                              mock_sensor_callback,
                              (uintptr_t)sensor_hal_idx);
    if (status != FWK_SUCCESS)
        return status;

    /* Mock sensor always defers this request */
    return FWK_PENDING;
}

static int get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    const struct mod_mock_sensor_dev_config *config;

    config = fwk_module_get_data(id);

    if ((info == NULL) || (config == NULL))
        return FWK_E_PARAM;

    *info = *config->info;
    return FWK_SUCCESS;
}

static const struct mod_sensor_driver_api mock_sensor_api = {
    .get_value = get_value,
    .get_info = get_info,
};

/*
 * Framework handlers
 */
static int mock_sensor_init(fwk_id_t module_id,
                            unsigned int element_count,
                            const void *data)
{
    return FWK_SUCCESS;
}

static int mock_sensor_element_init(fwk_id_t element_id,
                                    unsigned int unused,
                                    const void *data)
{
    return FWK_SUCCESS;
}

static int mock_sensor_bind(fwk_id_t id, unsigned int round)
{
    int status;
    const struct mod_mock_sensor_dev_config *config;

    if ((round > 0) || fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /*
         * Only bind in first round of calls
         * Nothing to do for module
         */
        return FWK_SUCCESS;
    }

    config = fwk_module_get_data(id);

    status = fwk_module_bind(config->alarm_id,
                             FWK_ID_API(FWK_MODULE_IDX_TIMER, 1),
                             &alarm_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(config->sensor_hal_id,
                           mod_sensor_api_id_driver_response,
                           &driver_response_api);
}

static int mock_sensor_process_bind_request(fwk_id_t source_id,
                                            fwk_id_t target_id,
                                            fwk_id_t api_type,
                                            const void **api)
{
    *api = &mock_sensor_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_mock_sensor = {
    .name = "Mock Sensor",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mock_sensor_init,
    .element_init = mock_sensor_element_init,
    .bind = mock_sensor_bind,
    .process_bind_request = mock_sensor_process_bind_request,
};
