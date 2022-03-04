/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mock_sensor.h>
#include <mod_sensor.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

#define MOCK_SENSOR_ALARM_DELAY_MS  10
struct mock_sensor_dev_ctx {
    struct mod_mock_sensor_dev_config *config;
    struct mod_timer_alarm_api *alarm_api;
    struct mod_sensor_driver_response_api *driver_response_api;
};

static struct mock_sensor_dev_ctx *ctx_table;

static void mock_sensor_callback(uintptr_t param)
{
    struct mod_sensor_driver_resp_params response;
    struct mock_sensor_dev_ctx *ctx;
    unsigned int sensor_hal_idx;
#ifdef BUILD_HAS_SCMI_SENSOR_V2
    unsigned int i;
#endif

    ctx = (struct mock_sensor_dev_ctx *)param;
    sensor_hal_idx = fwk_id_get_element_idx(ctx->config->sensor_hal_id);

    response.status = FWK_SUCCESS;
#ifdef BUILD_HAS_SCMI_SENSOR_V2
    if (ctx->config->axis_count > 1) {
        for (i = 0; i < ctx->config->axis_count; i++) {
            response.axis_value[i] = ctx->config->read_value[i];
        }
    } else {
        response.value = *ctx->config->read_value;
    }
#else
    response.value = *ctx->config->read_value;
#endif

    fwk_id_t sensor_id = fwk_id_build_element_id(
        fwk_module_id_sensor, (unsigned int)sensor_hal_idx);
    ctx->driver_response_api->reading_complete(sensor_id, &response);

    return;
}

/*
 * Module API
 */
static int get_value(fwk_id_t id, mod_sensor_value_t *value)
{
    struct mock_sensor_dev_ctx *ctx;
    int status;

    ctx = ctx_table + fwk_id_get_element_idx(id);

    status = ctx->alarm_api->start(
        ctx->config->alarm_id,
        MOCK_SENSOR_ALARM_DELAY_MS,
        MOD_TIMER_ALARM_TYPE_ONCE,
        mock_sensor_callback,
        (uintptr_t)ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Mock sensor always defers this request */
    return FWK_PENDING;
}

static int get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    struct mock_sensor_dev_ctx *ctx;

    ctx = ctx_table + fwk_id_get_element_idx(id);

    if (info == NULL) {
        return FWK_E_PARAM;
    }

    *info = *ctx->config->info;
    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_SCMI_SENSOR_V2
unsigned int get_axis_count(fwk_id_t id)
{
    struct mock_sensor_dev_ctx *ctx;

    ctx = ctx_table + fwk_id_get_element_idx(id);

    return ctx->config->axis_count;
}

int get_axis_info(fwk_id_t id, uint32_t axis, struct mod_sensor_axis_info *info)
{
    struct mock_sensor_dev_ctx *ctx;

    ctx = ctx_table + fwk_id_get_element_idx(id);

    fwk_assert(info != NULL || axis < ctx->config->axis_count);

    *info = ctx->config->axis_info[axis];
    return FWK_SUCCESS;
}
#endif

static const struct mod_sensor_driver_api mock_sensor_api = {
    .get_value = get_value,
    .get_info = get_info,
#ifdef BUILD_HAS_SCMI_SENSOR_V2
    .get_axis_count = get_axis_count,
    .get_axis_info = get_axis_info,
#endif
};

/*
 * Framework handlers
 */
static int mock_sensor_init(fwk_id_t module_id,
                            unsigned int element_count,
                            const void *data)
{
    ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct mock_sensor_dev_ctx));
    return FWK_SUCCESS;
}

static int mock_sensor_element_init(fwk_id_t element_id,
                                    unsigned int unused,
                                    const void *data)
{
    struct mock_sensor_dev_ctx *ctx;

    fwk_assert(data != NULL);

    ctx = ctx_table + fwk_id_get_element_idx(element_id);
    ctx->config = (struct mod_mock_sensor_dev_config *)data;

    return FWK_SUCCESS;
}

static int mock_sensor_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mock_sensor_dev_ctx *ctx;

    if ((round > 0) || fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /*
         * Only bind in first round of calls
         * Nothing to do for module
         */
        return FWK_SUCCESS;
    }

    ctx = ctx_table + fwk_id_get_element_idx(id);

    status = fwk_module_bind(
        ctx->config->alarm_id,
        FWK_ID_API(FWK_MODULE_IDX_TIMER, 1),
        &ctx->alarm_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return fwk_module_bind(
        ctx->config->sensor_hal_id,
        mod_sensor_api_id_driver_response,
        &ctx->driver_response_api);
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
    .api_count = 1,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mock_sensor_init,
    .element_init = mock_sensor_element_init,
    .bind = mock_sensor_bind,
    .process_bind_request = mock_sensor_process_bind_request,
};
