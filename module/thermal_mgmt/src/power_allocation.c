/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *  Power Allocation.
 *  This unit performs power allocation among actors based on power budget
 *  from thermal and the power requested.
 */

#include "thermal_mgmt.h"

/*
 * Helper functions.
 */

static inline bool is_power_request_satisfied(
    struct mod_thermal_mgmt_actor_ctx *actor_ctx)
{
    return (actor_ctx->granted_power >= actor_ctx->demand_power);
}

static void get_actor_power(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx,
    struct mod_thermal_mgmt_actor_ctx *actor_ctx,
    uint32_t req_level)
{
    struct mod_thermal_mgmt_driver_api *driver;
    fwk_id_t driver_id;

    driver = dev_ctx->driver_api;
    driver_id = actor_ctx->config->driver_id;

    actor_ctx->demand_power = driver->level_to_power(driver_id, req_level);
}

static void get_actor_level(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx,
    struct mod_thermal_mgmt_actor_ctx *actor_ctx,
    uint32_t *level)
{
    struct mod_thermal_mgmt_driver_api *driver;
    fwk_id_t driver_id;

    driver = dev_ctx->driver_api;
    driver_id = actor_ctx->config->driver_id;

    *level = driver->power_to_level(driver_id, actor_ctx->granted_power);
}

/*
 * Perform the first round of power distribution.
 * An actor gets at most the power requested. Some actors may get less than
 * their demand power.
 */
static void allocate_power(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx,
    struct mod_thermal_mgmt_actor_ctx *actor_ctx)
{
    actor_ctx->granted_power =
        ((actor_ctx->config->weight * actor_ctx->demand_power) *
         dev_ctx->allocatable_power) /
        dev_ctx->tot_weighted_demand_power;

    if (actor_ctx->granted_power > actor_ctx->demand_power) {
        actor_ctx->spare_power =
            actor_ctx->granted_power - actor_ctx->demand_power;
        actor_ctx->power_deficit = 0;

        actor_ctx->granted_power = actor_ctx->demand_power;
    } else {
        actor_ctx->spare_power = 0;
        actor_ctx->power_deficit =
            actor_ctx->demand_power - actor_ctx->granted_power;
    }

    dev_ctx->tot_spare_power += actor_ctx->spare_power;
    dev_ctx->tot_power_deficit += actor_ctx->power_deficit;
}

/*
 * Perform the second round of power distribution.
 * If an actor has received less than its demand power and there's a reserve of
 * power not allocated, it may get a fraction of that.
 */
static void re_allocate_power(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx,
    struct mod_thermal_mgmt_actor_ctx *actor_ctx)
{
    if (dev_ctx->tot_spare_power == 0) {
        return;
    }

    if (actor_ctx->power_deficit > 0) {
        /*
         * The actor has been given less than requested, and it may still take
         * some power
         */
        actor_ctx->granted_power +=
            (actor_ctx->power_deficit * dev_ctx->tot_spare_power) /
            dev_ctx->tot_power_deficit;

        if (actor_ctx->granted_power > actor_ctx->demand_power) {
            dev_ctx->carry_over_power +=
                actor_ctx->granted_power - actor_ctx->demand_power;

            actor_ctx->granted_power = actor_ctx->demand_power;
        } else {
            /*
             * The actor has received the power it requested. The amount of
             * power left can be used in the next fast-loop.
             */
            dev_ctx->carry_over_power += actor_ctx->spare_power;
        }
    }
}

static unsigned int get_dvfs_domain_idx(
    struct mod_thermal_mgmt_actor_ctx *actor_ctx)
{
    return fwk_id_get_element_idx(actor_ctx->config->dvfs_domain_id);
}

void distribute_power(
    fwk_id_t id,
    const uint32_t *perf_request,
    uint32_t *perf_limit)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    struct mod_thermal_mgmt_actor_ctx *actor_ctx;
    int status;
    unsigned int actor_idx, dom;
    uint32_t new_perf_limit, dev_perf_request;
    uint32_t actors_count;
    uint32_t idle_power, prev_used_power;
    uint16_t activity;

    dev_ctx = get_dev_ctx(id);

    actors_count = dev_ctx->config->thermal_actors_count;
    idle_power = 0;
    dev_ctx->tot_weighted_demand_power = 0;
    dev_ctx->tot_spare_power = 0;
    dev_ctx->tot_power_deficit = 0;

    /*
     * STEP 0:
     * Initialise the actors' demand power.
     */
    for (actor_idx = 0; actor_idx < actors_count; actor_idx++) {
        actor_ctx = get_actor_ctx(dev_ctx, actor_idx);
        dom = get_dvfs_domain_idx(actor_ctx);

        /* Here we take into account limits already placed by other plugins */
        if (perf_request[dom] > perf_limit[dom]) {
            dev_perf_request = perf_limit[dom];
        } else {
            dev_perf_request = perf_request[dom];
        }

        get_actor_power(dev_ctx, actor_ctx, dev_perf_request);
        if (actor_ctx->activity_api != NULL) {
            status = actor_ctx->activity_api->get_activity_factor(
                actor_ctx->config->activity_factor->driver_id, &activity);
            if (status != FWK_SUCCESS) {
                FWK_LOG_INFO(
                    "[THERMAL] Failed to get activity factor (%u,%u)",
                    fwk_id_get_element_idx(id),
                    actor_idx);
                continue;
            }

            /* Calculate used power and accumulate idle power */
            prev_used_power = (actor_ctx->granted_power * activity) / 1024;
            idle_power += actor_ctx->granted_power - prev_used_power;
        }

        dev_ctx->tot_weighted_demand_power +=
            actor_ctx->config->weight * actor_ctx->demand_power;
    }

    dev_ctx->allocatable_power =
        dev_ctx->thermal_allocatable_power + idle_power;

    /*
     * STEP 1:
     * The power available is allocated in proportion to the actors' weight and
     * their power demand.
     */
    for (actor_idx = 0; actor_idx < actors_count; actor_idx++) {
        actor_ctx = get_actor_ctx(dev_ctx, actor_idx);

        allocate_power(dev_ctx, actor_ctx);
    }

    /*
     * STEP 2:
     * A further allocation based on actors' power deficit, if any spare power
     * left.
     * Finally, get the corresponding performance level and place a new limit.
     */
    dev_ctx->tot_spare_power += dev_ctx->carry_over_power;
    dev_ctx->carry_over_power = 0;

    for (actor_idx = 0; actor_idx < actors_count; actor_idx++) {
        actor_ctx = get_actor_ctx(dev_ctx, actor_idx);
        dom = get_dvfs_domain_idx(actor_ctx);

        re_allocate_power(dev_ctx, actor_ctx);

        get_actor_level(dev_ctx, actor_ctx, &new_perf_limit);

        /*
         * If we have been granted the power we requested (which is at most the
         * limit already placed by other plugins), then there's no need to limit
         * further.
         */
        if (!is_power_request_satisfied(actor_ctx) &&
            (new_perf_limit < perf_limit[dom])) {
            perf_limit[dom] = new_perf_limit;
        }
    }
}
