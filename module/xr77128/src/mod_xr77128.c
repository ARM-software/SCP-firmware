/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     XR77128 driver
 */

#include <mod_cdns_i2c.h>
#include <mod_clock.h>
#include <mod_psu.h>
#include <mod_timer.h>
#include <mod_xr77128.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_interrupt.h>
#include <fwk_io.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#define XR77128_CHANNEL_ENABLED  0x1
#define XR77128_CHANNEL_DISABLED 0x0

/*
 * PSU properties
 */
#define XR77128_PSU_VOUT_PRESCALE_UV    2500 /* Low range */
#define XR77128_PSU_VOUT_STEP_COARSE_UV (5 * XR77128_PSU_VOUT_PRESCALE_UV)
#define XR77128_PSU_VOUT_STEP_FINE_UV   XR77128_PSU_VOUT_PRESCALE_UV
#define XR77128_PSU_MVOUT_SCALE_READ    15

/* Ramping/Settling time when changing voltage in ms */
#define XR77128_PSU_RAMP_DELAY_SET_MS 1

/* Ramping/Settling time when enabling a channel in ms */
#define XR77128_PSU_RAMP_DELAY_ENABLE_MS 2

enum xr77128_event_idx {
    XR77128_EVENT_IDX_REQUEST,
    XR77128_EVENT_IDX_COUNT,
};

enum xr77128_psu_request {
    XR77128_PSU_REQUEST_IDLE,
    XR77128_PSU_REQUEST_WAIT_FOR_IDLE,
    XR77128_PSU_REQUEST_GET_VOLTAGE,
    XR77128_PSU_REQUEST_CONVERT_VOLTAGE,
    XR77128_PSU_REQUEST_SET_VOLTAGE,
    XR77128_PSU_REQUEST_WAIT_FOR_VOLTAGE,
    XR77128_PSU_REQUEST_DONE_SET_VOLTAGE,
    XR77128_PSU_REQUEST_SET_ENABLED,
    XR77128_PSU_REQUEST_WAIT_FOR_ENABLED,
    XR77128_PSU_REQUEST_DONE_ENABLED,
};

struct psu_set_enabled_param {
    bool enabled;
};

struct psu_set_voltage_param {
    uint32_t voltage;
    uint16_t set_value;
};

/*! Identifier of the PSU (PMIC) driver API */
static const fwk_id_t mod_xr77128_api_id_psu =
    FWK_ID_API_INIT(FWK_MODULE_IDX_XR77128, MOD_XR77128_API_IDX_PSU);

static const fwk_id_t xr77128_event_id_request =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_XR77128, XR77128_EVENT_IDX_REQUEST);

struct xr77128_module_ctx {
    /* Table of device contexts */
    struct xr77128_dev_ctx *dev_ctx_table;

    /* Number of devices */
    unsigned int device_count;
};

struct xr77128_dev_ctx {
    /* Pointer to device configuration */
    const struct mod_xr77128_dev_config *config;

    /* Table of channel contexts */
    struct xr77128_channel_ctx *channel_ctx_table;

    /* Number of channels */
    unsigned int channel_count;

    /* I2C API */
    const struct mod_cdns_i2c_controller_api_polled *i2c_api;

    /* Response API to the PSU driver */
    const struct mod_psu_driver_response_api *psu_driver_response_api;

    /* Alarm API for voltage ramp-up delay */
    const struct mod_timer_alarm_api *alarm_api;

    /* Device state - set if any of the device channels is NOT idle */
    volatile bool device_busy;

    /* Storage for transmit data in I2C controller mode */
    uint8_t transmit_data[XR77128_DATA_SIZE_MAX];

    /* Storage for received data in I2C controller mode */
    uint8_t receive_data[XR77128_DATA_SIZE_MAX];
};

struct xr77128_channel_ctx {
    /* Pointer to channel configuration */
    const struct mod_xr77128_channel_config *config;

    /*  Required for generating event after alarm callback */
    fwk_id_t id;

    /* Indicate for a PSU driver whether the driver is enabled */
    bool psu_set_enabled;

    /* Indicates whether a given channel is enabled */
    bool is_channel_enabled;

    /*  Cache for the voltage of the PSU element */
    uint32_t current_voltage;

    /*
     * This field is used when doing a set_voltage request to propagate the
     * voltage parameter through the processing of the request
     */
    uint32_t psu_set_voltage;

    /* PSU operation requested */
    enum xr77128_psu_request psu_request;

    /*
     * Backed-up PSU request if any device channel is busy.
     *
     * This is required because occasionally the interleaved voltage change
     * requests targeted to two different channels go unhonoured. So, we save
     * the request if any of the channels is busy (hence the PSU), and send the
     * saved request once the PSU is idle again.
     */
    enum xr77128_psu_request saved_psu_request;
};

/* Module context */
static struct xr77128_module_ctx module_ctx;

/* Module helpers */
static struct xr77128_dev_ctx *get_dev_ctx(fwk_id_t id)
{
    fwk_assert(fwk_module_is_valid_element_id(id));

    return &(module_ctx.dev_ctx_table[fwk_id_get_element_idx(id)]);
}

static struct xr77128_channel_ctx *get_channel_ctx(fwk_id_t id)
{
    fwk_assert(fwk_module_is_valid_sub_element_id(id));

    struct xr77128_dev_ctx *dev_ctx = get_dev_ctx(id);

    return &(dev_ctx->channel_ctx_table[fwk_id_get_sub_element_idx(id)]);
}

static int xr77128_controller_tx_command(
    struct xr77128_dev_ctx *ctx,
    uint16_t length)
{
    int status;

    status = ctx->i2c_api->write(
        ctx->config->i2c_hal_id,
        ctx->config->target_addr,
        (char *)&ctx->transmit_data[0],
        length,
        true);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[XR77128] Error transmitting data! %d", status);
        return status;
    }

    return FWK_SUCCESS;
}

static int xr77128_controller_rx_response(
    struct xr77128_dev_ctx *ctx,
    uint16_t length)
{
    int status;

    status = ctx->i2c_api->read(
        ctx->config->i2c_hal_id,
        ctx->config->target_addr,
        (char *)&ctx->receive_data[0],
        length);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[XR77128] Error receiving data! %d", status);
    }

    return status;
}

static uint16_t calculate_pmic_set_voltage_val(uint32_t millivolt)
{
    uint16_t mvref;
    uint8_t fine_adj;
    uint16_t coarse_val;

    /* Compute the number of coarse voltage steps */
    coarse_val = (millivolt * 1000) / XR77128_PSU_VOUT_STEP_COARSE_UV;

    /* Compute the resulting voltage in mV */
    mvref = (coarse_val * XR77128_PSU_VOUT_STEP_COARSE_UV) / 1000;

    /*
     * Compute the number of fine steps required to reduce the error.
     * The truncation means in the worst case, we get just under 2.5mV of
     * undervoltage.
     */
    fine_adj = ((millivolt - mvref) * 1000) / XR77128_PSU_VOUT_STEP_FINE_UV;

    if (fine_adj > XR77128_MAX_FINE_ADJUST) {
        fine_adj = XR77128_MAX_FINE_ADJUST;
    }

    return (fine_adj << XR77128_FINE_ADJUST_POS) | coarse_val;
}

static void set_psu_request(
    fwk_id_t id,
    struct xr77128_channel_ctx *channel_ctx,
    enum xr77128_psu_request request)
{
    struct xr77128_dev_ctx *ctx;
    ctx = get_dev_ctx(id);

    if (ctx->device_busy) {
        channel_ctx->saved_psu_request = request;
        channel_ctx->psu_request = XR77128_PSU_REQUEST_WAIT_FOR_IDLE;
    } else {
        ctx->device_busy = true;
        channel_ctx->psu_request = request;
    }
}

/*
 * Driver functions for the PSU API
 */
static int xr77128_set_enabled(fwk_id_t id, bool enabled)
{
    int status;
    struct fwk_event event;
    struct xr77128_channel_ctx *channel_ctx;
    struct psu_set_enabled_param *param =
        (struct psu_set_enabled_param *)event.params;

    channel_ctx = get_channel_ctx(id);

    if (channel_ctx->psu_request != XR77128_PSU_REQUEST_IDLE) {
        return FWK_E_BUSY;
    }

    event = (struct fwk_event){
        .target_id = id,
        .id = xr77128_event_id_request,
    };

    param->enabled = enabled;

    status = fwk_put_event(&event);
    if (status != FWK_SUCCESS) {
        return status;
    }

    set_psu_request(id, channel_ctx, XR77128_PSU_REQUEST_SET_ENABLED);

    return FWK_PENDING;
}

static int xr77128_get_enabled(fwk_id_t id, bool *enabled)
{
    struct xr77128_channel_ctx *channel_ctx;

    if (enabled == NULL) {
        return FWK_E_PARAM;
    }

    channel_ctx = get_channel_ctx(id);

    *enabled = channel_ctx->is_channel_enabled;

    return FWK_SUCCESS;
}

static int xr77128_set_voltage(fwk_id_t id, uint32_t voltage)
{
    int status;
    struct fwk_event event;
    struct xr77128_channel_ctx *channel_ctx;
    struct psu_set_voltage_param *param =
        (struct psu_set_voltage_param *)event.params;

    channel_ctx = get_channel_ctx(id);

    if (voltage > channel_ctx->config->psu_max_vout) {
        return FWK_E_RANGE;
    }

    if (!channel_ctx->is_channel_enabled) {
        FWK_LOG_ERR("[XR77128] PSU has not been enabled yet");
        return FWK_E_STATE;
    }

    if ((channel_ctx->current_voltage != 0) &&
        (channel_ctx->current_voltage == voltage)) {
        return FWK_SUCCESS;
    }

    if (channel_ctx->psu_request != XR77128_PSU_REQUEST_IDLE) {
        return FWK_E_BUSY;
    }

    event = (struct fwk_event){
        .target_id = id,
        .id = xr77128_event_id_request,
    };

    param->set_value = calculate_pmic_set_voltage_val(voltage);
    param->voltage = voltage;

    status = fwk_put_event(&event);
    if (status != FWK_SUCCESS) {
        return status;
    }

    set_psu_request(id, channel_ctx, XR77128_PSU_REQUEST_SET_VOLTAGE);

    return FWK_PENDING;
}

static int xr77128_get_voltage(fwk_id_t id, uint32_t *voltage)
{
    int status;
    struct xr77128_channel_ctx *channel_ctx;

    if (voltage == NULL) {
        return FWK_E_PARAM;
    }

    channel_ctx = get_channel_ctx(id);

    if (channel_ctx->current_voltage != 0) {
        *voltage = channel_ctx->current_voltage;
        return FWK_SUCCESS;
    }

    if (channel_ctx->psu_request != XR77128_PSU_REQUEST_IDLE) {
        return FWK_E_BUSY;
    }

    struct fwk_event_light event = (struct fwk_event_light){
        .target_id = id,
        .id = xr77128_event_id_request,
    };

    status = fwk_put_event(&event);
    if (status != FWK_SUCCESS) {
        return status;
    }

    set_psu_request(id, channel_ctx, XR77128_PSU_REQUEST_GET_VOLTAGE);

    return FWK_PENDING;
}

static struct mod_psu_driver_api psu_driver_api = {
    .set_enabled = xr77128_set_enabled,
    .get_enabled = xr77128_get_enabled,
    .set_voltage = xr77128_set_voltage,
    .get_voltage = xr77128_get_voltage,
};

static int mod_xr77128_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    if (element_count == 0) {
        return FWK_E_DATA;
    }

    module_ctx.device_count = element_count;
    module_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct xr77128_dev_ctx));

    return FWK_SUCCESS;
}

static int mod_xr77128_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct xr77128_dev_ctx *ctx;
    struct xr77128_channel_ctx *channel_ctx;
    unsigned int channel_idx;

    if (data == NULL) {
        return FWK_E_DATA;
    }

    ctx = get_dev_ctx(element_id);

    ctx->config = (struct mod_xr77128_dev_config *)data;
    ctx->channel_count = sub_element_count;
    ctx->channel_ctx_table =
        fwk_mm_calloc(sub_element_count, sizeof(ctx->channel_ctx_table[0]));

    /* Initialize the channels built as sub-elements here */
    for (channel_idx = 0; channel_idx < sub_element_count; channel_idx++) {
        channel_ctx = &(ctx->channel_ctx_table[channel_idx]);
        channel_ctx->config = &(ctx->config->channels[channel_idx]);

        if (channel_ctx->config->psu_bus_idx >= XR77128_CHANNEL_COUNT) {
            return FWK_E_DATA;
        }

        channel_ctx->is_channel_enabled = channel_ctx->config->enabled;

        channel_ctx->id = fwk_id_build_sub_element_id(element_id, channel_idx);
    }

    return FWK_SUCCESS;
}

static int mod_xr77128_bind(fwk_id_t id, unsigned int round)
{
    int status;
    const struct mod_xr77128_dev_config *config;
    const struct xr77128_dev_ctx *ctx;
    struct xr77128_channel_ctx *channel_ctx;
    unsigned int channel_idx;

    if (round > 0) {
        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        ctx = get_dev_ctx(id);
        config = ctx->config;

        /* Bind to I2C HAL */
        status = fwk_module_bind(
            config->i2c_hal_id, config->i2c_api_id, &ctx->i2c_api);
        if (status != FWK_SUCCESS) {
            return FWK_E_HANDLER;
        }

        /* Bind sub-elements (PSU channels) */
        for (channel_idx = 0; channel_idx < ctx->channel_count; channel_idx++) {
            channel_ctx = &(ctx->channel_ctx_table[channel_idx]);

            status = fwk_module_bind(
                channel_ctx->config->driver_response_id,
                channel_ctx->config->driver_response_api_id,
                &ctx->psu_driver_response_api);
            if (status != FWK_SUCCESS) {
                return FWK_E_HANDLER;
            }

            /* Bind to the alarm HAL */
            status = fwk_module_bind(
                channel_ctx->config->alarm_hal_id,
                MOD_TIMER_API_ID_ALARM,
                &ctx->alarm_api);
            if (status != FWK_SUCCESS) {
                return FWK_E_HANDLER;
            }
        }
    }

    return FWK_SUCCESS;
}

static int mod_xr77128_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t id,
    fwk_id_t api_id,
    const void **api)
{
    if (!fwk_id_is_type(id, FWK_ID_TYPE_SUB_ELEMENT)) {
        return FWK_E_ACCESS;
    }

    if (fwk_id_is_equal(api_id, mod_xr77128_api_id_psu)) {
        *api = &psu_driver_api;
    } else {
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

/* Helper function for the PSU API */
static void alarm_callback(uintptr_t param)
{
    int status;
    struct xr77128_dev_ctx *ctx;
    struct xr77128_channel_ctx *channel_ctx =
        (struct xr77128_channel_ctx *)param;

    struct fwk_event_light event = (struct fwk_event_light){
        .source_id = channel_ctx->id,
        .target_id = channel_ctx->id,
        .id = xr77128_event_id_request,
    };

    ctx = get_dev_ctx(channel_ctx->id);
    ctx->device_busy = false;

    status = fwk_put_event(&event);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[XR77128] %s @%d", __func__, __LINE__);
    }
}

/* PSU request functions for event calls */
static int request_get_voltage(
    struct xr77128_dev_ctx *ctx,
    struct xr77128_channel_ctx *channel_ctx)
{
    int status;
    struct mod_psu_driver_response driver_response;

    ctx->transmit_data[0] =
        XR77128_PWR_READ_VOLTAGE_CHx + channel_ctx->config->psu_bus_idx;

    status = xr77128_controller_tx_command(ctx, 1);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[XR77128] %s @%d", __func__, __LINE__);
        return status;
    }

    status = xr77128_controller_rx_response(ctx, XR77128_RECEIVE_DATA_MAX);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[XR77128] %s @%d", __func__, __LINE__);
        return status;
    }

    driver_response.voltage = (uint32_t)(
        (((uint16_t)ctx->receive_data[0] << 8) | ctx->receive_data[1]) *
        XR77128_PSU_MVOUT_SCALE_READ);

    channel_ctx->current_voltage = driver_response.voltage;

    return FWK_SUCCESS;
}

static int request_set_voltage(
    const uint8_t *event_params,
    struct xr77128_dev_ctx *ctx,
    struct xr77128_channel_ctx *channel_ctx)
{
    int status;
    uint16_t set_value;

    set_value = ((struct psu_set_voltage_param *)event_params)->set_value;
    channel_ctx->psu_set_voltage =
        ((struct psu_set_voltage_param *)event_params)->voltage;

    ctx->transmit_data[0] =
        XR77128_PWR_SET_VOLTAGE_CHx + channel_ctx->config->psu_bus_idx;
    ctx->transmit_data[1] = (unsigned int)(set_value) >> 8;
    ctx->transmit_data[2] = (unsigned int)(set_value)&0xFFU;

    status = xr77128_controller_tx_command(ctx, XR77128_TRANSMIT_DATA_MAX);

    if (status != FWK_SUCCESS) {
        return status;
    }

    /*
     * The voltage will ramp to the new value and it is necessary to wait
     * for it to stabilize before checking the final output voltage using
     * the board's ADC.
     */
    status = ctx->alarm_api->start(
        channel_ctx->config->alarm_hal_id,
        XR77128_PSU_RAMP_DELAY_SET_MS,
        MOD_TIMER_ALARM_TYPE_ONCE,
        alarm_callback,
        (uintptr_t)channel_ctx);

    return status;
}

static int request_set_enabled(
    const uint8_t *event_params,
    struct xr77128_dev_ctx *ctx,
    struct xr77128_channel_ctx *channel_ctx)
{
    int status;

    channel_ctx->psu_set_enabled =
        ((struct psu_set_enabled_param *)event_params)->enabled;
    ctx->transmit_data[0] = XR77128_PWR_ENABLE_SUP,
    ctx->transmit_data[1] = channel_ctx->config->psu_bus_idx;
    ctx->transmit_data[2] = channel_ctx->psu_set_enabled ?
        XR77128_CHANNEL_ENABLED :
        XR77128_CHANNEL_DISABLED;

    status = xr77128_controller_tx_command(ctx, XR77128_TRANSMIT_DATA_MAX);

    if (status != FWK_SUCCESS) {
        return status;
    }

    /*
     * The transaction was successful so the stored channel
     * status can be updated.
     */
    channel_ctx->is_channel_enabled = channel_ctx->psu_set_enabled;

    /*
     * Wait a fixed time for the voltage to stabilize.
     */
    if (channel_ctx->is_channel_enabled) {
        status = ctx->alarm_api->start(
            channel_ctx->config->alarm_hal_id,
            XR77128_PSU_RAMP_DELAY_ENABLE_MS,
            MOD_TIMER_ALARM_TYPE_ONCE,
            alarm_callback,
            (uintptr_t)channel_ctx);
    }

    return status;
}

/*
 * This function is called when a request event is received for the XR77128 and
 * when an I2C transaction is completed.
 */
static int xr77128_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status = FWK_SUCCESS;
    struct xr77128_dev_ctx *ctx;
    struct xr77128_channel_ctx *channel_ctx;
    struct mod_psu_driver_response driver_response;

    ctx = get_dev_ctx(event->target_id);
    channel_ctx = get_channel_ctx(event->target_id);

    switch (channel_ctx->psu_request) {
    case XR77128_PSU_REQUEST_WAIT_FOR_IDLE:
        /*
         * If either of the PSU channels has been freed up, update the
         * psu_request with the saved request and mark the PSU busy.
         * Else, we simply enqueue the event again and hope to find the PSU
         * channels free in the next event run.
         */
        if (!ctx->device_busy) {
            /* Get the saved PSU request and set the PSU busy. */
            channel_ctx->psu_request = channel_ctx->saved_psu_request;
            ctx->device_busy = true;
        }

        status = fwk_put_event((struct fwk_event *)event);
        if (status == FWK_SUCCESS) {
            return status;
        }

        break;

    case XR77128_PSU_REQUEST_GET_VOLTAGE:
        status = request_get_voltage(ctx, channel_ctx);
        break;

    case XR77128_PSU_REQUEST_SET_VOLTAGE:
        channel_ctx->psu_request = XR77128_PSU_REQUEST_DONE_SET_VOLTAGE;
        status = request_set_voltage(event->params, ctx, channel_ctx);
        if (status == FWK_SUCCESS) {
            return status;
        }
        break;

    case XR77128_PSU_REQUEST_DONE_SET_VOLTAGE:
        /*
         * Voltage ramp up complete, nothing more to do.
         */
        channel_ctx->current_voltage = channel_ctx->psu_set_voltage;

        status = FWK_SUCCESS;

        break;

    case XR77128_PSU_REQUEST_SET_ENABLED:
        channel_ctx->psu_request = XR77128_PSU_REQUEST_DONE_ENABLED;
        status = request_set_enabled(event->params, ctx, channel_ctx);
        if (status == FWK_SUCCESS) {
            return status;
        }
        break;

    case XR77128_PSU_REQUEST_DONE_ENABLED:
        status = FWK_SUCCESS;
        break;

    default:
        status = FWK_E_SUPPORT;
        break;
    }

    driver_response.status = status;
    channel_ctx->psu_request = XR77128_PSU_REQUEST_IDLE;
    channel_ctx->saved_psu_request = XR77128_PSU_REQUEST_IDLE;

    ctx->device_busy = false;

    ctx->psu_driver_response_api->respond(
        channel_ctx->config->driver_response_id, driver_response);

    return FWK_SUCCESS;
}

const struct fwk_module module_xr77128 = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = (unsigned int)MOD_XR77128_API_COUNT,
    .init = mod_xr77128_init,
    .element_init = mod_xr77128_element_init,
    .bind = mod_xr77128_bind,
    .process_bind_request = mod_xr77128_process_bind_request,
    .process_event = xr77128_process_event,
    .event_count = (unsigned int)XR77128_EVENT_IDX_COUNT,
};
