/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_multi_thread.h>
#include <fwk_status.h>
#include <mod_i2c.h>
#include <mod_juno_xrp7724.h>
#include <mod_sensor.h>
#include <mod_timer.h>
#include <juno_id.h>

/* Maximum required length for the I2C transmissions */
#define TRANSMIT_DATA_MAX   3
/* Maximum required length for the I2C receptions */
#define RECEIVE_DATA_MAX    2

#define GPIO_DELAY_US       3000

/* Write GPIO command */
#define GPIO_WRITE_CMD      0x31
#define SET_GPIO_MASK       0x01

/* I2C transmission length to set a GPIO */
#define GPIO_WRITE_TRANSMIT_LENGTH 3

/* I2C transmission and reception length to read the sensor value */
#define SENSOR_WRITE_LENGTH  1
#define SENSOR_READ_LENGTH   2

/* Read temperature command */
#define SENSOR_READ_TEMP     0x15

enum juno_xrp7724_event_idx {
    JUNO_XRP7724_EVENT_IDX_REQUEST,
    JUNO_XRP7724_EVENT_IDX_COUNT
};

enum juno_xrp7724_gpio_request {
    JUNO_XRP7724_GPIO_REQUEST_IDLE,
    JUNO_XRP7724_GPIO_REQUEST_MODE_SHUTDOWN,
    JUNO_XRP7724_GPIO_REQUEST_ASSERT_SHUTDOWN,
    JUNO_XRP7724_GPIO_REQUEST_ASSERT_COLD_RESET,
    JUNO_XRP7724_GPIO_REQUEST_DONE,
};

enum juno_xrp7724_sensor_request {
    JUNO_XRP7724_SENSOR_REQUEST_IDLE,
    JUNO_XRP7724_SENSOR_REQUEST_READ_VALUE,
    JUNO_XRP7724_SENSOR_REQUEST_CONVERT_VALUE,
};

struct juno_xrp7724_ctx {
    const struct mod_juno_xrp7724_config *config;
    const struct mod_sensor_driver_response_api *sensor_driver_response_api;
    const struct mod_i2c_api *i2c_api;
    const struct mod_timer_api *timer_api;
    enum juno_xrp7724_gpio_request gpio_request;
    enum juno_xrp7724_sensor_request sensor_request;
    bool driver_skipped;
};

struct juno_xrp7724_dev_ctx {
    const struct mod_juno_xrp7724_dev_config *config;
    uint32_t cookie;
    uint8_t transmit_data[TRANSMIT_DATA_MAX];
    uint8_t receive_data[RECEIVE_DATA_MAX];
};

static const fwk_id_t juno_xrp7724_event_id_request =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
        JUNO_XRP7724_EVENT_IDX_REQUEST);

static struct juno_xrp7724_dev_ctx *ctx_table;
static struct juno_xrp7724_ctx module_ctx;

/* Helper function to set a GPIO */
static int set_gpio(fwk_id_t id, struct juno_xrp7724_dev_ctx *ctx)
{
    int status;

    fwk_assert(fwk_module_is_valid_sub_element_id(id));

    ctx->transmit_data[0] = GPIO_WRITE_CMD;
    ctx->transmit_data[1] = fwk_id_get_sub_element_idx(id);
    ctx->transmit_data[2] = SET_GPIO_MASK;

    /*
     * Send a request to perform a transmission on the I2C bus. When the
     * request has been completed, an event from I2C is received.
     */
    status = module_ctx.i2c_api->transmit_as_master(
        module_ctx.config->i2c_hal_id, module_ctx.config->slave_address,
        ctx->transmit_data, GPIO_WRITE_TRANSMIT_LENGTH);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

/* Helper function for the sensor API */
static uint64_t temperature_to_millidegree_celsius(uint16_t temp)
{
    return (temp * 5000ULL) - 272150;
}

/*
 * Functions for the system mode API
 */
static void juno_xrp7724_shutdown(void)
{
    int status;
    fwk_id_t gpio_id = fwk_id_build_element_id(fwk_module_id_juno_xrp7724,
        fwk_id_get_element_idx(module_ctx.config->gpio_mode_id));

    struct fwk_event resp;
    struct fwk_event event = (struct fwk_event) {
        .target_id = gpio_id,
        .id = juno_xrp7724_event_id_request,
    };

    fwk_assert(module_ctx.gpio_request == JUNO_XRP7724_GPIO_REQUEST_IDLE);

    /* Select the mode to perform a shutdown */
    module_ctx.gpio_request = JUNO_XRP7724_GPIO_REQUEST_MODE_SHUTDOWN;

    status = fwk_thread_put_event_and_wait(&event, &resp);
    fwk_assert(status == FWK_SUCCESS);
}

static void juno_xrp7724_reset(void)
{
    int status;
    fwk_id_t gpio_id = fwk_id_build_element_id(fwk_module_id_juno_xrp7724,
        fwk_id_get_element_idx(module_ctx.config->gpio_assert_id));

    struct fwk_event resp;
    struct fwk_event event = (struct fwk_event) {
        .target_id = gpio_id,
        .id = juno_xrp7724_event_id_request,
    };

    fwk_assert(module_ctx.gpio_request == JUNO_XRP7724_GPIO_REQUEST_IDLE);

    /*
     * There is no need to select the reset mode prior to send the assert
     * command. This GPIO is low whenever the board is shut down or rebooted so
     * we can assume it is low now. The assert command is directly sent.
     */
    module_ctx.gpio_request = JUNO_XRP7724_GPIO_REQUEST_ASSERT_COLD_RESET;

    status = fwk_thread_put_event_and_wait(&event, &resp);
    fwk_assert(status == FWK_SUCCESS);
}

static const struct mod_juno_xrp7724_api_system_mode system_mode_api = {
    .shutdown = juno_xrp7724_shutdown,
    .reset = juno_xrp7724_reset,
};

/*
 * Driver functions for the sensor API
 */
static int juno_xrp7724_sensor_get_value(fwk_id_t id, uint64_t *value)
{
    int status;
    struct fwk_event event;
    struct juno_xrp7724_dev_ctx *ctx;

    fwk_assert(fwk_module_is_valid_element_id(id));

    status = fwk_module_check_call(id);
    if (status != FWK_SUCCESS)
        return status;

    if (module_ctx.sensor_request != JUNO_XRP7724_SENSOR_REQUEST_IDLE)
        return FWK_E_BUSY;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];
    fwk_assert(ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_SENSOR);

    event = (struct fwk_event) {
        .target_id = id,
        .id = juno_xrp7724_event_id_request,
    };

    module_ctx.sensor_request = JUNO_XRP7724_SENSOR_REQUEST_READ_VALUE;

    status = fwk_thread_put_event(&event);
    if (status != FWK_SUCCESS) {
        module_ctx.sensor_request = JUNO_XRP7724_SENSOR_REQUEST_IDLE;
        return status;
    }

    return FWK_PENDING;
}

static int juno_xrp7724_sensor_get_info(fwk_id_t id,
                                        struct mod_sensor_info *info)
{
    int status;
    const struct juno_xrp7724_dev_ctx *ctx;

    fwk_assert(fwk_module_is_valid_element_id(id));
    fwk_assert(info != NULL);

    status = fwk_module_check_call(id);
    if (status != FWK_SUCCESS)
        return status;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];
    fwk_assert(ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_SENSOR);

    *info = *(ctx->config->sensor_info);

    return FWK_SUCCESS;
}

static const struct mod_sensor_driver_api sensor_driver_api = {
    .get_value = juno_xrp7724_sensor_get_value,
    .get_info = juno_xrp7724_sensor_get_info,
};

/*
 * Framework handlers
 */
static int juno_xrp7724_init(fwk_id_t module_id,
                             unsigned int element_count,
                             const void *data)
{
    int status;
    enum juno_idx_platform platform_id;

    fwk_assert(data != NULL);

    module_ctx.config = (struct mod_juno_xrp7724_config *)data;

    ctx_table = fwk_mm_calloc(element_count,
        sizeof(struct juno_xrp7724_dev_ctx));
    if (ctx_table == NULL)
        return FWK_E_NOMEM;

    status = juno_id_get_platform(&platform_id);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_PANIC;

    if (platform_id == JUNO_IDX_PLATFORM_FVP) {
        /* XRP7724 hardware is only available in the board */
        module_ctx.driver_skipped = true;
    }

    return FWK_SUCCESS;
}

static int juno_xrp7724_element_init(fwk_id_t element_id,
                                     unsigned int sub_element_count,
                                     const void *data)
{
    struct juno_xrp7724_dev_ctx *ctx;

    fwk_assert(data != NULL);

    ctx = ctx_table + fwk_id_get_element_idx(element_id);
    ctx->config = (struct mod_juno_xrp7724_dev_config *)data;

    if (ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_SENSOR) {
        if (ctx->config->sensor_info == NULL)
            return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

static int juno_xrp7724_bind(fwk_id_t id, unsigned int round)
{
    int status;
    const struct mod_juno_xrp7724_config *config = module_ctx.config;
    struct juno_xrp7724_dev_ctx *ctx;

    /* Only bind in first round of calls and if the driver is available */
    if ((round > 0) || (module_ctx.driver_skipped))
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /* Bind to I2C HAL */
        status = fwk_module_bind(config->i2c_hal_id,
            FWK_ID_API(FWK_MODULE_IDX_I2C, MOD_I2C_API_IDX_I2C),
            &module_ctx.i2c_api);
        if (status != FWK_SUCCESS)
            return FWK_E_HANDLER;

        /* Bind to timer HAL */
        status = fwk_module_bind(config->timer_hal_id,
            FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
            &module_ctx.timer_api);
        if (status != FWK_SUCCESS)
            return FWK_E_HANDLER;

        return FWK_SUCCESS;
    }

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    if (ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_SENSOR) {
        status = fwk_module_bind(ctx->config->driver_response_id,
            ctx->config->driver_response_api_id,
            &module_ctx.sensor_driver_response_api);
        if (status != FWK_SUCCESS)
            return FWK_E_HANDLER;
    }

    return FWK_SUCCESS;
}

static int juno_xrp7724_process_bind_request(fwk_id_t source_id,
                                             fwk_id_t target_id,
                                             fwk_id_t api_id,
                                             const void **api)
{
    if (module_ctx.driver_skipped)
        return FWK_E_ACCESS;

    if (fwk_id_is_equal(api_id, mod_juno_xrp7724_api_id_system_mode))
        *api = &system_mode_api;
    else if (fwk_id_is_equal(api_id, mod_juno_xrp7724_api_id_sensor))
        *api = &sensor_driver_api;
    else
        return FWK_E_PARAM;

    return FWK_SUCCESS;
}

/*
 * Helper function to process the GPIO events
 */
static int juno_xrp7724_gpio_process_request(fwk_id_t id, int response_status)
{
    int status;
    struct fwk_event resp_event;
    struct juno_xrp7724_dev_ctx *ctx;
    const struct mod_juno_xrp7724_config *config = module_ctx.config;

    fwk_assert(fwk_module_is_valid_element_id(id));

    ctx = ctx_table + fwk_id_get_element_idx(id);

    switch (module_ctx.gpio_request) {
    case JUNO_XRP7724_GPIO_REQUEST_MODE_SHUTDOWN:
        status = set_gpio(config->gpio_mode_id, ctx);
        if (status != FWK_SUCCESS) {
            module_ctx.gpio_request = JUNO_XRP7724_GPIO_REQUEST_IDLE;

            return FWK_E_DEVICE;
        }

        module_ctx.gpio_request = JUNO_XRP7724_GPIO_REQUEST_ASSERT_SHUTDOWN;

        return FWK_SUCCESS;

    case JUNO_XRP7724_GPIO_REQUEST_ASSERT_SHUTDOWN:
        /*
         * The response_status parameter indicates if the I2C transaction to
         * select the shutdown mode has been successful.
         * The shutdown operation is aborted if the first command has not been
         * sent successfully.
         */
        if (response_status != FWK_SUCCESS)
            break;

        /*
         * The shutdown assert command is delayed to ensure that the mode change
         * has been applied by the hardware device.
         */
        module_ctx.timer_api->delay(config->timer_hal_id, GPIO_DELAY_US);

        /* Send the assert command */
        status = set_gpio(config->gpio_assert_id, ctx);
        if (status != FWK_SUCCESS)
            break;

        module_ctx.gpio_request = JUNO_XRP7724_GPIO_REQUEST_DONE;

        return FWK_SUCCESS;

    case JUNO_XRP7724_GPIO_REQUEST_ASSERT_COLD_RESET:
        /* Send the assert command */
        status = set_gpio(config->gpio_assert_id, ctx);
        if (status != FWK_SUCCESS) {
            module_ctx.gpio_request = JUNO_XRP7724_GPIO_REQUEST_IDLE;

            return FWK_E_DEVICE;
        }

        module_ctx.gpio_request = JUNO_XRP7724_GPIO_REQUEST_DONE;

        return FWK_SUCCESS;

    case JUNO_XRP7724_GPIO_REQUEST_DONE:
        if (response_status != FWK_SUCCESS)
            break;

        /*
         * Allow some time to the hardware to apply the reset or shutdown
         * command
         */
        module_ctx.timer_api->delay(config->timer_hal_id, GPIO_DELAY_US);

        /* The board should have been reset or shut down at this point */

        break;

    default:
        return FWK_E_PARAM;
    }

    /*
     * This part of the code is executed if an error occurred during the
     * system mode request.
     */

    module_ctx.gpio_request = JUNO_XRP7724_GPIO_REQUEST_IDLE;

    status = fwk_thread_get_delayed_response(id, ctx->cookie, &resp_event);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return fwk_thread_put_event(&resp_event);
}

static int juno_xrp7724_sensor_process_request(fwk_id_t id, int status)
{
    struct juno_xrp7724_dev_ctx *ctx;
    uint64_t temp = 0;
    int request_status = status;
    struct mod_sensor_driver_resp_params resp_params;
    const struct mod_juno_xrp7724_config *module_config = module_ctx.config;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    switch (module_ctx.sensor_request) {
    case JUNO_XRP7724_SENSOR_REQUEST_READ_VALUE:
        module_ctx.sensor_request = JUNO_XRP7724_SENSOR_REQUEST_CONVERT_VALUE;

        ctx->transmit_data[0] = SENSOR_READ_TEMP;

        request_status =
            module_ctx.i2c_api->transmit_then_receive_as_master(
                module_config->i2c_hal_id, module_config->slave_address,
                ctx->transmit_data, ctx->receive_data, SENSOR_WRITE_LENGTH,
                SENSOR_READ_LENGTH);
        if (request_status == FWK_SUCCESS)
            return FWK_SUCCESS;

        break;

    case JUNO_XRP7724_SENSOR_REQUEST_CONVERT_VALUE:
        /*
         * The request_status parameter contains the I2C transaction status.
         * The conversion is done if the read of the sensor value has been
         * successful.
         */
        if (request_status == FWK_SUCCESS) {
            temp = temperature_to_millidegree_celsius(
                ((uint16_t)ctx->receive_data[0] << 8) | ctx->receive_data[1]);
        }

        break;

    default:
        request_status = FWK_E_PARAM;
    }

    module_ctx.sensor_request = JUNO_XRP7724_SENSOR_REQUEST_IDLE;

    resp_params.status = request_status;
    resp_params.value = temp;

    module_ctx.sensor_driver_response_api->reading_complete(
        ctx->config->driver_response_id,
        &resp_params);

    return FWK_SUCCESS;
}

/*
 * This function is called when a request event is received for the XRP7724 and
 * when an I2C transaction is completed.
 */
static int juno_xrp7724_process_event(const struct fwk_event *event,
                                      struct fwk_event *resp_event)
{
    int status;
    struct juno_xrp7724_dev_ctx *ctx;
    struct mod_i2c_event_param *param =
        (struct mod_i2c_event_param *)event->params;

    fwk_assert(fwk_module_is_valid_element_id(event->target_id));

    ctx = ctx_table + fwk_id_get_element_idx(event->target_id);

    switch (ctx->config->type) {
    case MOD_JUNO_XRP7724_ELEMENT_TYPE_GPIO:
        status = juno_xrp7724_gpio_process_request(event->target_id,
            param->status);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;

        if (fwk_id_is_equal(event->id, juno_xrp7724_event_id_request)) {
            resp_event->is_delayed_response = true;
            ctx->cookie = event->cookie;
        }

        return FWK_SUCCESS;

    case MOD_JUNO_XRP7724_ELEMENT_TYPE_SENSOR:
        return juno_xrp7724_sensor_process_request(event->target_id,
            param->status);

    default:
        return FWK_E_PARAM;
    }

}

const struct fwk_module module_juno_xrp7724 = {
    .name = "JUNO XRP7724",
    .api_count = MOD_JUNO_XRP7724_API_IDX_COUNT,
    .event_count = JUNO_XRP7724_EVENT_IDX_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = juno_xrp7724_init,
    .element_init = juno_xrp7724_element_init,
    .bind = juno_xrp7724_bind,
    .process_bind_request = juno_xrp7724_process_bind_request,
    .process_event = juno_xrp7724_process_event,
};
