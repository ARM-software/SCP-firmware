/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
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

static struct mod_thermal_mgmt_ctx *mod_thermal_ctx;

inline void power_allocation_set_shared_ctx(
    struct mod_thermal_mgmt_ctx *thermal_mgmt_mod_ctx)
{
    mod_thermal_ctx = thermal_mgmt_mod_ctx;
}

static struct mod_thermal_mgmt_dev_ctx *get_dev_ctx(unsigned int dom)
{
    return &mod_thermal_ctx->dev_ctx_table[dom];
}

static inline bool is_power_request_satisfied(
    struct mod_thermal_mgmt_dev_ctx *dev_ctx)
{
    return (dev_ctx->granted_power >= dev_ctx->demand_power);
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

/*
 * Perform the first round of power distribution.
 * An actor gets at most the power requested. Some actors may get less than
 * their demand power.
 */
static void allocate_power(struct mod_thermal_mgmt_dev_ctx *dev_ctx)
{
    dev_ctx->granted_power =
        ((dev_ctx->config->weight * dev_ctx->demand_power) *
         mod_thermal_ctx->allocatable_power) /
        mod_thermal_ctx->tot_weighted_demand_power;

    if (dev_ctx->granted_power > dev_ctx->demand_power) {
        dev_ctx->spare_power = dev_ctx->granted_power - dev_ctx->demand_power;
        dev_ctx->power_deficit = 0;

        dev_ctx->granted_power = dev_ctx->demand_power;
    } else {
        dev_ctx->spare_power = 0;
        dev_ctx->power_deficit = dev_ctx->demand_power - dev_ctx->granted_power;
    }

    mod_thermal_ctx->tot_spare_power += dev_ctx->spare_power;
    mod_thermal_ctx->tot_power_deficit += dev_ctx->power_deficit;
}

/*
 * Perform the second round of power distribution.
 * If an actor has received less than its demand power and there's a reserve of
 * power not allocated, it may get a fraction of that.
 */
static void re_allocate_power(struct mod_thermal_mgmt_dev_ctx *dev_ctx)
{
    if (mod_thermal_ctx->tot_spare_power == 0) {
        return;
    }

    if (dev_ctx->power_deficit > 0) {
        /*
         * The actor has been given less than requested, and it may still take
         * some power
         */
        dev_ctx->granted_power +=
            (dev_ctx->power_deficit * mod_thermal_ctx->tot_spare_power) /
            mod_thermal_ctx->tot_power_deficit;

        if (dev_ctx->granted_power > dev_ctx->demand_power) {
            mod_thermal_ctx->carry_over_power +=
                dev_ctx->granted_power - dev_ctx->demand_power;

            dev_ctx->granted_power = dev_ctx->demand_power;
        } else {
            /*
             * The actor has received the power it requested. The amount of
             * power left can be used in the next fast-loop.
             */
            mod_thermal_ctx->carry_over_power += dev_ctx->spare_power;
        }
    }
}

static struct mod_thermal_mgmt_dev_ctx *get_thermal_dev_ctx(
    unsigned int dvfs_dom_idx)
{
    unsigned int domain_idx;

    fwk_id_t dvfs_id =
        fwk_id_build_element_id(fwk_module_id_dvfs, dvfs_dom_idx);

    /* Attempt to find the thermal domain relevant to the given dvfs domain */
    for (domain_idx = 0; domain_idx < mod_thermal_ctx->domain_count;
         domain_idx++) {
        if (fwk_id_is_equal(
                mod_thermal_ctx->dev_ctx_table[domain_idx]
                    .config->dvfs_domain_id,
                dvfs_id)) {
            break;
        }
    }

    if (domain_idx == mod_thermal_ctx->domain_count) {
        /* There is no thermal domain corresponding to any DVFS domain */
        return NULL;
    }

    return get_dev_ctx(domain_idx);
}

void distribute_power(uint32_t *perf_request, uint32_t *perf_limit)
{
    struct mod_thermal_mgmt_dev_ctx *dev_ctx;
    unsigned int dvfs_doms_count, dom;
    uint32_t new_perf_limit, dev_perf_request;

    dvfs_doms_count = mod_thermal_ctx->dvfs_doms_count;

    mod_thermal_ctx->tot_weighted_demand_power = 0;
    mod_thermal_ctx->tot_spare_power = 0;
    mod_thermal_ctx->tot_power_deficit = 0;

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

        mod_thermal_ctx->tot_weighted_demand_power +=
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
    mod_thermal_ctx->tot_spare_power += mod_thermal_ctx->carry_over_power;
    mod_thermal_ctx->carry_over_power = 0;

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
