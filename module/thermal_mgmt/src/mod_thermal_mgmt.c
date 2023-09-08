/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
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

struct mod_thermal_mgmt_dev_ctx *get_dev_ctx(fwk_id_t id)
{
    return &mod_ctx.dev_ctx_table[fwk_id_get_element_idx(id)];
}

struct mod_thermal_mgmt_actor_ctx *get_actor_ctx(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx,
    unsigned int actor)
{
    return &dev_ctx->actor_ctx_table[actor];
}

static void pi_control(fwk_id_t id)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int32_t err, terr, pi_power;
    int32_t k_p, k_i;

    dev_ctx = get_dev_ctx(id);

    if (dev_ctx->cur_temp <
        dev_ctx->config->pi_controller.switch_on_temperature) {
        /* The PI loop is not activated */
        dev_ctx->integral_error = 0;
        dev_ctx->thermal_allocatable_power =
            (uint32_t)dev_ctx->config->cold_state_power;

        return;
    }

    k_i = dev_ctx->config->pi_controller.k_integral;

    err = (int32_t)dev_ctx->config->pi_controller.control_temperature -
        (int32_t)dev_ctx->cur_temp;

    k_p = (err < 0) ? dev_ctx->config->pi_controller.k_p_overshoot :
                      dev_ctx->config->pi_controller.k_p_undershoot;

    /* Evaluate the integral term */
    if (((err > 0) && (dev_ctx->integral_error < (INT32_MAX - err))) ||
        ((err < 0) && (dev_ctx->integral_error > (INT32_MIN - err)))) {
        terr = dev_ctx->integral_error + err;

        if ((err < dev_ctx->config->pi_controller.integral_cutoff) &&
            (terr < dev_ctx->config->pi_controller.integral_max)) {
            /*
             * The error is below the cutoff value and,
             * the accumulated error is still within the maximum permissible
             * value, thus continue integration.
             */
            dev_ctx->integral_error = terr;
        }
    }

    pi_power = (k_p * err) + (k_i * dev_ctx->integral_error);

    if (pi_power + (int32_t)dev_ctx->config->tdp > 0) {
        dev_ctx->thermal_allocatable_power =
            pi_power + (uint32_t)dev_ctx->config->tdp;
    } else {
        dev_ctx->thermal_allocatable_power = 0;
    }
}

static void thermal_protection(struct mod_thermal_mgmt_dev_ctx *dev_ctx)
{
    if (dev_ctx->cur_temp >=
        dev_ctx->config->temp_protection->crit_temp_threshold) {
        FWK_LOG_CRIT(
            "[THERMAL][%s] temp (%u) reached critical threshold!",
            fwk_module_get_element_name(dev_ctx->id),
            (unsigned int)dev_ctx->cur_temp);

        if (dev_ctx->thermal_protection_api->critical != NULL) {
            dev_ctx->thermal_protection_api->critical(
                dev_ctx->config->temp_protection->driver_id, dev_ctx->id);
        }
    } else if (
        dev_ctx->cur_temp >=
        dev_ctx->config->temp_protection->warn_temp_threshold) {
        FWK_LOG_WARN(
            "[THERMAL][%s] temp (%u) reached warning threshold!",
            fwk_module_get_element_name(dev_ctx->id),
            (unsigned int)dev_ctx->cur_temp);

        if (dev_ctx->thermal_protection_api->warning != NULL) {
            dev_ctx->thermal_protection_api->warning(
                dev_ctx->config->temp_protection->driver_id, dev_ctx->id);
        }
    }
}

static int read_temperature(fwk_id_t id)
{
#if THERMAL_HAS_ASYNC_SENSORS
    /* Initiate the temperature reading sequence */
    struct fwk_event_light event = {
        .source_id = id,
        .target_id = id,
        .id = mod_thermal_event_id_read_temp,
    };

    return fwk_put_event(&event);
#else
    int status;
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;

    dev_ctx = get_dev_ctx(id);

    status = dev_ctx->sensor_api->get_data(
        dev_ctx->config->sensor_id, &dev_ctx->sensor_data);
    if (status == FWK_SUCCESS) {
        dev_ctx->cur_temp = (uint32_t)dev_ctx->sensor_data.value;

        dev_ctx->control_needs_update = true;
    }

    return status;
#endif
}

static int control_update(fwk_id_t id)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;

    dev_ctx = get_dev_ctx(id);

    dev_ctx->tick_counter++;
    if (dev_ctx->tick_counter > dev_ctx->config->slow_loop_mult) {
        dev_ctx->tick_counter = 0;

        if (dev_ctx->control_needs_update) {
            /* The last reading was not processed */
            FWK_LOG_WARN("[TPM] Failed to process last reading");

            return FWK_E_PANIC;
        }

        /*
         * ASYNC: The new temperature is not yet available but should be ready
         * by next fast-loop tick.
         * SYNC: The new temperature is ready.
         *
         * Either way we need to attempt to continue to process the loop.
         */
        status = read_temperature(id);
        if (status != FWK_SUCCESS) {
            return FWK_E_DEVICE;
        }
    }

    if (dev_ctx->control_needs_update) {
        dev_ctx->control_needs_update = false;
        if (dev_ctx->config->thermal_actors_count > 0) {
            pi_control(id);
        }

        dev_ctx->tot_spare_power = 0;

        if (dev_ctx->config->temp_protection != NULL) {
            thermal_protection(dev_ctx);
        }
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
    unsigned int dev_idx;
    fwk_id_t dev_id;
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;

    for (dev_idx = 0; dev_idx < mod_ctx.dev_ctx_count; dev_idx++) {
        dev_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_THERMAL_MGMT, dev_idx);
        dev_ctx = get_dev_ctx(dev_id);

        status = control_update(dev_ctx->id);
        if (status != FWK_SUCCESS) {
            return status;
        }
        if (dev_ctx->config->thermal_actors_count > 0) {
            distribute_power(dev_ctx->id, data->level, data->adj_max_limit);
        }
    }

    return FWK_SUCCESS;
}

struct perf_plugins_api mod_thermal_perf_plugins_api = {
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
    mod_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct mod_thermal_mgmt_dev_ctx));
    mod_ctx.dev_ctx_count = element_count;

    return FWK_SUCCESS;
}

static int thermal_mgmt_dev_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_thermal_mgmt_dev_config *config;
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    struct mod_thermal_mgmt_actor_ctx *actor_ctx;

    uint64_t sum_weights;
    uint32_t actor;

    if (data == NULL) {
        return FWK_E_PARAM;
    }

    config = (struct mod_thermal_mgmt_dev_config *)data;
    dev_ctx = get_dev_ctx(element_id);
    dev_ctx->config = config;
    dev_ctx->id = element_id;

    if (dev_ctx->config->thermal_actors_count > 0) {
        /* Assume TDP until PI loop is updated */
        dev_ctx->thermal_allocatable_power = (uint32_t)dev_ctx->config->tdp;

        dev_ctx->actor_ctx_table = fwk_mm_calloc(
            dev_ctx->config->thermal_actors_count,
            sizeof(struct mod_thermal_mgmt_actor_ctx));
    } else if (dev_ctx->config->temp_protection == NULL) {
        /* There is neither temperature protection enabled nor actors defined */
        return FWK_E_PARAM;
    }

    for (actor = 0; actor < dev_ctx->config->thermal_actors_count; actor++) {
        /* Get actor context and set configuration */
        actor_ctx = get_actor_ctx(dev_ctx, actor);
        actor_ctx->config = &config->thermal_actors_table[actor];

        if (!fwk_module_is_valid_element_id(
                actor_ctx->config->dvfs_domain_id)) {
            return FWK_E_PARAM;
        }

        if (!fwk_id_type_is_valid(actor_ctx->config->driver_id) ||
            fwk_id_is_equal(actor_ctx->config->driver_id, FWK_ID_NONE)) {
            return FWK_E_PARAM;
        }

        sum_weights = actor_ctx->config->weight * config->tdp * config->tdp;

        if (sum_weights > UINT32_MAX) {
            FWK_LOG_WARN(
                "[THERMAL] WARN: Possible overflow for device %u",
                fwk_id_get_element_idx(element_id));
        }
    }

    return FWK_SUCCESS;
}

static int thermal_mgmt_bind(fwk_id_t id, unsigned int round)
{
    int status;
    unsigned int actor;
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    struct mod_thermal_mgmt_actor_ctx *actor_ctx;

    if (round > 0) {
        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    dev_ctx = get_dev_ctx(id);

    /* Bind to sensor */
    status = fwk_module_bind(
        dev_ctx->config->sensor_id,
        mod_sensor_api_id_sensor,
        &dev_ctx->sensor_api);
    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }

    if (dev_ctx->config->temp_protection != NULL) {
        /* Bind to thermal protection driver */
        status = fwk_module_bind(
            dev_ctx->config->temp_protection->driver_id,
            dev_ctx->config->temp_protection->driver_api_id,
            &dev_ctx->thermal_protection_api);
        if (status != FWK_SUCCESS) {
            return FWK_E_PANIC;
        }
    }

    /* Bind to a respective thermal driver */
    status = fwk_module_bind(
        FWK_ID_MODULE(fwk_id_get_module_idx(dev_ctx->config->driver_api_id)),
        dev_ctx->config->driver_api_id,
        &dev_ctx->driver_api);
    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }

    for (actor = 0; actor < dev_ctx->config->thermal_actors_count; actor++) {
        /* Get actor context and bind */
        actor_ctx = get_actor_ctx(dev_ctx, actor);

        if (actor_ctx->config->activity_factor != NULL) {
            /* Bind to thermal protection driver */
            status = fwk_module_bind(
                actor_ctx->config->activity_factor->driver_id,
                actor_ctx->config->activity_factor->driver_api_id,
                &actor_ctx->activity_api);
            if (status != FWK_SUCCESS) {
                return FWK_E_PANIC;
            }
        }
    }

    return FWK_SUCCESS;
}

static int thermal_mgmt_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **thermal_bind_request_api)
{
    *thermal_bind_request_api = &mod_thermal_perf_plugins_api;

    return FWK_SUCCESS;
}

#if THERMAL_HAS_ASYNC_SENSORS
static int thermal_mgmt_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    int status;

    dev_ctx = get_dev_ctx(event->target_id);

    if (fwk_id_is_equal(event->id, mod_thermal_event_id_read_temp)) {
        /* Temperature-reading event */
        status = dev_ctx->sensor_api->get_data(
            dev_ctx->config->sensor_id, &dev_ctx->sensor_data);
        if (status == FWK_SUCCESS) {
            dev_ctx->cur_temp = (uint32_t)dev_ctx->sensor_data.value;
        }
    } else if (fwk_id_is_equal(event->id, mod_sensor_event_id_read_request)) {
        /* Response event from Sensor HAL */
        if (dev_ctx->sensor_data.status == FWK_SUCCESS) {
            dev_ctx->cur_temp = (uint32_t)dev_ctx->sensor_data.value;
            status = FWK_SUCCESS;
        } else {
            status = FWK_E_DEVICE;
        }
    } else {
        status = FWK_E_PARAM;
    }

    if (status == FWK_SUCCESS) {
        dev_ctx->control_needs_update = true;
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
