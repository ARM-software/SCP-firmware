/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Performance FastChannels support.
 *      This module works with the support of SCMI Performance.
 */
#include <internal/scmi_perf.h>

#include <mod_dvfs.h>
#include <mod_scmi.h>
#include <mod_scmi_perf.h>
#include <mod_timer.h>
#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
#    include "perf_plugins_handler.h"
#endif

#ifdef BUILD_HAS_MOD_TRANSPORT
#    include <mod_transport.h>
#endif

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_string.h>

struct mod_scmi_perf_fc_ctx {
    struct mod_scmi_perf_ctx *perf_ctx;

    struct mod_scmi_perf_private_api_perf_stub *api_fch_stub;
    const struct mod_timer_alarm_api *fc_alarm_api;

    uint32_t fast_channels_rate_limit;

    volatile uint32_t pending_req_count;

#ifdef BUILD_HAS_MOD_TRANSPORT_FC

    /*
     * For a timer based fast channel interrupt type we must register
     * fast channel callback only once with the fast channel driver
     * This will hold the status if callback is registered with the
     * fast channel driver.
     */
    bool callback_registered;
#endif
};

static unsigned int fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_COUNT] = {
    [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] = sizeof(uint32_t),
    [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
        sizeof(struct mod_scmi_perf_fast_channel_limit),
    [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] = sizeof(uint32_t),
    [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
        sizeof(struct mod_scmi_perf_fast_channel_limit)
};

static struct mod_scmi_perf_fc_ctx perf_fch_ctx;

static void fast_channel_callback(uintptr_t param);

/*
 * Static Helpers
 */

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
static inline struct scmi_perf_domain_ctx *perf_fch_get_ctx(fwk_id_t domain_id)
{
    return &perf_fch_ctx.perf_ctx
                ->domain_ctx_table[fwk_id_get_element_idx(domain_id)];
}
#endif

#ifdef BUILD_HAS_MOD_TRANSPORT_FC

static inline uint32_t *get_fc_set_level_addr(uint32_t domain_idx)
{
    const struct scmi_perf_domain_ctx *domain_ctx;
    struct mod_scmi_perf_ctx *perf_ctx = perf_fch_ctx.perf_ctx;
    domain_ctx = &perf_ctx->domain_ctx_table[domain_idx];
    const struct fast_channel_ctx *fch_ctx =
        &domain_ctx->fch_ctx[MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET];
    return (uint32_t *)((uintptr_t)fch_ctx->fch_address.local_view_address);
}

static inline struct mod_scmi_perf_fast_channel_limit *get_fc_set_limit_addr(
    uint32_t domain_idx)
{
    const struct scmi_perf_domain_ctx *domain_ctx;
    struct mod_scmi_perf_ctx *perf_ctx = perf_fch_ctx.perf_ctx;
    domain_ctx = &perf_ctx->domain_ctx_table[domain_idx];

    const struct fast_channel_ctx *fch_ctx =
        &domain_ctx->fch_ctx[MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET];

    return (struct mod_scmi_perf_fast_channel_limit
                *)((uintptr_t)fch_ctx->fch_address.local_view_address);
}

void perf_fch_set_fch_get_level(uint32_t domain_idx, uint32_t level)
{
    const struct scmi_perf_domain_ctx *domain_ctx;
    const struct fast_channel_ctx *fch_ctx;
    uint32_t *get_level;
    struct mod_scmi_perf_ctx *perf_ctx = perf_fch_ctx.perf_ctx;
    domain_ctx = &perf_ctx->domain_ctx_table[domain_idx];
    if (perf_fch_domain_has_fastchannels(domain_idx)) {
        fch_ctx = &domain_ctx->fch_ctx[MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET];
        get_level =
            (uint32_t *)((uintptr_t)fch_ctx->fch_address.local_view_address);
        if (get_level != NULL) { /* note: get_level may not be defined */
            *get_level = level;
        }
    }
}

static const struct scmi_perf_fch_config *get_fch_config(
    unsigned int domain_idx,
    unsigned int fch_idx)
{
    return &(*perf_fch_ctx.perf_ctx->config->domains)[domain_idx]
                .fch_config[fch_idx];
}

static struct fast_channel_ctx *get_fch_ctx(
    unsigned int domain_idx,
    unsigned int fch_idx)
{
    return &perf_fch_ctx.perf_ctx->domain_ctx_table[domain_idx]
                .fch_ctx[fch_idx];
}

static int fch_context_init(
    const struct scmi_perf_fch_config *fch_config,
    struct fast_channel_ctx *fch_ctx)
{
    int status = FWK_E_DATA;
    enum mod_transport_fch_interrupt_type interrupt_type;

    status = fch_ctx->transport_fch_api->transport_get_fch_address(
        fch_config->transport_id, &fch_ctx->fch_address);

    if (status != FWK_SUCCESS) {
        return FWK_E_DATA;
    }

    status = fch_ctx->transport_fch_api->transport_get_fch_interrupt_type(
        fch_config->transport_id, &interrupt_type);

    if (status != FWK_SUCCESS) {
        return FWK_E_DATA;
    }

    if (interrupt_type == MOD_TRANSPORT_FCH_INTERRUPT_TYPE_TIMER &&
        !perf_fch_ctx.callback_registered) {
        /*
         * For polled fast channels, we need to register one single
         * call back for all channels so register this only once.
         */
        status = fch_ctx->transport_fch_api->transport_fch_register_callback(
            fch_config->transport_id, (uintptr_t)NULL, fast_channel_callback);

        if (status != FWK_SUCCESS) {
            return FWK_E_DATA;
        }

        perf_fch_ctx.callback_registered = true;
    } else if (interrupt_type == MOD_TRANSPORT_FCH_INTERRUPT_TYPE_HW) {
        status = fch_ctx->transport_fch_api->transport_fch_register_callback(
            fch_config->transport_id, (uintptr_t)NULL, fast_channel_callback);

        if (status != FWK_SUCCESS) {
            return FWK_E_DATA;
        }
    }

    return FWK_SUCCESS;
}

#else

static inline uint32_t *get_fc_set_level_addr(uint32_t domain_idx)
{
    struct mod_scmi_perf_ctx *perf_ctx = perf_fch_ctx.perf_ctx;
    const struct mod_scmi_perf_domain_config *domain;
    domain = &(*perf_ctx->config->domains)[domain_idx];
    return (uint32_t *)((uintptr_t)domain->fast_channels_addr_scp
                            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET]);
}

static inline struct mod_scmi_perf_fast_channel_limit *get_fc_set_limit_addr(
    uint32_t domain_idx)
{
    struct mod_scmi_perf_ctx *perf_ctx = perf_fch_ctx.perf_ctx;
    const struct mod_scmi_perf_domain_config *domain;
    domain = &(*perf_ctx->config->domains)[domain_idx];
    return (struct mod_scmi_perf_fast_channel_limit
                *)((uintptr_t)domain->fast_channels_addr_scp
                       [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET]);
}

void perf_fch_set_fch_get_level(uint32_t domain_idx, uint32_t level)
{
    struct mod_scmi_perf_ctx *perf_ctx = perf_fch_ctx.perf_ctx;
    const struct mod_scmi_perf_domain_config *domain;
    uint32_t *get_level;

    domain = &(*perf_ctx->config->domains)[domain_idx];

    if (domain->fast_channels_addr_scp != NULL) {
        get_level = (uint32_t *)((uintptr_t)domain->fast_channels_addr_scp
                                     [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET]);
        if (get_level != NULL) { /* note: get_level may not be defined */
            *get_level = level;
        }
    }
}
#endif

static inline void decrement_pending_req_count(void)
{
    if (perf_fch_ctx.pending_req_count > 0) {
        perf_fch_ctx.pending_req_count--;
    }
}

static inline void log_and_increment_pending_req_count(void)
{
    if (perf_fch_ctx.pending_req_count > 0) {
        FWK_LOG_INFO("[SCMI-PERF] Multiple FC events pending");
    }

    perf_fch_ctx.pending_req_count++;
}

/*
 * SCMI Performance helpers
 */
bool perf_fch_domain_has_fastchannels(uint32_t domain_idx)
{
#ifdef BUILD_HAS_MOD_TRANSPORT_FC
    const struct mod_scmi_perf_domain_config *domain =
        &(*perf_fch_ctx.perf_ctx->config->domains)[domain_idx];

    return domain->supports_fast_channels;
#else
    const struct mod_scmi_perf_domain_config *domain =
        &(*perf_fch_ctx.perf_ctx->config->domains)[domain_idx];

    return (domain->fast_channels_addr_scp != NULL);
#endif
}

bool perf_fch_prot_msg_attributes_has_fastchannels(
    const struct scmi_protocol_message_attributes_a2p *parameters)
{
    return (
        (parameters->message_id <= MOD_SCMI_PERF_LEVEL_GET) &&
        (parameters->message_id >= MOD_SCMI_PERF_LIMITS_SET));
}

static inline int respond_to_scmi(
    fwk_id_t service_id,
    struct scmi_perf_describe_fc_p2a *return_values)
{
    return perf_fch_ctx.perf_ctx->scmi_api->respond(
        service_id,
        return_values,
        (return_values->status == SCMI_SUCCESS) ?
            sizeof(*return_values) :
            sizeof(return_values->status));
}

#ifdef BUILD_HAS_MOD_TRANSPORT_FC
int perf_fch_describe_fast_channels(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    uint32_t chan_index = 0;
    struct mod_scmi_perf_ctx *perf_ctx = perf_fch_ctx.perf_ctx;
    const struct scmi_perf_domain_ctx *domain_ctx;
    const struct fast_channel_ctx *fch_ctx;
    const struct scmi_perf_describe_fc_a2p *parameters;
    struct scmi_perf_describe_fc_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
    };

    enum scmi_perf_command_id message_id;

    parameters = (const struct scmi_perf_describe_fc_a2p *)payload;

    if (parameters->domain_id >= perf_fch_ctx.perf_ctx->domain_count) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        return respond_to_scmi(service_id, &return_values);
    }

    domain_ctx = &perf_ctx->domain_ctx_table[parameters->domain_id];

    if (!perf_fch_domain_has_fastchannels(parameters->domain_id)) {
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;

        return respond_to_scmi(service_id, &return_values);
    }

    if (parameters->message_id >= MOD_SCMI_PERF_COMMAND_COUNT) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        return respond_to_scmi(service_id, &return_values);
    }

    message_id = (enum scmi_perf_command_id)parameters->message_id;

    switch (message_id) {
    case MOD_SCMI_PERF_LEVEL_GET:
        chan_index = (uint32_t)MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET;
        break;

    case MOD_SCMI_PERF_LEVEL_SET:
        chan_index = (uint32_t)MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET;
        break;

    case MOD_SCMI_PERF_LIMITS_SET:
        chan_index = (uint32_t)MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET;
        break;

    case MOD_SCMI_PERF_LIMITS_GET:
        chan_index = (uint32_t)MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET;
        break;

    default:
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;
        break;
    }

    /* Check for failed cases above */
    if (return_values.status != SCMI_SUCCESS) {
        return respond_to_scmi(service_id, &return_values);
    }

    fch_ctx = &domain_ctx->fch_ctx[chan_index];

    if (fch_ctx->fch_address.target_view_address == 0x0) {
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;

        return respond_to_scmi(service_id, &return_values);
    }

    return_values.attributes = 0; /* Doorbell not supported */
    return_values.rate_limit = perf_fch_ctx.fast_channels_rate_limit;
    return_values.chan_addr_low =
        (uint32_t)(fch_ctx->fch_address.target_view_address & ~0UL);
    return_values.chan_addr_high =
        (uint32_t)((uint64_t)fch_ctx->fch_address.target_view_address >> 32);
    return_values.chan_size = fch_ctx->fch_address.length;

    return respond_to_scmi(service_id, &return_values);
}
#else
int perf_fch_describe_fast_channels(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct mod_scmi_perf_domain_config *domain;
    const struct scmi_perf_describe_fc_a2p *parameters;
    struct scmi_perf_describe_fc_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
    };
    uint32_t chan_size = 0, chan_index = 0;
    enum scmi_perf_command_id message_id;

    parameters = (const struct scmi_perf_describe_fc_a2p *)payload;

    if (parameters->domain_id >= perf_fch_ctx.perf_ctx->domain_count) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        return respond_to_scmi(service_id, &return_values);
    }

    domain = &(*perf_fch_ctx.perf_ctx->config->domains)[parameters->domain_id];

    if (domain->fast_channels_addr_scp == NULL) {
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;

        return respond_to_scmi(service_id, &return_values);
    }

    if (parameters->message_id >= MOD_SCMI_PERF_COMMAND_COUNT) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;

        return respond_to_scmi(service_id, &return_values);
    }

    message_id = (enum scmi_perf_command_id)parameters->message_id;

    switch (message_id) {
    case MOD_SCMI_PERF_LEVEL_GET:
        chan_index = (uint32_t)MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET;
        chan_size =
            fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET];
        break;

    case MOD_SCMI_PERF_LEVEL_SET:
        chan_index = (uint32_t)MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET;
        chan_size =
            fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET];
        break;

    case MOD_SCMI_PERF_LIMITS_SET:
        chan_index = (uint32_t)MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET;
        chan_size =
            fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET];
        break;

    case MOD_SCMI_PERF_LIMITS_GET:
        chan_index = (uint32_t)MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET;
        chan_size =
            fast_channel_elem_size[MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET];
        break;

    default:
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;
        break;
    }

    /* Check for failed cases above */
    if (return_values.status != SCMI_SUCCESS) {
        return respond_to_scmi(service_id, &return_values);
    }

    if (domain->fast_channels_addr_ap == NULL ||
        domain->fast_channels_addr_ap[chan_index] == 0x0) {
        return_values.status = (int32_t)SCMI_NOT_SUPPORTED;

        return respond_to_scmi(service_id, &return_values);
    }

    return_values.attributes = 0; /* Doorbell not supported */
    return_values.rate_limit = perf_fch_ctx.fast_channels_rate_limit;
    return_values.chan_addr_low =
        (uint32_t)(domain->fast_channels_addr_ap[chan_index] & ~0UL);
    return_values.chan_addr_high =
        (uint32_t)(domain->fast_channels_addr_ap[chan_index] >> 32);
    return_values.chan_size = chan_size;

    return respond_to_scmi(service_id, &return_values);
}
#endif

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
static void adjust_level_for_limits(
    const struct mod_scmi_perf_level_limits *limits,
    uint32_t *level)
{
    if (*level < limits->minimum) {
        *level = limits->minimum;
    } else if (*level > limits->maximum) {
        *level = limits->maximum;
    }
}

/*
 * Evaluate the level & limits in one go.
 * Because the limits may also come from the external plugins, their value may
 * not always be exact to the OPPs, so allow approximation.
 */
static void perf_eval_performance(
    fwk_id_t domain_id,
    const struct mod_scmi_perf_level_limits *limits,
    uint32_t *level)
{
    struct scmi_perf_domain_ctx *domain_ctx;
    int status;

    if (limits->minimum > limits->maximum) {
        return;
    }

    domain_ctx = perf_fch_get_ctx(domain_id);

    adjust_level_for_limits(limits, level);
    if ((limits->minimum == domain_ctx->level_limits.minimum) &&
        (limits->maximum == domain_ctx->level_limits.maximum)) {
        status = perf_fch_ctx.api_fch_stub->find_opp_for_level(
            domain_ctx, level, true);
        if (status != FWK_SUCCESS) {
            FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
        }
        return;
    }

    perf_fch_ctx.api_fch_stub->notify_limits_updated(
        domain_id, limits->minimum, limits->maximum);

    domain_ctx->level_limits.minimum = limits->minimum;
    domain_ctx->level_limits.maximum = limits->maximum;

    status =
        perf_fch_ctx.api_fch_stub->find_opp_for_level(domain_ctx, level, true);
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
    }
}
#endif

/*
 * Fast Channel Polling
 */
static void fast_channel_callback(uintptr_t param)
{
    int status;

    struct fwk_event_light event = (struct fwk_event_light){
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_SCMI_PERF,
            SCMI_PERF_EVENT_IDX_FAST_CHANNELS_PROCESS),
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF),
    };

    status = fwk_put_event(&event);
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI-PERF] Error creating FC process event.");
        return;
    }

    log_and_increment_pending_req_count();
}

static inline void load_tlimits(
    struct mod_scmi_perf_fast_channel_limit *set_limit,
    uint32_t *tmax,
    uint32_t *tmin,
    struct scmi_perf_domain_ctx *domain_ctx)
{
    if (set_limit != NULL) {
        *tmax = set_limit->range_max;
        *tmin = set_limit->range_min;
    } else {
        *tmax = domain_ctx->level_limits.maximum;
        *tmin = domain_ctx->level_limits.minimum;
    }
}

static inline void load_tlevel(
    uint32_t *set_level,
    uint32_t *tlevel,
    struct scmi_perf_domain_ctx *domain_ctx)
{
    *tlevel = (set_level != NULL) ? *set_level : domain_ctx->curr_level;
}

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
static void perf_fch_process_plugins_handler(void)
{
    struct mod_scmi_perf_fast_channel_limit *set_limit;
    struct scmi_perf_domain_ctx *domain_ctx;
    uint32_t *set_level;
    uint32_t tlevel, tmax, tmin;
    unsigned int i;
    int status;

    struct mod_scmi_perf_ctx *perf_ctx = perf_fch_ctx.perf_ctx;
    struct fc_perf_update update;

    for (i = 0; i < perf_ctx->domain_count; i++) {
        if (perf_fch_domain_has_fastchannels(i)) {
            set_limit = get_fc_set_limit_addr(i);
            set_level = get_fc_set_level_addr(i);

            domain_ctx = &perf_ctx->domain_ctx_table[i];
            load_tlimits(set_limit, &tmax, &tmin, domain_ctx);

            load_tlevel(set_level, &tlevel, domain_ctx);

            update = (struct fc_perf_update){
                .domain_id = get_dependency_id(i),
                .level = tlevel,
                .max_limit = tmax,
                .min_limit = tmin,
            };

            perf_plugins_handler_update(i, &update);
        }
    }

    for (i = 0; i < perf_ctx->domain_count; i++) {
        if (perf_fch_domain_has_fastchannels(i)) {
            set_limit = get_fc_set_limit_addr(i);
            set_level = get_fc_set_level_addr(i);

            domain_ctx = &perf_ctx->domain_ctx_table[i];

            load_tlimits(set_limit, &tmax, &tmin, domain_ctx);

            load_tlevel(set_level, &tlevel, domain_ctx);

            update = (struct fc_perf_update){
                .domain_id = get_dependency_id(i),
                .level = tlevel,
                .max_limit = tmax,
                .min_limit = tmin,
            };

            perf_plugins_handler_get(i, &update);

            tlevel = update.level;
            tmax = update.adj_max_limit;
            tmin = update.adj_min_limit;

            perf_eval_performance(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_PERF, i),
                &((struct mod_scmi_perf_level_limits){
                    .minimum = tmin,
                    .maximum = tmax,
                }),
                &tlevel);

            status = perf_fch_ctx.perf_ctx->dvfs_api->set_level(
                get_dependency_id(i), 0, tlevel);
            if (status != FWK_SUCCESS) {
                FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
            }
        }
    }

    decrement_pending_req_count();
}
#endif

#ifndef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
static void perf_fch_process(void)
{
    struct mod_scmi_perf_fast_channel_limit *set_limit;
    struct scmi_perf_domain_ctx *domain_ctx;
    uint32_t *set_level;
    uint32_t tlevel, tmax, tmin;
    unsigned int i;
    int status;

    struct mod_scmi_perf_ctx *perf_ctx = perf_fch_ctx.perf_ctx;

    for (i = 0; i < perf_ctx->domain_count; i++) {
        if (perf_fch_domain_has_fastchannels(i)) {
            set_limit = get_fc_set_limit_addr(i);
            set_level = get_fc_set_level_addr(i);

            domain_ctx = &perf_ctx->domain_ctx_table[i];

            load_tlimits(set_limit, &tmax, &tmin, domain_ctx);

            load_tlevel(set_level, &tlevel, domain_ctx);

            if (set_level != NULL && tlevel > 0) {
                status = perf_fch_ctx.api_fch_stub->perf_set_level(
                    get_dependency_id(i), 0, tlevel);
                if (status != FWK_SUCCESS) {
                    FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
                }
            }
            if (set_limit != NULL) {
                if ((tmax == 0) && (tmin == 0)) {
                    continue;
                }
                status = perf_fch_ctx.api_fch_stub->perf_set_limits(
                    get_dependency_id(i),
                    0,
                    &((struct mod_scmi_perf_level_limits){
                        .minimum = tmin,
                        .maximum = tmax,
                    }));
                if (status != FWK_SUCCESS) {
                    FWK_LOG_DEBUG("[SCMI-PERF] %s @%d", __func__, __LINE__);
                }
            }
        }
    }

    decrement_pending_req_count();
}
#endif

/*
 * Framework Handlers, via SCMI-Perf
 */
int perf_fch_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data,
    struct mod_scmi_perf_ctx *mod_ctx,
    struct mod_scmi_perf_private_api_perf_stub *api)
{
    perf_fch_ctx.perf_ctx = mod_ctx;
    perf_fch_ctx.api_fch_stub = api;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_MOD_TRANSPORT_FC
int perf_fch_bind(fwk_id_t id, unsigned int round)
{
    unsigned int domain_idx;
    const struct scmi_perf_fch_config *fch_config;
    static struct fast_channel_ctx *fch_ctx;
    int status;
    /* Bind fast channels */
    for (domain_idx = 0; domain_idx < perf_fch_ctx.perf_ctx->domain_count;
         domain_idx++) {
        unsigned int fch_idx;
        for (fch_idx = 0; fch_idx < MOD_SCMI_PERF_FAST_CHANNEL_COUNT;
             fch_idx++) {
            fch_config = get_fch_config(domain_idx, fch_idx);
            fch_ctx = get_fch_ctx(domain_idx, fch_idx);
            status = fwk_module_bind(
                fch_config->transport_id,
                fch_config->transport_api_id,
                &fch_ctx->transport_fch_api);
            if (status != FWK_SUCCESS) {
                return FWK_E_PANIC;
            }
        }
    }

    return FWK_SUCCESS;
}
#else
int perf_fch_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (!fwk_id_is_equal(
            perf_fch_ctx.perf_ctx->config->fast_channels_alarm_id,
            FWK_ID_NONE)) {
        status = fwk_module_bind(
            perf_fch_ctx.perf_ctx->config->fast_channels_alarm_id,
            MOD_TIMER_API_ID_ALARM,
            &perf_fch_ctx.fc_alarm_api);
        if (status != FWK_SUCCESS) {
            return FWK_E_PANIC;
        }
    }

    return FWK_SUCCESS;
}
#endif

static void *get_fch_local_address(
    unsigned int domain_idx,
    unsigned int fch_idx)
{
#ifdef BUILD_HAS_MOD_TRANSPORT_FC
    const struct scmi_perf_fch_config *fch_config;
    struct fast_channel_ctx *fch_ctx;
    int status;

    fch_config = get_fch_config(domain_idx, fch_idx);
    fch_ctx = get_fch_ctx(domain_idx, fch_idx);

    status = fch_context_init(fch_config, fch_ctx);

    if (status != FWK_SUCCESS) {
        return NULL;
    }

    return (void *)fch_ctx->fch_address.local_view_address;
#else
    const struct mod_scmi_perf_domain_config *domain;
    domain = &(*perf_fch_ctx.perf_ctx->config->domains)[domain_idx];
    return (void *)(uintptr_t)domain->fast_channels_addr_scp[fch_idx];
#endif
}

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
static int initialize_fch_channels_plugins_handler(void)
{
    struct mod_scmi_perf_fast_channel_limit *fc_limits;

    struct scmi_perf_domain_ctx *domain_ctx;
    struct mod_dvfs_opp opp;
    unsigned int j, i;
    void *fc_elem;
    int status;

    struct perf_opp_table *opp_table = NULL;

    /*
     * Initialise FastChannels level to sustained level and limits to min/max
     * OPPs.
     */
    for (i = 0; i < perf_fch_ctx.perf_ctx->domain_count; i++) {
        if (perf_fch_domain_has_fastchannels(i)) {
            for (j = 0; j < MOD_SCMI_PERF_FAST_CHANNEL_COUNT; j++) {
                fc_elem = get_fch_local_address(i, j);
                if (fc_elem != NULL) {
                    if ((j == MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET) ||
                        (j == MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET)) {
                        status =
                            perf_fch_ctx.perf_ctx->dvfs_api->get_sustained_opp(
                                get_dependency_id(i), &opp);
                        if (status != FWK_SUCCESS) {
                            return status;
                        }

                        fwk_str_memcpy(
                            fc_elem, &opp.level, fast_channel_elem_size[j]);
                    } else {
                        /* _LIMIT_SET or _LIMIT_GET */
                        domain_ctx =
                            &perf_fch_ctx.perf_ctx->domain_ctx_table[i];
                        opp_table = domain_ctx->opp_table;

                        fc_limits =
                            (struct mod_scmi_perf_fast_channel_limit *)fc_elem;
                        fc_limits->range_min = opp_table->opps[0].level;
                        fc_limits->range_max =
                            opp_table->opps[opp_table->opp_count - 1].level;
                    }
                }
            }
        }
    }

    return FWK_SUCCESS;
}
#endif

#ifndef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
static void initialize_fch_channels(void)
{
    unsigned int j, i;
    void *fc_elem;

    /* Initialise FastChannels to 0 */
    for (i = 0; i < perf_fch_ctx.perf_ctx->domain_count; i++) {
        if (perf_fch_domain_has_fastchannels(i)) {
            for (j = 0; j < MOD_SCMI_PERF_FAST_CHANNEL_COUNT; j++) {
                fc_elem = get_fch_local_address(i, j);
                if (fc_elem != NULL) {
                    fwk_str_memset(fc_elem, 0, fast_channel_elem_size[j]);
                }
            }
        }
    }
}
#endif

int perf_fch_start(fwk_id_t id)
{
#ifndef BUILD_HAS_MOD_TRANSPORT_FC
    uint32_t fc_interval_msecs;
    int status;

    /*
     * Set up the Fast Channel polling if required
     */
    if (!fwk_id_is_equal(
            perf_fch_ctx.perf_ctx->config->fast_channels_alarm_id,
            FWK_ID_NONE)) {
        if (perf_fch_ctx.fast_channels_rate_limit <
            SCMI_PERF_FC_MIN_RATE_LIMIT) {
            fc_interval_msecs = (uint32_t)SCMI_PERF_FC_MIN_RATE_LIMIT / 1000;
        } else {
            fc_interval_msecs =
                (uint32_t)perf_fch_ctx.fast_channels_rate_limit / 1000;
        }
        status = perf_fch_ctx.fc_alarm_api->start(
            perf_fch_ctx.perf_ctx->config->fast_channels_alarm_id,
            fc_interval_msecs,
            MOD_TIMER_ALARM_TYPE_PERIODIC,
            fast_channel_callback,
            (uintptr_t)0);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }
#endif

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
    return initialize_fch_channels_plugins_handler();
#else
    initialize_fch_channels();

    return FWK_SUCCESS;
#endif
}

int perf_fch_process_event(const struct fwk_event *event)
{
    int status;
    enum scmi_perf_event_idx event_idx =
        (enum scmi_perf_event_idx)fwk_id_get_event_idx(event->id);

    switch (event_idx) {
    case SCMI_PERF_EVENT_IDX_FAST_CHANNELS_PROCESS:

#ifdef BUILD_HAS_SCMI_PERF_PLUGIN_HANDLER
        perf_fch_process_plugins_handler();
#else
        perf_fch_process();
#endif

        status = FWK_SUCCESS;
        break;

    default:
        status = FWK_E_PARAM;
        break;
    }

    return status;
}
