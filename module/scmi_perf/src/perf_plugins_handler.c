/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Performance Plugins Handler
 *
 *      This module is an extension of the SCMI-Performance module to support
 *      the addition of optional performance-related modules (plugins). Such
 *      modules can affect the final performance of one or more domains.
 *      It can also be used when the domains exposed by SCMI are different from
 *      the real frequency domains available in hardware as this layer can
 *      perform the domain aggregation.
 */

#include "perf_plugins_handler.h"
#include "scmi_perf.h"

#include <mod_dvfs.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
struct perf_plugins_dev_ctx {
    /*
     * Each logical domain within the same dependency - note that physical
     * domain index is equal to this count.
     */
    unsigned int log_dom_count;

    /*
     * Cause of the logical/physical domains handling, the size of each of the
     * tables will be (sub_element + 1) which is equivalent to
     * (number of logical domains + one physical domain).
     * This is done to optimise the internal handling of the data for both types
     * of domains.
     *
     * The last entry of each table is reserved for the physical domain.
     */
    struct perf_plugins_perf_update perf_table;

    /* For this domain, store max/min */
    uint32_t max;
    uint32_t lmax;
    uint32_t lmin;
};

struct perf_plugins_ctx {
    struct perf_plugins_api **plugins_api_table;

    const struct mod_scmi_perf_config *config;

    struct perf_plugins_dev_ctx *dev_ctx;

    /*
     * Table of dependency domain.
     *
     * It is an artefact to contain both the information of physical and
     * logical within the same identifier in the following way:
     * FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, physical_domain, logical_domain)
     *
     * This helps the internal logic for handling the domain aggregation back
     * and forth with scmi-perf.
     * It assumes that DVFS ignores the identifier to be a sub-element.
     */
    fwk_id_t *dep_id_table;

    struct perf_plugins_perf_update full_perf_table;

    size_t dvfs_doms_count;
};

static struct perf_plugins_ctx perf_plugins_ctx;

static inline struct perf_plugins_dev_ctx *perf_ph_get_ctx(fwk_id_t domain_id)
{
    return &perf_plugins_ctx.dev_ctx[fwk_id_get_element_idx(domain_id)];
}

static inline fwk_id_t get_phy_domain_id(fwk_id_t domain_id)
{
    return FWK_ID_ELEMENT(
        FWK_MODULE_IDX_DVFS, fwk_id_get_element_idx(domain_id));
}

fwk_id_t perf_plugins_get_dependency_id(unsigned int dom_idx)
{
    return perf_plugins_ctx.dep_id_table[dom_idx];
}

static inline bool are_limits_valid(uint32_t min, uint32_t max)
{
    return (max > min);
}

static int plugin_set_limits(struct plugin_limits_req *data)
{
    struct perf_plugins_dev_ctx *dev_ctx;
    fwk_id_t domain_id;
    int status;

    if (data == NULL) {
        return FWK_E_PARAM;
    }

    struct mod_scmi_perf_level_limits req_limits = {
        .minimum = data->min_limit,
        .maximum = data->max_limit,
    };

    domain_id = get_phy_domain_id(data->domain_id);
    dev_ctx = perf_ph_get_ctx(domain_id);

    /* Compare with the aggregated value stored for this physical domain */
    if (req_limits.maximum > dev_ctx->lmax) {
        req_limits.maximum = dev_ctx->lmax;
    }

    if (req_limits.minimum < dev_ctx->lmin) {
        req_limits.minimum = dev_ctx->lmin;
    }

    if (!are_limits_valid(req_limits.minimum, req_limits.maximum)) {
        return FWK_E_PARAM;
    }

    status = perf_set_limits(domain_id, 0, &req_limits);
    if (status != FWK_SUCCESS) {
        return FWK_E_DEVICE;
    }

    return FWK_SUCCESS;
}

static struct perf_plugins_handler_api handler_api = {
    .plugin_set_limits = plugin_set_limits,
};

/*
 * Write back the adjusted values for use by the next plugins.
 * Note that for logical domains, the aggregated values are written.
 */
static void write_back_adj_values(
    struct perf_plugins_dev_ctx *dev_ctx,
    size_t phy_dom_idx)
{
    for (unsigned int i = 0; i < (dev_ctx->log_dom_count + 1); i++) {
        dev_ctx->perf_table.adj_min_limit[i] = dev_ctx->lmin;
        dev_ctx->perf_table.adj_max_limit[i] = dev_ctx->lmax;
    }

    perf_plugins_ctx.full_perf_table.adj_min_limit[phy_dom_idx] = dev_ctx->lmin;
    perf_plugins_ctx.full_perf_table.adj_max_limit[phy_dom_idx] = dev_ctx->lmax;
}

static void plugins_policy_sync_level_limits(
    struct perf_plugins_dev_ctx *dev_ctx,
    enum plugin_domain_type dom_type)
{
    uint32_t *adj_max_limit_table, *adj_min_limit_table;
    uint32_t *adj_min_lim_full_table, *adj_max_lim_full_table;
    uint32_t needle_lim_max, needle_lim_min;
    unsigned int phy_dom, phy_dom_idx;
    uint32_t *level_table, *min_limit_table, *max_limit_table;

    adj_min_lim_full_table = perf_plugins_ctx.full_perf_table.adj_min_limit;
    adj_max_lim_full_table = perf_plugins_ctx.full_perf_table.adj_max_limit;

    adj_max_limit_table = dev_ctx->perf_table.adj_max_limit;
    adj_min_limit_table = dev_ctx->perf_table.adj_min_limit;

    if (dom_type != PERF_PLUGIN_DOM_TYPE_FULL) {
        phy_dom = dev_ctx->log_dom_count;
        level_table = dev_ctx->perf_table.level;
        min_limit_table = dev_ctx->perf_table.min_limit;
        max_limit_table = dev_ctx->perf_table.max_limit;

        /*
         * Conservative approach for adjusted values:
         * - pick the max for the min limit
         * - pick the min for the max limit
         */
        needle_lim_min = min_limit_table[phy_dom];
        needle_lim_max = max_limit_table[phy_dom];

        /* Find min/max on adjusted values, physical included */
        for (unsigned int i = 0; i < (phy_dom + 1); i++) {
            if (adj_min_limit_table[i] >= needle_lim_min) {
                needle_lim_min = adj_min_limit_table[i];
            }

            if (adj_max_limit_table[i] <= needle_lim_max) {
                needle_lim_max = adj_max_limit_table[i];
            }
        }

        /*
         * Now we apply the same policy against the temporary results for the
         * plugins updates so far.
         */
        if (needle_lim_min > dev_ctx->lmin) {
            dev_ctx->lmin = needle_lim_min;
        }
        if (needle_lim_max < dev_ctx->lmax) {
            dev_ctx->lmax = needle_lim_max;
        }

        dev_ctx->max = level_table[phy_dom];

        phy_dom_idx = fwk_id_get_element_idx(dev_ctx->perf_table.domain_id);
        write_back_adj_values(dev_ctx, (size_t)phy_dom_idx);

    } else {
        /*
         * This needs to iterate over all the domains, since we are at the last
         * domain and this plugin may have changed the limits for all the
         * domains.
         */
        for (size_t i = 0; i < perf_plugins_ctx.dvfs_doms_count; i++) {
            dev_ctx = perf_ph_get_ctx(FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, i));

            /* Update with adjusted min-max */
            if (adj_min_lim_full_table[i] > dev_ctx->lmin) {
                dev_ctx->lmin = adj_min_lim_full_table[i];
            }
            if (adj_max_lim_full_table[i] < dev_ctx->lmax) {
                dev_ctx->lmax = adj_max_lim_full_table[i];
            }

            dev_ctx->max = perf_plugins_ctx.full_perf_table.level[i];

            write_back_adj_values(dev_ctx, i);
        }
    }
}

static void plugins_policy_update(struct fc_perf_update *fc_update)
{
    struct perf_plugins_dev_ctx *dev_ctx;

    dev_ctx = perf_ph_get_ctx(fc_update->domain_id);

    fc_update->level = dev_ctx->max;
    fc_update->adj_min_limit = dev_ctx->lmin;
    fc_update->adj_max_limit = are_limits_valid(dev_ctx->lmin, dev_ctx->lmax) ?
        dev_ctx->lmax :
        dev_ctx->lmin;
}

static void plugins_policy_update_no_plugins(struct fc_perf_update *fc_update)
{
    struct perf_plugins_dev_ctx *dev_ctx;
    unsigned int phy_dom;

    dev_ctx = perf_ph_get_ctx(fc_update->domain_id);

    if (dev_ctx->log_dom_count == 1) {
        /* No plugins, no logical domains */
        fc_update->adj_max_limit = fc_update->max_limit;
        fc_update->adj_min_limit = fc_update->min_limit;
    } else {
        /* No plugins, with logical domains */
        phy_dom = dev_ctx->log_dom_count;

        fc_update->level = dev_ctx->perf_table.level[phy_dom];
        fc_update->adj_max_limit = dev_ctx->perf_table.adj_max_limit[phy_dom];
        fc_update->adj_min_limit = dev_ctx->perf_table.adj_min_limit[phy_dom];
    }
}

static inline void get_perf_table(
    struct perf_plugins_perf_update *dom,
    struct perf_plugins_dev_ctx *ctx,
    unsigned int dom_ix)
{
    dom->level = &ctx->perf_table.level[dom_ix];
    dom->max_limit = &ctx->perf_table.max_limit[dom_ix];
    dom->adj_max_limit = &ctx->perf_table.adj_max_limit[dom_ix];
    dom->min_limit = &ctx->perf_table.min_limit[dom_ix];
    dom->adj_min_limit = &ctx->perf_table.adj_min_limit[dom_ix];
}

static void assign_data_for_plugins(
    fwk_id_t id,
    struct perf_plugins_perf_update *snapshot,
    enum plugin_domain_type dom_type)
{
    struct perf_plugins_dev_ctx *dev_ctx;
    unsigned int dom_idx;

    dev_ctx = perf_ph_get_ctx(id);

    /* Determine the first entry on the table */
    if (dom_type == PERF_PLUGIN_DOM_TYPE_FULL) {
        snapshot->level = &perf_plugins_ctx.full_perf_table.level[0];

        snapshot->max_limit = &perf_plugins_ctx.full_perf_table.max_limit[0];
        snapshot->min_limit = &perf_plugins_ctx.full_perf_table.min_limit[0];

        snapshot->adj_max_limit =
            &perf_plugins_ctx.full_perf_table.adj_max_limit[0];
        snapshot->adj_min_limit =
            &perf_plugins_ctx.full_perf_table.adj_min_limit[0];

        return;
    } else if (dom_type == PERF_PLUGIN_DOM_TYPE_LOGICAL) {
        /* Provide the beginning of the table */
        dom_idx = 0;
    } else {
        /* Provide the last column of the table */
        dom_idx = dev_ctx->log_dom_count;
    }

    /* Alter the module identifier only */
    snapshot->domain_id = FWK_ID_SUB_ELEMENT(
        FWK_MODULE_IDX_SCMI_PERF,
        fwk_id_get_element_idx(id),
        fwk_id_get_sub_element_idx(id));

    get_perf_table(snapshot, dev_ctx, dom_idx);
}

static inline void domain_aggregate(
    struct perf_plugins_perf_update *this_dom,
    struct perf_plugins_perf_update *phy_dom)
{
    /*
     * Choose the best values of performance for the physical domain which would
     * satisfy all the logical domains.
     *
     * - pick the max for the level
     * - pick the max for the min limit
     * - pick the min for the max limit
     */
    if (this_dom->level[0] > phy_dom->level[0]) {
        phy_dom->level[0] = this_dom->level[0];
    }

    if (this_dom->max_limit[0] < phy_dom->max_limit[0]) {
        phy_dom->max_limit[0] = this_dom->max_limit[0];
    }

    if (this_dom->min_limit[0] > phy_dom->min_limit[0]) {
        phy_dom->min_limit[0] = this_dom->min_limit[0];
    }
}

static void store_and_aggregate(struct fc_perf_update *fc_update)
{
    unsigned int this_dom_idx, phy_dom_idx;
    struct perf_plugins_dev_ctx *dev_ctx;
    struct perf_plugins_perf_update this_dom;
    struct perf_plugins_perf_update phy_dom;
    unsigned int phy_ix;

    dev_ctx = perf_ph_get_ctx(fc_update->domain_id);

    this_dom_idx = fwk_id_get_sub_element_idx(fc_update->domain_id);
    phy_dom_idx = dev_ctx->log_dom_count;

    get_perf_table(&this_dom, dev_ctx, this_dom_idx);
    get_perf_table(&phy_dom, dev_ctx, phy_dom_idx);

    this_dom.level[0] = fc_update->level;
    this_dom.max_limit[0] = fc_update->max_limit;
    this_dom.min_limit[0] = fc_update->min_limit;
    this_dom.adj_max_limit[0] = fc_update->max_limit;
    this_dom.adj_min_limit[0] = fc_update->min_limit;

    if (this_dom_idx == 0) {
        /* Init max/min for physical domain at the 1st iteration */
        phy_dom.level[0] = 0;
        phy_dom.max_limit[0] = UINT32_MAX;
        phy_dom.min_limit[0] = 0;

        /* as well as ctx */
        dev_ctx->lmin = 0;
        dev_ctx->lmax = UINT32_MAX;
    }

    domain_aggregate(&this_dom, &phy_dom);

    phy_dom.adj_max_limit[0] = phy_dom.max_limit[0];
    phy_dom.adj_min_limit[0] = phy_dom.min_limit[0];

    if (this_dom_idx == (phy_dom_idx - 1)) {
        /* Copy values, all-performance table */
        phy_ix = fwk_id_get_element_idx(fc_update->domain_id);

        perf_plugins_ctx.full_perf_table.level[phy_ix] = phy_dom.level[0];

        perf_plugins_ctx.full_perf_table.max_limit[phy_ix] =
            phy_dom.max_limit[0];
        perf_plugins_ctx.full_perf_table.min_limit[phy_ix] =
            phy_dom.min_limit[0];

        perf_plugins_ctx.full_perf_table.adj_max_limit[phy_ix] =
            phy_dom.adj_max_limit[0];
        perf_plugins_ctx.full_perf_table.adj_min_limit[phy_ix] =
            phy_dom.adj_min_limit[0];
    }
}

void perf_plugins_handler_update(
    unsigned int perf_dom_idx,
    struct fc_perf_update *fc_update)
{
    struct perf_plugins_api *api;
    struct perf_plugins_perf_update perf_snapshot;
    unsigned int this_dom_idx, last_logical_dom_idx;
    struct perf_plugins_dev_ctx *dev_ctx;
    enum plugin_domain_type dom_type;
    const struct mod_scmi_perf_config *config;
    int status;

    store_and_aggregate(fc_update);

    dev_ctx = perf_ph_get_ctx(fc_update->domain_id);

    this_dom_idx = fwk_id_get_sub_element_idx(fc_update->domain_id);
    last_logical_dom_idx = dev_ctx->log_dom_count - 1;

    config = perf_plugins_ctx.config;

    /*
     * EITHER last logical domain OR it's only physical domain,
     * call the plugin - if any - with the data snapshot for this domain.
     */
    if ((this_dom_idx == last_logical_dom_idx) &&
        (config->plugins_count != 0)) {
        for (size_t i = 0; i < config->plugins_count; i++) {
            dom_type = config->plugins[i].dom_type;

            if ((perf_dom_idx != (config->perf_doms_count - 1)) &&
                (dom_type == PERF_PLUGIN_DOM_TYPE_FULL)) {
                /*
                 * For plugins that need the full snapshot, wait until the
                 * last performance domain.
                 */
                continue;
            }

            assign_data_for_plugins(
                fc_update->domain_id, &perf_snapshot, dom_type);

            api = perf_plugins_ctx.plugins_api_table[i];
            status = api->update(&perf_snapshot);
            if (status != FWK_SUCCESS) {
                FWK_LOG_DEBUG(
                    "[P-Handler] Update: Plugin%u returned error %i",
                    (unsigned int)i,
                    status);
            }

            plugins_policy_sync_level_limits(dev_ctx, dom_type);
        }
    }
}

void perf_plugins_handler_get(
    unsigned int perf_dom_idx,
    struct fc_perf_update *fc_update)
{
    unsigned int this_dom_idx, last_logical_dom_idx;
    const struct mod_scmi_perf_config *config;
    struct perf_plugins_dev_ctx *dev_ctx;

    dev_ctx = perf_ph_get_ctx(fc_update->domain_id);

    this_dom_idx = fwk_id_get_sub_element_idx(fc_update->domain_id);
    last_logical_dom_idx = dev_ctx->log_dom_count - 1;

    if (this_dom_idx != last_logical_dom_idx) {
        /*
         * Any other logical domain, no need to forward the request to DVFS,
         * thus use previous values.
         */
        fc_update->level = dev_ctx->max;
        fc_update->adj_max_limit = dev_ctx->lmax;
        fc_update->adj_min_limit = dev_ctx->lmin;
    } else {
        config = perf_plugins_ctx.config;

        if (config->plugins_count == 0) {
            /* domains coordination only */
            plugins_policy_update_no_plugins(fc_update);
        } else {
            plugins_policy_update(fc_update);
        }
    }
}

void perf_plugins_handler_report(struct perf_plugins_perf_report *data)
{
    const struct mod_scmi_perf_config *config;
    struct perf_plugins_api *api;
    int status;

    config = perf_plugins_ctx.config;

    if (config->plugins_count == 0) {
        return;
    }

    for (size_t i = 0; i < config->plugins_count; i++) {
        api = perf_plugins_ctx.plugins_api_table[i];

        if (api->report != NULL) {
            status = api->report(data);
            if (status != FWK_SUCCESS) {
                FWK_LOG_DEBUG(
                    "[P-Handler] Report: Plugin%u returned error %i",
                    (unsigned int)i,
                    status);
            }
        }
    }
}

static void perf_plugins_alloc_tables(
    struct perf_plugins_perf_update *table,
    size_t count)
{
    table->level = fwk_mm_calloc(count, sizeof(uint32_t));
    table->max_limit = fwk_mm_calloc(count, sizeof(uint32_t));
    table->adj_max_limit = fwk_mm_calloc(count, sizeof(uint32_t));
    table->min_limit = fwk_mm_calloc(count, sizeof(uint32_t));
    table->adj_min_limit = fwk_mm_calloc(count, sizeof(uint32_t));
}

int perf_plugins_handler_init(const struct mod_scmi_perf_config *config)
{
    const struct mod_scmi_perf_domain_config *domain;
    int dvfs_doms_count;
    struct perf_plugins_dev_ctx *dev_ctx;
    unsigned int pgroup, ldom = 0;
    size_t all_doms_count;
    unsigned int phy_group;
    bool has_phy_group;

    dvfs_doms_count =
        fwk_module_get_element_count(FWK_ID_MODULE(FWK_MODULE_IDX_DVFS));
    if (dvfs_doms_count <= 0) {
        return FWK_E_SUPPORT;
    }

    perf_plugins_ctx.dvfs_doms_count = (size_t)dvfs_doms_count;

    perf_plugins_ctx.dev_ctx = fwk_mm_calloc(
        (size_t)dvfs_doms_count, sizeof(struct perf_plugins_dev_ctx));

    /*
     * Iterate all over the SCMI performance domain table to build the number of
     * logical domains that belong to the same physical domain.
     */
    for (size_t i = 0; i < config->perf_doms_count; i++) {
        domain = &(*config->domains)[i];

        has_phy_group = fwk_optional_id_is_defined(domain->phy_group_id);
        if (has_phy_group) {
            phy_group = fwk_id_get_element_idx(domain->phy_group_id);
        } else {
            phy_group = i;
        }

        dev_ctx = &perf_plugins_ctx.dev_ctx[phy_group];
        dev_ctx->log_dom_count++;
    }

    /*
     * Allocate a table for each of the physical domain to contain the
     * requested levels/limits by each of its logical domains.
     * The size will be: number of logical domains + 1 (for physical domain).
     *
     * At the same time, initialise min and max.
     */
    for (size_t i = 0; i < perf_plugins_ctx.dvfs_doms_count; i++) {
        dev_ctx = &perf_plugins_ctx.dev_ctx[i];

        all_doms_count = (size_t)(dev_ctx->log_dom_count + 1);

        perf_plugins_alloc_tables(&dev_ctx->perf_table, all_doms_count);

        dev_ctx->lmin = 0;
        dev_ctx->lmax = UINT32_MAX;
        dev_ctx->perf_table.domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, i);
    }

    /*
     * Allocate a table to store the physical/aggregated values for each of the
     * physical domain for plugins that require the full picture of performance.
     */
    perf_plugins_alloc_tables(
        &perf_plugins_ctx.full_perf_table, perf_plugins_ctx.dvfs_doms_count);

    /*
     * The data provided for a full snapshot will always have a module
     * identifier.
     */
    perf_plugins_ctx.full_perf_table.domain_id =
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF);

    /* Allocate a table of dependency domains identifiers */
    perf_plugins_ctx.dep_id_table =
        fwk_mm_calloc(config->perf_doms_count, sizeof(fwk_id_t));

    domain = &(*config->domains)[0];

    /*
     * Build the dependency domains.
     *
     * If we find the optional identifier for physical domain description,
     * we assume that SCMI performance domains are grouped and ordered together
     * for the same physical domain.
     */
    for (size_t j = 0; j < config->perf_doms_count; j++) {
        has_phy_group = fwk_optional_id_is_defined(domain->phy_group_id);
        if (has_phy_group) {
            /* Custom mapping */
            pgroup = fwk_id_get_element_idx(domain->phy_group_id);
            domain = &(*config->domains)[j];

            if (fwk_id_get_element_idx(domain->phy_group_id) != pgroup) {
                /* Restart count of logical domains */
                ldom = 0;
            }

            perf_plugins_ctx.dep_id_table[j] = FWK_ID_SUB_ELEMENT(
                FWK_MODULE_IDX_DVFS,
                fwk_id_get_element_idx(domain->phy_group_id),
                ldom);

            ldom++;
        } else {
            /* Direct implicit mapping to dvfs */
            perf_plugins_ctx.dep_id_table[j] =
                FWK_ID_SUB_ELEMENT(FWK_MODULE_IDX_DVFS, j, 0);
        }
    }

    perf_plugins_ctx.config = config;

    return FWK_SUCCESS;
}

int perf_plugins_handler_bind(void)
{
    fwk_id_t plugin_id;
    const struct mod_scmi_perf_config *config = perf_plugins_ctx.config;
    int status;

    if (config->plugins_count == 0) {
        return FWK_SUCCESS;
    }

    perf_plugins_ctx.plugins_api_table =
        fwk_mm_calloc(config->plugins_count, sizeof(struct perf_plugins_api *));

    /* Bind to all the performance plugins */
    for (size_t i = 0; i < config->plugins_count; i++) {
        plugin_id = config->plugins[i].id;

        status = fwk_module_bind(
            plugin_id,
            FWK_ID_API(
                fwk_id_get_module_idx(plugin_id), MOD_SCMI_PERF_PLUGIN_API),
            &perf_plugins_ctx.plugins_api_table[i]);
        if (status != FWK_SUCCESS) {
            return FWK_E_PARAM;
        }
    }

    return FWK_SUCCESS;
}

int perf_plugins_handler_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &handler_api;

    return FWK_SUCCESS;
}
#endif /* BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER */
