/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <mod_sensor.h>

struct sensor_dev_ctx {
    struct mod_sensor_dev_config *config;
    struct mod_sensor_driver_api *driver_api;
};

static struct sensor_dev_ctx *ctx_table;

static int get_ctx_if_valid_call(fwk_id_t id,
                                 void *data,
                                 struct sensor_dev_ctx **ctx)
{
    int status;

    fwk_assert(fwk_module_is_valid_element_id(id));
    fwk_assert(ctx != NULL);

    if (!fwk_expect(data != NULL))
        return FWK_E_PARAM;

    status = fwk_module_check_call(id);
    if (status != FWK_SUCCESS)
        return status;

    *ctx = ctx_table + fwk_id_get_element_idx(id);

    return FWK_SUCCESS;
}

/*
 * Module API
 */
static int get_value(fwk_id_t id, uint64_t *value)
{
    int status;
    struct sensor_dev_ctx *ctx;

    status = get_ctx_if_valid_call(id, value, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    status = ctx->driver_api->get_value(ctx->config->driver_id, value);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static int get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    int status;
    struct sensor_dev_ctx *ctx;

    status = get_ctx_if_valid_call(id, info, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    status = ctx->driver_api->get_info(ctx->config->driver_id, info);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static struct mod_sensor_api sensor_api = {
    .get_value = get_value,
    .get_info  = get_info,
};

/*
 * Framework handlers
 */
static int sensor_init(fwk_id_t module_id,
                       unsigned int element_count,
                       const void *unused)
{
    ctx_table = fwk_mm_alloc(element_count, sizeof(ctx_table[0]));

    if (ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int sensor_dev_init(fwk_id_t element_id,
                           unsigned int unused,
                           const void *data)
{
    struct sensor_dev_ctx *ctx;
    struct mod_sensor_dev_config *config;

    ctx = ctx_table + fwk_id_get_element_idx(element_id);
    config = (struct mod_sensor_dev_config*)data;

    ctx->config = config;

    return FWK_SUCCESS;
}

static int sensor_bind(fwk_id_t id, unsigned int round)
{
    struct sensor_dev_ctx *ctx;
    int status;
    struct mod_sensor_driver_api *driver = NULL;

    if ((round > 0) || fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /*
         * Only bind in first round of calls
         * Nothing to do for module
         */
        return FWK_SUCCESS;
    }

    ctx = ctx_table + fwk_id_get_element_idx(id);

    /* Bind to driver */
    status = fwk_module_bind(ctx->config->driver_id,
        FWK_ID_API(fwk_id_get_module_idx(ctx->config->driver_id), 0),
        &driver);
    if (status != FWK_SUCCESS)
        return status;

    /* Validate driver API */
    if ((driver == NULL) || (driver->get_value == NULL))
        return FWK_E_DATA;

    ctx->driver_api = driver;

    return FWK_SUCCESS;
}

static int sensor_process_bind_request(fwk_id_t source_id,
                                       fwk_id_t target_id,
                                       fwk_id_t api_type,
                                       const void **api)
{
    *api = &sensor_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_sensor = {
    .name = "SENSOR",
    .api_count = 1,
    .event_count = 0,
    .type = FWK_MODULE_TYPE_HAL,
    .init = sensor_init,
    .element_init = sensor_dev_init,
    .bind = sensor_bind,
    .process_bind_request = sensor_process_bind_request,
};
