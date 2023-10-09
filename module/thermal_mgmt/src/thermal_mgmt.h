/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef THERMAL_MGMT_H
#define THERMAL_MGMT_H

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

struct mod_thermal_mgmt_actor_ctx {
    /* Thermal actor configuration */
    struct mod_thermal_mgmt_actor_config *config;

    /* When the power allocated for this actor is less than what it requested */
    uint32_t power_deficit;

    /* What the demand power is for this actor */
    uint32_t demand_power;

    /* The power granted to an actor */
    uint32_t granted_power;

    /* The excess of power (initially) granted vs the demand power */
    uint32_t spare_power;

    /* Activity factor API */
    struct mod_thermal_mgmt_activity_factor_api *activity_api;
};

struct mod_thermal_mgmt_dev_ctx {
    /* Thermal device configuration */
    struct mod_thermal_mgmt_dev_config *config;

    /* Identifier of the thermal device */
    fwk_id_t id;

    /* Tick counter for the slow loop */
    unsigned int tick_counter;

    /* Current temperature */
    uint32_t cur_temp;

    /* Does the PI loop need update */
    bool control_needs_update;

    /* Sensor API */
    const struct mod_sensor_api *sensor_api;

    /* Driver API */
    struct mod_thermal_mgmt_driver_api *driver_api;

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

    /* Allocatable power calculated in the control loop */
    uint32_t thermal_allocatable_power;

    /* Total power budget */
    uint32_t allocatable_power;

    /* Integral (accumulated) error */
    int32_t integral_error;

#ifdef PID
    /* Derivative error */
    int32_t derivative_error;

   /* Last error for the derivative */
   int32_t last_error;
#endif

    /* Table of thermal actors */
    struct mod_thermal_mgmt_actor_ctx *actor_ctx_table;

    /* Sensor data */
    struct mod_sensor_data sensor_data;

    /* Thermal protection API */
    struct mod_thermal_mgmt_protection_api *thermal_protection_api;
};

struct mod_thermal_mgmt_ctx {
    /* Table of thermal domains */
    struct mod_thermal_mgmt_dev_ctx *dev_ctx_table;

    /* Number of thermal domains */
    unsigned int dev_ctx_count;
};

/* Helper functions */
struct mod_thermal_mgmt_dev_ctx *get_dev_ctx(fwk_id_t dom);

struct mod_thermal_mgmt_actor_ctx *get_actor_ctx(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx,
    unsigned int actor);

/* Power allocation */
void distribute_power(
    fwk_id_t id,
    uint32_t *perf_request,
    uint32_t *perf_limit);

#endif /* THERMAL_MGMT_H */
