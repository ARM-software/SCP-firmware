/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_id.h"

#include <mod_i2c.h>
#include <mod_juno_xrp7724.h>
#include <mod_psu.h>
#include <mod_sensor.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_multi_thread.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

/*
 * PSU Properties
 */
#define PSU_VOUT_PRESCALE_UV    2500 /* Low range */
#define PSU_VOUT_STEP_COARSE_UV (5 * PSU_VOUT_PRESCALE_UV)
#define PSU_VOUT_STEP_FINE_UV   PSU_VOUT_PRESCALE_UV
#define PSU_MVOUT_SCALE_READ    15
#define PSU_FINE_ADJUST_POS     12
#define PSU_MAX_FINE_ADJUST     0x7
#define PSU_CHANNEL_COUNT       4

#define PSU_MAX_VOUT_MV          1100

/* Ramping/Settling time when changing voltage in ms */
#define PSU_RAMP_DELAY_SET_MS    1

/* Ramping/Settling time when enabling a channel in ms */
#define PSU_RAMP_DELAY_ENABLE_MS 2

#define PSU_TARGET_MARGIN_MV     20

/*
 * Power Command Set
 */
#define PSU_PWR_GET_VOLTAGE_CHx 0x10
#define PSU_PWR_ENABLE_SUPPLY   0x1E
#define PSU_PWR_SET_VOLTAGE_CHx 0x20

#define PSU_WRITE_LENGTH 3
#define PSU_READ_LENGTH  2

#define PSU_CHANNEL_ENABLED 0x1
#define PSU_CHANNEL_DISABLED 0x0

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

enum juno_xrp7724_psu_request {
    JUNO_XRP7724_PSU_REQUEST_IDLE,
    JUNO_XRP7724_PSU_REQUEST_READ_VOLTAGE,
    JUNO_XRP7724_PSU_REQUEST_CONVERT_VOLTAGE,
    JUNO_XRP7724_PSU_REQUEST_SET_VOLTAGE,
    JUNO_XRP7724_PSU_REQUEST_WAIT_FOR_VOLTAGE,
    JUNO_XRP7724_PSU_REQUEST_COMPARE_VOLTAGE,
    JUNO_XRP7724_PSU_REQUEST_SET_ENABLED,
    JUNO_XRP7724_PSU_REQUEST_WAIT_FOR_ENABLED,
    JUNO_XRP7724_PSU_REQUEST_DONE_ENABLED,
};

struct psu_set_enabled_param {
    bool enabled;
};

struct psu_set_voltage_param {
    uint64_t voltage;
    uint16_t set_value;
};

struct juno_xrp7724_ctx {
    const struct mod_juno_xrp7724_config *config;
    const struct mod_sensor_driver_response_api *sensor_driver_response_api;
    const struct mod_psu_driver_response_api *psu_driver_response_api;
    const struct mod_i2c_api *i2c_api;
    const struct mod_timer_api *timer_api;
    const struct mod_sensor_api *adc_api;
    enum juno_xrp7724_gpio_request gpio_request;
    enum juno_xrp7724_sensor_request sensor_request;
    bool driver_skipped;
};

struct juno_xrp7724_dev_psu_ctx {
    /* Indicate for a PSU element whether the channel is enabled */
    bool is_psu_channel_enabled;
    fwk_id_t element_id;
    bool psu_set_enabled;

    /*  Cache for the voltage of the PSU element */
    uint64_t current_voltage;

    /*
     * This field is used when doing a set_voltage request to propagate the
     * voltage parameter through the processing of the request
     */
    uint64_t psu_set_voltage;
    enum juno_xrp7724_psu_request psu_request;
};

struct juno_xrp7724_dev_ctx {
    const struct mod_juno_xrp7724_dev_config *config;
    uint32_t cookie;
    uint8_t transmit_data[TRANSMIT_DATA_MAX];
    uint8_t receive_data[RECEIVE_DATA_MAX];
    fwk_id_t alarm_hal_id;
    const struct mod_timer_alarm_api *alarm_api;
    /* PSU data for the device */
    struct juno_xrp7724_dev_psu_ctx juno_xrp7724_dev_psu;
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
    if (status == FWK_PENDING)
        status = FWK_SUCCESS;
    else
        status = FWK_E_DEVICE;

    return status;
}

/* Helper function for the sensor API */
static uint64_t temperature_to_millidegree_celsius(uint16_t temp)
{
    return (temp * 5000ULL) - 272150;
}

/* Helper function for the PSU API */
static void alarm_callback(uintptr_t param)
{
    int status;
    struct fwk_event event;
    const struct juno_xrp7724_dev_ctx *ctx =
        (struct juno_xrp7724_dev_ctx *)param;

    event = (struct fwk_event) {
        .source_id = ctx->juno_xrp7724_dev_psu.element_id,
        .target_id = ctx->juno_xrp7724_dev_psu.element_id,
        .id = juno_xrp7724_event_id_request,
    };

    status = fwk_thread_put_event(&event);
    fwk_assert(status == FWK_SUCCESS);
}

/*
 * Functions for the system mode API
 */
static int juno_xrp7724_shutdown(void)
{
    int status;
    fwk_id_t gpio_id = fwk_id_build_element_id(fwk_module_id_juno_xrp7724,
        fwk_id_get_element_idx(module_ctx.config->gpio_mode_id));

    struct fwk_event event = (struct fwk_event) {
        .target_id = gpio_id,
        .id = juno_xrp7724_event_id_request,
    };

    fwk_assert(module_ctx.gpio_request == JUNO_XRP7724_GPIO_REQUEST_IDLE);

    /* Select the mode to perform a shutdown */
    module_ctx.gpio_request = JUNO_XRP7724_GPIO_REQUEST_MODE_SHUTDOWN;

    status = fwk_thread_put_event(&event);
    if (status == FWK_SUCCESS)
        return FWK_PENDING;

    return status;
}

static int juno_xrp7724_reset(void)
{
    int status;
    fwk_id_t gpio_id = fwk_id_build_element_id(fwk_module_id_juno_xrp7724,
        fwk_id_get_element_idx(module_ctx.config->gpio_assert_id));

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

    status = fwk_thread_put_event(&event);
    if (status == FWK_SUCCESS)
        return FWK_PENDING;

    return status;
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

    if (module_ctx.sensor_request != JUNO_XRP7724_SENSOR_REQUEST_IDLE)
        return FWK_E_BUSY;

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
    const struct juno_xrp7724_dev_ctx *ctx;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    *info = *(ctx->config->sensor_info);

    return FWK_SUCCESS;
}

static const struct mod_sensor_driver_api sensor_driver_api = {
    .get_value = juno_xrp7724_sensor_get_value,
    .get_info = juno_xrp7724_sensor_get_info,
};

/*
 * Driver functions for the PSU API
 */
static int juno_xrp7724_set_enabled(fwk_id_t id, bool enabled)
{
    int status;
    struct fwk_event event;
    struct juno_xrp7724_dev_ctx *ctx;
    struct psu_set_enabled_param *param =
        (struct psu_set_enabled_param *)event.params;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];
    fwk_assert(ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_PSU);

    /*
     * We only have a single I2C bus so only one I2C request is allowed
     * at any one time.
     */
    if (ctx->juno_xrp7724_dev_psu.psu_request != JUNO_XRP7724_PSU_REQUEST_IDLE)
        return FWK_E_BUSY;

    event = (struct fwk_event) {
        .target_id = id,
        .id = juno_xrp7724_event_id_request,
    };

    param->enabled = enabled;

    status = fwk_thread_put_event(&event);
    if (status != FWK_SUCCESS)
        return status;

    ctx->juno_xrp7724_dev_psu.psu_request =
        JUNO_XRP7724_PSU_REQUEST_SET_ENABLED;

    return FWK_PENDING;
}

static int juno_xrp7724_get_enabled(fwk_id_t id, bool *enabled)
{
    const struct juno_xrp7724_dev_ctx *ctx;

    if (enabled == NULL)
        return FWK_E_PARAM;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];
    fwk_assert(ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_PSU);

    *enabled = ctx->juno_xrp7724_dev_psu.is_psu_channel_enabled;

    return FWK_SUCCESS;
}

static int juno_xrp7724_set_voltage(fwk_id_t id, uint64_t voltage)
{
    int status;
    struct fwk_event event;
    uint16_t set_value;
    uint32_t mvref;
    uint8_t fine_adj;
    uint32_t coarse_val;
    struct juno_xrp7724_dev_ctx *ctx;
    struct psu_set_voltage_param *param =
        (struct psu_set_voltage_param *)event.params;

    /* Platform voltage cap */
    if (voltage > PSU_MAX_VOUT_MV)
        return FWK_E_RANGE;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];
    fwk_assert(ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_PSU);

    if (ctx->juno_xrp7724_dev_psu.psu_request != JUNO_XRP7724_PSU_REQUEST_IDLE)
        return FWK_E_BUSY;

    if (ctx->juno_xrp7724_dev_psu.current_voltage != 0 &&
        ctx->juno_xrp7724_dev_psu.current_voltage == voltage) {
        return FWK_SUCCESS;
    }

    /* Compute the number of coarse voltage steps */
    coarse_val = (voltage * 1000) / PSU_VOUT_STEP_COARSE_UV;

    /* Compute the resulting voltage in mV */
    mvref = (coarse_val * PSU_VOUT_STEP_COARSE_UV) / 1000;

    /*
     * Compute the number of fine steps required to reduce the error.
     * The truncation means in the worst case, we get just under 2.5mV of
     * undervoltage.
     */
    fine_adj = ((voltage - mvref) * 1000) / PSU_VOUT_STEP_FINE_UV;

    if (fine_adj > PSU_MAX_FINE_ADJUST)
        fine_adj = PSU_MAX_FINE_ADJUST;

    set_value = (fine_adj << PSU_FINE_ADJUST_POS) | coarse_val;

    event = (struct fwk_event) {
        .target_id = id,
        .id = juno_xrp7724_event_id_request,
    };

    param->set_value = set_value;
    param->voltage = voltage;

    status = fwk_thread_put_event(&event);
    if (status != FWK_SUCCESS)
        return status;

    ctx->juno_xrp7724_dev_psu.psu_request =
        JUNO_XRP7724_PSU_REQUEST_SET_VOLTAGE;

    return FWK_PENDING;
}

static int juno_xrp7724_get_voltage(fwk_id_t id, uint64_t *voltage)
{
    int status;
    struct fwk_event event;
    struct juno_xrp7724_dev_ctx *ctx;

    if (voltage == NULL)
        return FWK_E_PARAM;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];
    fwk_assert(ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_PSU);

    if (ctx->juno_xrp7724_dev_psu.psu_request != JUNO_XRP7724_PSU_REQUEST_IDLE)
        return FWK_E_BUSY;

    if (ctx->juno_xrp7724_dev_psu.current_voltage != 0) {
        *voltage = ctx->juno_xrp7724_dev_psu.current_voltage;
        return FWK_SUCCESS;
    }

    event = (struct fwk_event) {
        .target_id = id,
        .id = juno_xrp7724_event_id_request,
    };

    status = fwk_thread_put_event(&event);
    if (status != FWK_SUCCESS)
        return status;

    ctx->juno_xrp7724_dev_psu.psu_request =
        JUNO_XRP7724_PSU_REQUEST_READ_VOLTAGE;

    return FWK_PENDING;
}

static struct mod_psu_driver_api psu_driver_api = {
    .set_enabled = juno_xrp7724_set_enabled,
    .get_enabled = juno_xrp7724_get_enabled,
    .set_voltage = juno_xrp7724_set_voltage,
    .get_voltage = juno_xrp7724_get_voltage,
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

    if (ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_PSU) {
        if (ctx->config->psu_bus_idx >= PSU_CHANNEL_COUNT)
            return FWK_E_DATA;
        ctx->juno_xrp7724_dev_psu.is_psu_channel_enabled = true;
        ctx->juno_xrp7724_dev_psu.element_id = element_id;
        ctx->alarm_hal_id = ctx->config->alarm_hal_id;
    }

    return FWK_SUCCESS;
}

static int juno_xrp7724_bind(fwk_id_t id, unsigned int round)
{
    int status;
    const struct mod_juno_xrp7724_config *config = module_ctx.config;
    const struct juno_xrp7724_dev_ctx *ctx;

    /* Only bind in first round of calls and if the driver is available */
    if ((round > 0) || (module_ctx.driver_skipped))
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /* Bind to I2C HAL */
        status = fwk_module_bind(config->i2c_hal_id, mod_i2c_api_id_i2c,
            &module_ctx.i2c_api);
        if (status != FWK_SUCCESS)
            return FWK_E_HANDLER;

        /* Bind to timer HAL */
        status = fwk_module_bind(config->timer_hal_id, MOD_TIMER_API_ID_TIMER,
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

    if (ctx->config->type == MOD_JUNO_XRP7724_ELEMENT_TYPE_PSU) {
        status = fwk_module_bind(ctx->config->driver_response_id,
            ctx->config->driver_response_api_id,
            &module_ctx.psu_driver_response_api);
        if (status != FWK_SUCCESS)
            return FWK_E_HANDLER;

        status = fwk_module_bind(ctx->config->psu_adc_id,
            mod_sensor_api_id_sensor, &module_ctx.adc_api);
        if (status != FWK_SUCCESS)
            return FWK_E_HANDLER;

        /* Bind to the alarm HAL */
        status = fwk_module_bind(
            ctx->alarm_hal_id, MOD_TIMER_API_ID_ALARM, &ctx->alarm_api);
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
    if (fwk_id_is_equal(api_id, mod_juno_xrp7724_api_id_system_mode))
        *api = &system_mode_api;
    else if (fwk_id_is_equal(api_id, mod_juno_xrp7724_api_id_sensor))
        *api = &sensor_driver_api;
    else if (fwk_id_is_equal(api_id, mod_juno_xrp7724_api_id_psu))
        *api = &psu_driver_api;
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
    struct mod_sensor_driver_resp_params resp_params;
    const struct mod_juno_xrp7724_config *module_config = module_ctx.config;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    switch (module_ctx.sensor_request) {
    case JUNO_XRP7724_SENSOR_REQUEST_READ_VALUE:
        ctx->transmit_data[0] = SENSOR_READ_TEMP;
        status =
            module_ctx.i2c_api->transmit_then_receive_as_master(
                module_config->i2c_hal_id, module_config->slave_address,
                ctx->transmit_data, ctx->receive_data, SENSOR_WRITE_LENGTH,
                SENSOR_READ_LENGTH);
        if (status == FWK_PENDING) {
            module_ctx.sensor_request =
                JUNO_XRP7724_SENSOR_REQUEST_CONVERT_VALUE;
            return FWK_SUCCESS;
        }

        if (status != FWK_SUCCESS)
            break;

        /* FALLTHRU */
    case JUNO_XRP7724_SENSOR_REQUEST_CONVERT_VALUE:
        /*
         * The status parameter contains the I2C transaction status.
         * The conversion is done if the read of the sensor value has been
         * successful.
         */
        if (status == FWK_SUCCESS) {
            temp = temperature_to_millidegree_celsius(
                ((uint16_t)ctx->receive_data[0] << 8) | ctx->receive_data[1]);
        }

        break;

    default:
        status = FWK_E_PARAM;
    }

    module_ctx.sensor_request = JUNO_XRP7724_SENSOR_REQUEST_IDLE;

    resp_params.status = status;
    resp_params.value = temp;

    module_ctx.sensor_driver_response_api->reading_complete(
        ctx->config->driver_response_id,
        &resp_params);

    return FWK_SUCCESS;
}


static int juno_xrp7724_psu_process_request(fwk_id_t id,
    const uint8_t *event_params, int status)
{
    uint16_t set_value;
    uint64_t adc_val;
    struct juno_xrp7724_dev_ctx *ctx;
    struct mod_psu_driver_response driver_response;
    const struct mod_juno_xrp7724_config *module_config = module_ctx.config;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    switch (ctx->juno_xrp7724_dev_psu.psu_request) {
    case JUNO_XRP7724_PSU_REQUEST_READ_VOLTAGE:
        ctx->juno_xrp7724_dev_psu.psu_request =
            JUNO_XRP7724_PSU_REQUEST_CONVERT_VOLTAGE;

        ctx->transmit_data[0] = PSU_PWR_GET_VOLTAGE_CHx +
            ctx->config->psu_bus_idx;

        status = module_ctx.i2c_api->transmit_then_receive_as_master(
            module_config->i2c_hal_id, module_config->slave_address,
            ctx->transmit_data, ctx->receive_data, 1, PSU_READ_LENGTH);
        if (status == FWK_PENDING)
            return FWK_SUCCESS;

        if (status != FWK_SUCCESS)
            break;

        /* FALLTHRU */

    case JUNO_XRP7724_PSU_REQUEST_CONVERT_VOLTAGE:
        /*
         * If the I2C transaction completed successfully convert the voltage
         */
        if (status == FWK_SUCCESS) {
            driver_response.voltage = (((uint16_t)ctx->receive_data[0] << 8) |
                ctx->receive_data[1]) * PSU_MVOUT_SCALE_READ;

            ctx->juno_xrp7724_dev_psu.current_voltage =
                driver_response.voltage;
        }

        break;

    case JUNO_XRP7724_PSU_REQUEST_SET_VOLTAGE:
        ctx->juno_xrp7724_dev_psu.psu_request =
            JUNO_XRP7724_PSU_REQUEST_WAIT_FOR_VOLTAGE;

        set_value = ((struct psu_set_voltage_param *)event_params)->set_value;
        ctx->juno_xrp7724_dev_psu.psu_set_voltage =
            ((struct psu_set_voltage_param *)event_params)->voltage;

        ctx->transmit_data[0] = PSU_PWR_SET_VOLTAGE_CHx +
            ctx->config->psu_bus_idx;
        ctx->transmit_data[1] = (uint8_t)(set_value >> 8);
        ctx->transmit_data[2] = (uint8_t)(set_value & 0xFF);

        status = module_ctx.i2c_api->transmit_as_master(
            module_config->i2c_hal_id, module_config->slave_address,
            ctx->transmit_data, PSU_WRITE_LENGTH);
        if (status == FWK_PENDING)
            return FWK_SUCCESS;

        if (status != FWK_SUCCESS)
            break;

        /* FALLTHRU */

    case JUNO_XRP7724_PSU_REQUEST_WAIT_FOR_VOLTAGE:
        /* Check I2C response status */
        if (status != FWK_SUCCESS)
            break;

        /*
         * If the channel is currently enabled then the voltage will ramp to the
         * new value and it is necessary to wait for it to stabilize before
         * checking the final output voltage using the board's ADC.
         *
         * If the channel is not enabled then there is no need to wait or read
         * back the output voltage.
         */
        if (ctx->juno_xrp7724_dev_psu.is_psu_channel_enabled) {
            status = ctx->alarm_api->start(ctx->alarm_hal_id,
                PSU_RAMP_DELAY_SET_MS, MOD_TIMER_ALARM_TYPE_ONCE,
                alarm_callback, (uintptr_t)ctx);
            if (status != FWK_SUCCESS)
                break;

            ctx->juno_xrp7724_dev_psu.psu_request =
                 JUNO_XRP7724_PSU_REQUEST_COMPARE_VOLTAGE;

            return FWK_SUCCESS;
        }

        /*
         * If channel is not enabled there is nothing more to do.
         */
         ctx->juno_xrp7724_dev_psu.current_voltage =
            ctx->juno_xrp7724_dev_psu.psu_set_voltage;
         status = FWK_SUCCESS;

         break;

    case JUNO_XRP7724_PSU_REQUEST_COMPARE_VOLTAGE:
        status = module_ctx.adc_api->get_value(ctx->config->psu_adc_id,
                                              &adc_val);
        if (status != FWK_SUCCESS)
                break;

        if (((adc_val + PSU_TARGET_MARGIN_MV) <
            ctx->juno_xrp7724_dev_psu.psu_set_voltage) ||
            ((adc_val - PSU_TARGET_MARGIN_MV) >
            ctx->juno_xrp7724_dev_psu.psu_set_voltage)) {
                status = FWK_E_DEVICE;
        } else {
            ctx->juno_xrp7724_dev_psu.current_voltage =
                ctx->juno_xrp7724_dev_psu.psu_set_voltage;
        }

        break;

    case JUNO_XRP7724_PSU_REQUEST_SET_ENABLED:
        ctx->juno_xrp7724_dev_psu.psu_request =
            JUNO_XRP7724_PSU_REQUEST_WAIT_FOR_ENABLED;

        ctx->juno_xrp7724_dev_psu.psu_set_enabled =
            ((struct psu_set_enabled_param *)event_params)->enabled;
        ctx->transmit_data[0] = PSU_PWR_ENABLE_SUPPLY,
        ctx->transmit_data[1] = ctx->config->psu_bus_idx;
        ctx->transmit_data[2] = ctx->juno_xrp7724_dev_psu.psu_set_enabled ?
            PSU_CHANNEL_ENABLED : PSU_CHANNEL_DISABLED;

        status = module_ctx.i2c_api->transmit_as_master(
            module_config->i2c_hal_id, module_config->slave_address,
            ctx->transmit_data, PSU_WRITE_LENGTH);
        if (status == FWK_PENDING)
            return FWK_SUCCESS;

        if (status != FWK_SUCCESS)
            break;

        /* FALLTHRU */

    case JUNO_XRP7724_PSU_REQUEST_WAIT_FOR_ENABLED:
        /* Check I2C response status */
        if (status == FWK_SUCCESS) {
            /*
             * The transaction was successful so the stored channel
             * status can be updated.
             */
            ctx->juno_xrp7724_dev_psu.is_psu_channel_enabled =
                ctx->juno_xrp7724_dev_psu.psu_set_enabled;

            /*
             * Wait a fixed time for the voltage to stabilize.
             */
            if (ctx->juno_xrp7724_dev_psu.is_psu_channel_enabled) {
                status = ctx->alarm_api->start(
                    ctx->alarm_hal_id, PSU_RAMP_DELAY_ENABLE_MS,
                    MOD_TIMER_ALARM_TYPE_ONCE, alarm_callback, (uintptr_t)ctx);
                if (status != FWK_SUCCESS)
                    break;

                ctx->juno_xrp7724_dev_psu.psu_request =
                    JUNO_XRP7724_PSU_REQUEST_DONE_ENABLED;
                return FWK_SUCCESS;
            } else
                status = FWK_SUCCESS;
        }

        break;

    case JUNO_XRP7724_PSU_REQUEST_DONE_ENABLED:
        status = FWK_SUCCESS;
        break;

    default:
        return FWK_E_SUPPORT;
    }

    driver_response.status = status;
    ctx->juno_xrp7724_dev_psu.psu_request = JUNO_XRP7724_PSU_REQUEST_IDLE;

    module_ctx.psu_driver_response_api->respond(ctx->config->driver_response_id,
                                               driver_response);

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

    case MOD_JUNO_XRP7724_ELEMENT_TYPE_PSU:
        return juno_xrp7724_psu_process_request(event->target_id,
            event->params, param->status);
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
