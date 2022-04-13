/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *  Thermal Management.
 *  The power allocation is in a separate unit. This module
 *  only probe the temperature and calculate the power budget. The allocation
 *  is done in `power_allocation`.
 *  It is meant to work as a performance plugin.
 */

#include "thermal_mgmt.h"

#if THERMAL_HAS_ASYNC_SENSORS
static const fwk_id_t mod_thermal_event_id_read_temp = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_THERMAL_MGMT,
    MOD_THERMAL_EVENT_IDX_READ_TEMP);
#endif

static struct mod_thermal_mgmt_ctx mod_ctx;

static inline struct mod_thermal_mgmt_dev_ctx *get_dev_ctx(unsigned int dom)
{
    return &mod_ctx.dev_ctx_table[dom];
}

static void pi_control(void)
{
    int32_t err, terr, pi_power;
    int32_t k_p, k_i;

    if (mod_ctx.cur_temp < mod_ctx.config->switch_on_temperature) {
        /* The PI loop is not activated */
        mod_ctx.integral_error = 0;
        mod_ctx.allocatable_power = (uint32_t)mod_ctx.config->tdp;

        return;
    }

    k_i = mod_ctx.config->k_integral;

    err = (int32_t)mod_ctx.config->control_temperature -
        (int32_t)mod_ctx.cur_temp;

    k_p = (err < 0) ? mod_ctx.config->k_p_overshoot :
                      mod_ctx.config->k_p_undershoot;

    /* Evaluate the integral term */
    if (((err > 0) && (mod_ctx.integral_error < (INT32_MAX - err))) ||
        ((err < 0) && (mod_ctx.integral_error > (INT32_MIN - err)))) {
        terr = mod_ctx.integral_error + err;

        if ((err < mod_ctx.config->integral_cutoff) &&
            (terr < mod_ctx.config->integral_max)) {
            /*
             * The error is below the cutoff value and,
             * the accumulated error is still within the maximum permissible
             * value, thus continue integration.
             */
            mod_ctx.integral_error = terr;
        }
    }

    pi_power = (k_p * err) + (k_i * mod_ctx.integral_error);

    if (pi_power + (int32_t)mod_ctx.config->tdp > 0) {
        mod_ctx.allocatable_power = pi_power + (uint32_t)mod_ctx.config->tdp;
    } else {
        mod_ctx.allocatable_power = 0;
    }
}

static int read_temperature(void)
{
#if THERMAL_HAS_ASYNC_SENSORS
    /* Initiate the temperature reading sequence */
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_THERMAL_MGMT),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_THERMAL_MGMT),
        .id = mod_thermal_event_id_read_temp,
    };

    return fwk_put_event(&event);
#else
    int status;
    uint64_t value;

    status = mod_ctx.sensor_api->get_data(
        mod_ctx.config->sensor_id, &mod_ctx.sensor_data);
    if (status == FWK_SUCCESS) {
        mod_ctx.cur_temp = (uint32_t)mod_ctx.sensor_data.value;

        mod_ctx.pi_control_needs_update = true;
    }

    return status;
#endif
}

static int pi_control_update(void)
{
    int status;

    mod_ctx.tick_counter++;
    if (mod_ctx.tick_counter > mod_ctx.config->slow_loop_mult) {
        mod_ctx.tick_counter = 0;

        if (mod_ctx.pi_control_needs_update) {
            /* The last reading was not processed */
            FWK_LOG_WARN("[TPM] Failed to process last reading\n");

            return FWK_E_PANIC;
        }

        /*
         * ASYNC: The new temperature is not yet available but should be ready
         * by next fast-loop tick.
         * SYNC: The new temperature is ready.
         *
         * Either way we need to attempt to continue to process the loop.
         */
        status = read_temperature();
        if (status != FWK_SUCCESS) {
            return FWK_E_DEVICE;
        }
    }

    if (mod_ctx.pi_control_needs_update) {
        mod_ctx.pi_control_needs_update = false;

        pi_control();

        mod_ctx.tot_spare_power = 0;
    }

    return FWK_SUCCESS;
}

/*
 * Thermal Management should be configured in such a way that this callback
 * is called once every performance update via the plugins handler and the data
 * contains all the performance domains info (see _TYPE_FULL view).
 */
static int thermal_update(struct perf_plugins_perf_update *data)
{
    int status;

    status = pi_control_update();
    if (status != FWK_SUCCESS) {
        return status;
    }

    distribute_power(data->level, data->adj_max_limit);

    return FWK_SUCCESS;
}

struct perf_plugins_api perf_plugins_api = {
    .update = thermal_update,
};

/*
 * Framework handler functions.
 */

static int thermal_mgmt_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    int dvfs_doms_count;

    mod_ctx.config = (struct mod_thermal_mgmt_config *)data;

    /* Assume TDP until PI loop is updated */
    mod_ctx.allocatable_power = (uint32_t)mod_ctx.config->tdp;

    dvfs_doms_count =
        fwk_module_get_element_count(FWK_ID_MODULE(FWK_MODULE_IDX_DVFS));
    if (dvfs_doms_count <= 0) {
        return FWK_E_SUPPORT;
    }

    mod_ctx.dvfs_doms_count = (unsigned int)dvfs_doms_count;

    mod_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(*mod_ctx.dev_ctx_table));

    mod_ctx.domain_count = element_count;

    power_allocation_set_shared_ctx(&mod_ctx);

    return FWK_SUCCESS;
}

static int thermal_mgmt_dev_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_thermal_mgmt_dev_config *config;
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    uint64_t sum_weights;

    if (data == NULL) {
        return FWK_E_PARAM;
    }

    config = (struct mod_thermal_mgmt_dev_config *)data;

    if (!fwk_module_is_valid_element_id(config->dvfs_domain_id)) {
        return FWK_E_PARAM;
    }

    dev_ctx = get_dev_ctx(fwk_id_get_element_idx(element_id));

    if (!fwk_id_type_is_valid(config->driver_id) ||
        fwk_id_is_equal(config->driver_id, FWK_ID_NONE)) {
        return FWK_E_PARAM;
    }

    dev_ctx->config = config;

    sum_weights =
        dev_ctx->config->weight * mod_ctx.config->tdp * mod_ctx.config->tdp;
    if (sum_weights > UINT32_MAX) {
        FWK_LOG_WARN(
            "[THERMAL] WARN: Possible overflow for device %u",
            fwk_id_get_element_idx(element_id));
    }

    return FWK_SUCCESS;
}

static int thermal_mgmt_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;

    if (round > 0) {
        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /* Bind to sensor */
        status = fwk_module_bind(
            mod_ctx.config->sensor_id,
            mod_sensor_api_id_sensor,
            &mod_ctx.sensor_api);
        if (status != FWK_SUCCESS) {
            return FWK_E_PANIC;
        }

        return FWK_SUCCESS;
    }

    dev_ctx = get_dev_ctx(fwk_id_get_element_idx(id));

    /* Bind to a respective thermal driver */
    status = fwk_module_bind(
        dev_ctx->config->driver_id,
        dev_ctx->config->driver_api_id,
        &dev_ctx->driver_api);
    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int thermal_mgmt_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &perf_plugins_api;

    return FWK_SUCCESS;
}

#if THERMAL_HAS_ASYNC_SENSORS
static int thermal_mgmt_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;

    if (fwk_id_is_equal(event->id, mod_thermal_event_id_read_temp)) {
        /* Temperature-reading event */
        status = mod_ctx.sensor_api->get_data(
            mod_ctx.config->sensor_id, &mod_ctx.sensor_data);
        if (status == FWK_SUCCESS) {
            mod_ctx.cur_temp = (uint32_t)mod_ctx.sensor_data.value;
        }
    } else if (fwk_id_is_equal(event->id, mod_sensor_event_id_read_request)) {
        /* Response event from Sensor HAL */
        if (mod_ctx.sensor_data.status == FWK_SUCCESS) {
            mod_ctx.cur_temp = (uint32_t)mod_ctx.sensor_data.value;
            status = FWK_SUCCESS;
        } else {
            status = FWK_E_DEVICE;
        }
    } else {
        status = FWK_E_PARAM;
    }

    if (status == FWK_SUCCESS) {
        mod_ctx.pi_control_needs_update = true;
    } else if (status == FWK_PENDING) {
        status = FWK_SUCCESS;
    }

    return status;
}
#endif

const struct fwk_module module_thermal_mgmt = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = (unsigned int)MOD_THERMAL_API_COUNT,
    .event_count = (unsigned int)MOD_THERMAL_EVENT_IDX_COUNT,
    .init = thermal_mgmt_init,
    .element_init = thermal_mgmt_dev_init,
    .bind = thermal_mgmt_bind,
    .process_bind_request = thermal_mgmt_process_bind_request,
#if THERMAL_HAS_ASYNC_SENSORS
    .process_event = thermal_mgmt_process_event,
#endif
};
