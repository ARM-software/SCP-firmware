/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *  Thermal Management.
 *  This module implements a basic power allocator based on the temperature.
 *  It is meant to work as a performance plugin.
 */

#include <mod_scmi_perf.h>
#include <mod_sensor.h>
#include <mod_thermal_mgmt.h>

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdint.h>
#include <stdlib.h>

#define THERMAL_HAS_ASYNC_SENSORS 1

enum mod_thermal_mgmt_event_idx {
    MOD_THERMAL_EVENT_IDX_READ_TEMP,

    MOD_THERMAL_EVENT_IDX_COUNT,
};

struct mod_thermal_mgmt_dev_ctx {
    /* Thermal device configuration */
    struct mod_thermal_mgmt_dev_config *config;

    /* Driver API */
    struct mod_thermal_mgmt_driver_api *driver_api;

    /* When the power allocated for this actor is less than what it requested */
    uint32_t power_deficit;

    /* What the demand power is for this actor */
    uint32_t demand_power;

    /* The power granted to an actor */
    uint32_t granted_power;

    /* The excess of power (initially) granted vs the demand power */
    uint32_t spare_power;
};

struct mod_thermal_mgmt_ctx {
    /* Tick counter for the slow loop */
    unsigned int tick_counter;

    /* Thermal module configuration */
    struct mod_thermal_mgmt_config *config;

    /* Current temperature */
    uint32_t cur_temp;

    /* Does the PI loop need update */
    bool pi_control_needs_update;

    /* Sensor API */
    const struct mod_sensor_api *sensor_api;

    /* The total power that can be re-distributed */
    uint32_t tot_spare_power;

    /* The total power that actors could still take */
    uint32_t tot_power_deficit;

    /* The total power carried over to the next fast-loop */
    uint32_t carry_over_power;

    /*
     * The power as: SUM(weight_actor * demand_power_actor)
     * Use to distribute power according to actors' weight.
     */
    uint32_t tot_weighted_demand_power;

    /* Total power budget */
    uint32_t allocatable_power;

    /* Integral (accumulated) error */
    int32_t integral_error;

    /* Number of DVFS domains */
    unsigned int dvfs_doms_count;

    /* Number of Thermal domains */
    unsigned int domain_count;

    /* Table of thermal actors */
    struct mod_thermal_mgmt_dev_ctx *dev_ctx_table;

    /* Sensor data */
    struct mod_sensor_data sensor_data;
};

#if THERMAL_HAS_ASYNC_SENSORS
static const fwk_id_t mod_thermal_event_id_read_temp = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_THERMAL_MGMT,
    MOD_THERMAL_EVENT_IDX_READ_TEMP);
#endif

static struct mod_thermal_mgmt_ctx mod_ctx;

/*
 * Forward declarations.
 */
static void get_actor_power(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx,
    uint32_t req_level);

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

static inline struct mod_thermal_mgmt_dev_ctx *get_dev_ctx(unsigned int dom)
{
    return &mod_ctx.dev_ctx_table[dom];
}

static struct mod_thermal_mgmt_dev_ctx *get_thermal_dev_ctx(
    unsigned int dvfs_dom_idx)
{
    unsigned int domain_idx;
    fwk_id_t dvfs_id =
        fwk_id_build_element_id(fwk_module_id_dvfs, dvfs_dom_idx);

    /* Attempt to find the thermal domain relevant to the given dvfs domain */
    for (domain_idx = 0; domain_idx < mod_ctx.domain_count; domain_idx++) {
        if (fwk_id_is_equal(
                mod_ctx.dev_ctx_table[domain_idx].config->dvfs_domain_id,
                dvfs_id)) {
            break;
        }
    }

    if (domain_idx == mod_ctx.domain_count) {
        /* There is no thermal domain corresponding to any DVFS domain */
        return NULL;
    }

    return get_dev_ctx(domain_idx);
}
/*
 * Perform the first round of power distribution.
 * An actor gets at most the power requested. Some actors may get less than
 * their demand power.
 */
static void allocate_power(struct mod_thermal_mgmt_dev_ctx *dev_ctx)
{
    dev_ctx->granted_power =
        ((dev_ctx->config->weight * dev_ctx->demand_power) *
         mod_ctx.allocatable_power) /
        mod_ctx.tot_weighted_demand_power;

    if (dev_ctx->granted_power > dev_ctx->demand_power) {
        dev_ctx->spare_power = dev_ctx->granted_power - dev_ctx->demand_power;
        dev_ctx->power_deficit = 0;

        dev_ctx->granted_power = dev_ctx->demand_power;
    } else {
        dev_ctx->spare_power = 0;
        dev_ctx->power_deficit = dev_ctx->demand_power - dev_ctx->granted_power;
    }

    mod_ctx.tot_spare_power += dev_ctx->spare_power;
    mod_ctx.tot_power_deficit += dev_ctx->power_deficit;
}

/*
 * Perform the second round of power distribution.
 * If an actor has received less than its demand power and there's a reserve of
 * power not allocated, it may get a fraction of that.
 */
static void re_allocate_power(struct mod_thermal_mgmt_dev_ctx *dev_ctx)
{
    if (mod_ctx.tot_spare_power == 0) {
        return;
    }

    if (dev_ctx->power_deficit > 0) {
        /*
         * The actor has been given less than requested, and it may still take
         * some power
         */
        dev_ctx->granted_power +=
            (dev_ctx->power_deficit * mod_ctx.tot_spare_power) /
            mod_ctx.tot_power_deficit;

        if (dev_ctx->granted_power > dev_ctx->demand_power) {
            mod_ctx.carry_over_power +=
                dev_ctx->granted_power - dev_ctx->demand_power;

            dev_ctx->granted_power = dev_ctx->demand_power;
        } else {
            /*
             * The actor has received the power it requested. The amount of
             * power left can be used in the next fast-loop.
             */
            mod_ctx.carry_over_power += dev_ctx->spare_power;
        }
    }
}

static void get_actor_power(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx,
    uint32_t req_level)
{
    struct mod_thermal_mgmt_driver_api *driver;
    fwk_id_t driver_id;

    driver = dev_ctx->driver_api;
    driver_id = dev_ctx->config->driver_id;

    dev_ctx->demand_power = driver->level_to_power(driver_id, req_level);
}

static void get_actor_level(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx,
    uint32_t *level)
{
    struct mod_thermal_mgmt_driver_api *driver;
    fwk_id_t driver_id;

    driver = dev_ctx->driver_api;
    driver_id = dev_ctx->config->driver_id;

    *level = driver->power_to_level(driver_id, dev_ctx->granted_power);
}

static inline bool is_power_request_satisfied(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx)
{
    return (dev_ctx->granted_power >= dev_ctx->demand_power);
}

static void distribute_power(uint32_t *perf_request, uint32_t *perf_limit)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    unsigned int dvfs_doms_count, dom;
    uint32_t new_perf_limit, dev_perf_request;

    dvfs_doms_count = mod_ctx.dvfs_doms_count;

    mod_ctx.tot_weighted_demand_power = 0;
    mod_ctx.tot_spare_power = 0;
    mod_ctx.tot_power_deficit = 0;

    /*
     * STEP 0:
     * Initialise the actors' demand power.
     */
    for (dom = 0; dom < dvfs_doms_count; dom++) {
        dev_ctx = get_thermal_dev_ctx(dom);
        if (dev_ctx == NULL) {
            continue;
        }

        /* Here we take into account limits already placed by other plugins */
        if (perf_request[dom] > perf_limit[dom]) {
            dev_perf_request = perf_limit[dom];
        } else {
            dev_perf_request = perf_request[dom];
        }

        get_actor_power(dev_ctx, dev_perf_request);

        mod_ctx.tot_weighted_demand_power +=
            dev_ctx->config->weight * dev_ctx->demand_power;
    }

    /*
     * STEP 1:
     * The power available is allocated in proportion to the actors' weight and
     * their power demand.
     */
    for (dom = 0; dom < dvfs_doms_count; dom++) {
        dev_ctx = get_thermal_dev_ctx(dom);
        if (dev_ctx == NULL) {
            continue;
        }

        allocate_power(dev_ctx);
    }

    /*
     * STEP 2:
     * A further allocation based on actors' power deficit, if any spare power
     * left.
     * Finally, get the corresponding performance level and place a new limit.
     */
    mod_ctx.tot_spare_power += mod_ctx.carry_over_power;
    mod_ctx.carry_over_power = 0;

    for (dom = 0; dom < dvfs_doms_count; dom++) {
        dev_ctx = get_thermal_dev_ctx(dom);
        if (dev_ctx == NULL) {
            continue;
        }

        re_allocate_power(dev_ctx);

        get_actor_level(dev_ctx, &new_perf_limit);

        /*
         * If we have been granted the power we requested (which is at most the
         * limit already placed by other plugins), then there's no need to limit
         * further.
         */
        if (!is_power_request_satisfied(dev_ctx) &&
            (new_perf_limit < perf_limit[dom])) {
            perf_limit[dom] = new_perf_limit;
        }
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
