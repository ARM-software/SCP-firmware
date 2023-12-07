/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI performance domain management protocol support.
 */

#include <internal/scmi_perf.h>

#include <mod_dvfs.h>
#include <mod_scmi.h>
#include <mod_scmi_perf.h>
#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
#    include "perf_plugins_handler.h"
#endif
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <stdbool.h>
#include <stddef.h>

static struct mod_scmi_perf_ctx scmi_perf_ctx;

/*
 * SCMI PERF Helpers
 */

/* This identifier is either:
 * - exactly the one built by the perf-plugins-handler (sub-element type)
 * - or the DVFS domain
 */
fwk_id_t get_dependency_id(unsigned int el_idx)
{
#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
    return perf_plugins_get_dependency_id(el_idx);
#else
    return FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, el_idx);
#endif
}

static inline struct scmi_perf_domain_ctx *get_ctx(fwk_id_t domain_id)
{
    return &scmi_perf_ctx.domain_ctx_table[fwk_id_get_element_idx(domain_id)];
}

static inline int opp_for_level_found(
    uint32_t *level,
    struct perf_opp_table *opp_table,
    size_t i)
{
    *level = opp_table->opps[i].level;

    return FWK_SUCCESS;
}

static int find_opp_for_level(
    struct scmi_perf_domain_ctx *domain_ctx,
    uint32_t *level,
    bool use_nearest)
{
    struct perf_opp_table *opp_table;
    size_t i;
    uint32_t opp_level, limit_max;

    opp_table = domain_ctx->opp_table;
    limit_max = domain_ctx->level_limits.maximum;

    for (i = 0; i < opp_table->opp_count; i++) {
        opp_level = opp_table->opps[i].level;

        if ((use_nearest &&
             ((opp_level < *level) && (opp_level < limit_max))) ||
            (!use_nearest && (opp_level != *level))) {
            /*
             * The current OPP level is either below the desired level
             * or not exact match found.
             */
            continue;
        } else {
            /*
             * Either found exact match, or the current OPP is above the limit.
             * Must be within limits.
             */
            if ((opp_level > limit_max) && (i > 0)) {
                i--;
            }

            return opp_for_level_found(level, opp_table, i);
        }
    }

    /* Either not exact match or approximate to the highest level */
    if (use_nearest) {
        i--;

        return opp_for_level_found(level, opp_table, i);
    }

    return FWK_E_RANGE;
}

#if defined(BUILD_HAS_SCMI_PERF_PROTOCOL_OPS) || \
    defined(BUILD_HAS_SCMI_PERF_FAST_CHANNELS)
static int perf_set_level(
    fwk_id_t domain_id,
    unsigned int agent_id,
    uint32_t perf_level)
{
    struct scmi_perf_domain_ctx *domain_ctx;
    int status;

    domain_ctx = get_ctx(domain_id);

    status = find_opp_for_level(
        domain_ctx, &perf_level, scmi_perf_ctx.config->approximate_level);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if ((perf_level < domain_ctx->level_limits.minimum) ||
        (perf_level > domain_ctx->level_limits.maximum)) {
        return FWK_E_RANGE;
    }

    return scmi_perf_ctx.dvfs_api->set_level(domain_id, agent_id, perf_level);
}
#endif

static int validate_new_limits(
    struct scmi_perf_domain_ctx *domain_ctx,
    const struct mod_scmi_perf_level_limits *limits)
{
    uint32_t limit;
    int status;

    if (scmi_perf_ctx.config->approximate_level) {
        /* When approx level is chosen, a level is always found */
        return FWK_SUCCESS;
    }

    limit = limits->minimum;
    status = find_opp_for_level(domain_ctx, &limit, false);
    if (status != FWK_SUCCESS) {
        return status;
    }

    limit = limits->maximum;
    return find_opp_for_level(domain_ctx, &limit, false);
}

#ifdef BUILD_HAS_MOD_TRANSPORT_FC
static void scmi_perf_notify_limits_fch_updated(
    fwk_id_t domain_id,
    uint32_t range_min,
    uint32_t range_max)
{
    unsigned int idx;
    idx = fwk_id_get_element_idx(domain_id);
    struct mod_scmi_perf_fast_channel_limit *get_limit;
    struct scmi_perf_domain_ctx *domain_ctx;
    const struct fast_channel_ctx *fch_ctx;
    domain_ctx = &scmi_perf_ctx.domain_ctx_table[idx];
    if (perf_fch_domain_has_fastchannels(idx)) {
        fch_ctx = &domain_ctx->fch_ctx[MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET];
        get_limit = (struct mod_scmi_perf_fast_channel_limit
                         *)((uintptr_t)fch_ctx->fch_address.local_view_address);
        if (get_limit != NULL) { /* note: get_limit may not be defined */
            get_limit->range_max = range_max;
            get_limit->range_min = range_min;
        }
    }
}
#else
static void scmi_perf_notify_limits_fch_updated(
    fwk_id_t domain_id,
    uint32_t range_min,
    uint32_t range_max)
{
    unsigned int idx;
    const struct mod_scmi_perf_domain_config *domain;
    struct mod_scmi_perf_fast_channel_limit *get_limit;

    idx = fwk_id_get_element_idx(domain_id);

    domain = &(*scmi_perf_ctx.config->domains)[idx];
    if (domain->fast_channels_addr_scp != NULL) {
        get_limit = (struct mod_scmi_perf_fast_channel_limit
                         *)((uintptr_t)domain->fast_channels_addr_scp
                                [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET]);
        if (get_limit != NULL) { /* note: get_limit may not be defined */
            get_limit->range_max = range_max;
            get_limit->range_min = range_min;
        }
    }
}
#endif

/*
 * A domain limits range has been updated. Depending on the system
 * configuration we may send an SCMI notification to the agents which
 * have registered for these notifications and/or update the associated
 * fast channels.
 */
static void scmi_perf_notify_limits_updated(
    fwk_id_t domain_id,
    uint32_t range_min,
    uint32_t range_max)
{
    scmi_perf_notify_limits_fch_updated(domain_id, range_min, range_max);

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
    struct perf_plugins_perf_report perf_report = {
        .dep_dom_id = domain_id,
        .max_limit = range_max,
        .min_limit = range_min,
    };

    perf_plugins_handler_report(&perf_report);
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    unsigned int idx;
    idx = fwk_id_get_element_idx(domain_id);
    perf_prot_ops_notify_limits(idx, range_min, range_max);
#endif
}

int perf_set_limits(
    fwk_id_t domain_id,
    unsigned int agent_id,
    const struct mod_scmi_perf_level_limits *limits)
{
    struct scmi_perf_domain_ctx *domain_ctx;
    uint32_t needle;
    int status;
    bool needs_new_level = true;

    if (limits->minimum > limits->maximum) {
        return FWK_E_PARAM;
    }

    domain_ctx = get_ctx(domain_id);

    if ((limits->minimum == domain_ctx->level_limits.minimum) &&
        (limits->maximum == domain_ctx->level_limits.maximum)) {
        return FWK_SUCCESS;
    }

    status = validate_new_limits(domain_ctx, limits);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Adjust opp for new limits */
    if (domain_ctx->curr_level < limits->minimum) {
        needle = limits->minimum;
    } else if (domain_ctx->curr_level > limits->maximum) {
        needle = limits->maximum;
    } else {
        /* No level transition necessary */
        needs_new_level = false;
    }

    scmi_perf_notify_limits_updated(
        domain_id, limits->minimum, limits->maximum);

    domain_ctx->level_limits.minimum = limits->minimum;
    domain_ctx->level_limits.maximum = limits->maximum;

    if (!needs_new_level) {
        return FWK_SUCCESS;
    }

    status = find_opp_for_level(
        domain_ctx, &needle, scmi_perf_ctx.config->approximate_level);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return scmi_perf_ctx.dvfs_api->set_level(domain_id, agent_id, needle);
}

/*
 * A domain performance level has been updated. Depending on the system
 * configuration we may send an SCMI notification to the agents which
 * have registered for these notifications and/or update the associated
 * fast channels.
 */
static void scmi_perf_notify_level_updated(
    fwk_id_t domain_id,
    uintptr_t cookie,
    uint32_t level)
{
    struct scmi_perf_domain_ctx *domain_ctx;
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    fwk_id_t dep_dom_id;
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    unsigned int idx = fwk_id_get_element_idx(domain_id);
#endif

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
    struct perf_plugins_perf_report perf_report = {
        .dep_dom_id = domain_id,
        .level = level,
    };

    perf_plugins_handler_report(&perf_report);
#endif

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    /*
     * The SCMI spec enforces that "[PERFORMANCE_LEVEL_GET] this command returns
     * the current performance level of a domain", thus when a physical domain
     * has been updated, we update all the relevant logical domains.
     */
    for (uint32_t i = 0; i < scmi_perf_ctx.domain_count; i++) {
        dep_dom_id = get_dependency_id((unsigned int)i);

        if (fwk_id_get_element_idx(dep_dom_id) ==
            fwk_id_get_element_idx(domain_id)) {
            perf_fch_set_fch_get_level(i, level);
        }
    }
#endif

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    perf_prot_ops_notify_level(idx, level, cookie);
#endif

#ifdef BUILD_HAS_MOD_STATISTICS
    perf_prot_ops_update_stats(domain_id, level);
#endif

    domain_ctx = get_ctx(domain_id);
    domain_ctx->curr_level = level;
}

static struct mod_scmi_perf_updated_api perf_update_api = {
    .notify_level_updated = scmi_perf_notify_level_updated,
};

#if defined(BUILD_HAS_SCMI_PERF_PROTOCOL_OPS) || \
    defined(BUILD_HAS_SCMI_PERF_FAST_CHANNELS)
static struct mod_scmi_perf_private_api_perf_stub api_perf_stub = {
    .perf_set_level = perf_set_level,
    .perf_set_limits = perf_set_limits,
    .find_opp_for_level = find_opp_for_level,
    .notify_limits_updated = scmi_perf_notify_limits_updated,
};
#endif

/*
 * Framework handlers
 */
static int scmi_perf_init(fwk_id_t module_id, unsigned int element_count,
                          const void *data)
{
    int dvfs_doms_count;
    const struct mod_scmi_perf_config *config =
        (const struct mod_scmi_perf_config *)data;

#if defined(BUILD_HAS_SCMI_PERF_PROTOCOL_OPS) || \
    defined(BUILD_HAS_SCMI_PERF_FAST_CHANNELS)
    int status;
#endif

    if ((config == NULL) || (config->domains == NULL)) {
        return FWK_E_PARAM;
    }

    dvfs_doms_count =
        fwk_module_get_element_count(FWK_ID_MODULE(FWK_MODULE_IDX_DVFS));
    if (dvfs_doms_count <= 0) {
        return FWK_E_SUPPORT;
    }

    scmi_perf_ctx.dvfs_doms_count = (unsigned int)dvfs_doms_count;

    scmi_perf_ctx.domain_ctx_table = fwk_mm_calloc(
        config->perf_doms_count, sizeof(struct scmi_perf_domain_ctx));

    scmi_perf_ctx.config = config;
    scmi_perf_ctx.domain_count = (uint32_t)config->perf_doms_count;
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    status = perf_fch_init(
        module_id, element_count, data, &scmi_perf_ctx, &api_perf_stub);
    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }
#endif

#ifdef BUILD_HAS_SCMI_PERF_PROTOCOL_OPS
    status = perf_prot_ops_init(
        module_id, element_count, data, &scmi_perf_ctx, &api_perf_stub);
    if (status != FWK_SUCCESS) {
        return FWK_E_PANIC;
    }
#endif

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
    return perf_plugins_handler_init(config);
#else
    return FWK_SUCCESS;
#endif
}

static int scmi_perf_bind(fwk_id_t id, unsigned int round)
{
#if defined(BUILD_HAS_SCMI_PERF_PROTOCOL_OPS) || \
    defined(BUILD_HAS_SCMI_PERF_FAST_CHANNELS)
    int status;
#endif

    if (round == 1) {
        return FWK_SUCCESS;
    }

#if defined(BUILD_HAS_SCMI_PERF_FAST_CHANNELS)
    status = perf_fch_bind(id, 0);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif
#ifdef BUILD_HAS_SCMI_PERF_PROTOCOL_OPS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_perf_ctx.scmi_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = perf_prot_ops_bind(id, 0);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
    status = perf_plugins_handler_bind();
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

    return fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_DVFS),
        FWK_ID_API(FWK_MODULE_IDX_DVFS, MOD_DVFS_API_IDX_DVFS),
        &scmi_perf_ctx.dvfs_api);
}

static int scmi_perf_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    enum scmi_perf_api_idx api_id_type =
        (enum scmi_perf_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
#ifdef BUILD_HAS_SCMI_PERF_PROTOCOL_OPS
    case MOD_SCMI_PERF_PROTOCOL_API:
        perf_prot_ops_process_bind_request(source_id, target_id, api_id, api);
        break;
#endif

    case MOD_SCMI_PERF_DVFS_UPDATE_API:
        if (!fwk_id_is_equal(source_id, fwk_module_id_dvfs)) {
            /* Only DVFS can use this API */
            return FWK_E_ACCESS;
        }

        *api = &perf_update_api;
        break;

    case MOD_SCMI_PERF_PLUGINS_API:
#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
        return perf_plugins_handler_process_bind_request(
            source_id, target_id, api_id, api);
#else
        return FWK_E_ACCESS;
#endif
        break;

    default:
        return FWK_E_ACCESS;
    }

    return FWK_SUCCESS;
}


static int scmi_perf_start(fwk_id_t id)
{
    int status = FWK_SUCCESS;

    struct scmi_perf_domain_ctx *domain_ctx;
    fwk_id_t domain_id;
    size_t opp_count;
    unsigned int i;

    const struct mod_dvfs_domain_config *dvfs_config;
    struct perf_opp_table *opp_table = NULL;
    const struct mod_scmi_perf_domain_config *domain_cfg;
    unsigned int dom_idx;
    bool has_phy_group;

    scmi_perf_ctx.opp_table = fwk_mm_calloc(
        scmi_perf_ctx.dvfs_doms_count, sizeof(struct perf_opp_table));

    for (i = 0; i < scmi_perf_ctx.dvfs_doms_count; i++) {
        opp_table = &scmi_perf_ctx.opp_table[i];

        domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, i);

        /* Get the number of levels for this DVFS domain */
        status = scmi_perf_ctx.dvfs_api->get_opp_count(domain_id, &opp_count);
        if (status != FWK_SUCCESS) {
            return status;
        }

        dvfs_config =
            (const struct mod_dvfs_domain_config *)fwk_module_get_data(
                domain_id);

        opp_table->opps = &dvfs_config->opps[0];
        opp_table->opp_count = opp_count;
        opp_table->dvfs_id = domain_id;
    }

    /* Assign to each performance domain the correct OPP table */
    for (dom_idx = 0; dom_idx < scmi_perf_ctx.domain_count; dom_idx++) {
        domain_cfg = &(*scmi_perf_ctx.config->domains)[dom_idx];

        has_phy_group = fwk_optional_id_is_defined(domain_cfg->phy_group_id);
        if (has_phy_group) {
            domain_id = domain_cfg->phy_group_id;
        } else {
            domain_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_DVFS, dom_idx);
        }

        domain_ctx = &scmi_perf_ctx.domain_ctx_table[dom_idx];

        /*
         * Search the corresponding physical domain for this performance domain
         * and assign the correct opp_table.
         */
        for (i = 0; i < scmi_perf_ctx.dvfs_doms_count; i++) {
            opp_table = &scmi_perf_ctx.opp_table[i];

            if (fwk_id_is_equal(opp_table->dvfs_id, domain_id)) {
                domain_ctx->opp_table = opp_table;

                /* init limits */
                domain_ctx->level_limits.minimum = opp_table->opps[0].level;
                domain_ctx->level_limits.maximum =
                    opp_table->opps[opp_table->opp_count - 1].level;

                break;
            }
        }
        if (domain_ctx->opp_table == NULL) {
            /* The corresponding physical domain did not have a match */
            return FWK_E_PANIC;
        }
    }

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    status = perf_fch_start(id);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

#ifdef BUILD_HAS_SCMI_PERF_PROTOCOL_OPS
    status = perf_prot_ops_start(id);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

    return status;
}

/* Handle internal events */
static int process_internal_event(const struct fwk_event *event)
{
    int status;
    enum scmi_perf_event_idx event_idx =
        (enum scmi_perf_event_idx)fwk_id_get_event_idx(event->id);

    switch (event_idx) {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    case SCMI_PERF_EVENT_IDX_FAST_CHANNELS_PROCESS:
        status = perf_fch_process_event(event);
        break;
#endif
    default:
        status = FWK_E_PARAM;
        break;
    }

    return status;
}

static int scmi_perf_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
#ifdef BUILD_HAS_SCMI_PERF_PROTOCOL_OPS
    if ((fwk_id_get_module_idx(event->source_id) ==
         fwk_id_get_module_idx(fwk_module_id_scmi)) ||
        (fwk_id_get_module_idx(event->source_id) ==
         fwk_id_get_module_idx(fwk_module_id_dvfs))) {
        /* Handle requests from SCMi and responses from DVFS */
        return perf_prot_ops_process_events(event, resp_event);
    }
#endif

    /* Response internal events */
    if (fwk_id_get_module_idx(event->source_id) ==
        fwk_id_get_module_idx(fwk_module_id_scmi_perf)) {
        return process_internal_event(event);
    }

    return FWK_E_PARAM;
}

/* SCMI Performance Management Protocol Definition */
const struct fwk_module module_scmi_perf = {
    .api_count = (unsigned int)MOD_SCMI_PERF_API_COUNT,
    .event_count = (unsigned int)SCMI_PERF_EVENT_IDX_COUNT,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_perf_init,
    .bind = scmi_perf_bind,
    .start = scmi_perf_start,
    .process_bind_request = scmi_perf_process_bind_request,
    .process_event = scmi_perf_process_event,
};
