/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI power capping and monitoring protocol fast channels.
 */

#include "internal/scmi_power_capping.h"
#include "internal/scmi_power_capping_fast_channels.h"

#ifdef BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS
#    include <mod_scmi.h>
#endif
#include <mod_transport.h>

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>

#ifndef BUILD_HAS_SCMI_POWER_CAPPING
#    include <fwk_module.h>
#endif

struct pcapping_fast_channel_ctx {
    const struct scmi_pcapping_fch_config *fch_config;
    /* The fast channel address */
    struct mod_transport_fast_channel_addr fch_address;
    uint32_t fch_attributes;
    uint32_t fch_rate_limit;
    /* Transport Fast Channels API */
    const struct mod_transport_fast_channels_api *transport_fch_api;
};

struct {
    /* Table of power capping domain contexts */
    struct mod_scmi_power_capping_domain_context
        *power_capping_domain_ctx_table;
    const struct mod_scmi_power_capping_power_apis *power_management_apis;
    struct pcapping_fast_channel_ctx *fch_ctx_table;
    uint32_t fch_count;
    bool callback_registered;
    enum mod_transport_fch_interrupt_type interrupt_type;
} pcapping_fast_channel_global_ctx = { .callback_registered = false };

static fwk_id_t pcapping_fast_channel_get_power_allocator_id(
    uint32_t domain_idx)
{
    return pcapping_fast_channel_global_ctx
        .power_capping_domain_ctx_table[domain_idx]
        .config->power_allocator_domain_id;
}

static fwk_id_t pcapping_fast_channel_get_power_coordinator_id(
    uint32_t domain_idx)
{
    return pcapping_fast_channel_global_ctx
        .power_capping_domain_ctx_table[domain_idx]
        .config->power_coordinator_domain_id;
}

static void pcapping_fast_channel_callback(uintptr_t param)
{
    int status;
    uint32_t *event_param;

    struct fwk_event event = (struct fwk_event){
        .id = FWK_ID_EVENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_CAPPING,
            SCMI_POWER_CAPPING_EVENT_IDX_FAST_CHANNELS_PROCESS),
        .source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_POWER_CAPPING),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_POWER_CAPPING),
    };
    event_param = (uint32_t *)event.params;
    *event_param = (uint32_t)param;

    status = fwk_put_event(&event);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            "[SCMI-Power-Capping-Fast-Channel] Error creating fast channel "
            "event.");
    }
}

static void pcapping_fast_channel_get_cap(
    uint32_t domain_idx,
    uint32_t *fch_addr)
{
    int status;

    status = pcapping_fast_channel_global_ctx.power_management_apis
                 ->power_allocator_api->get_cap(
                     pcapping_fast_channel_get_power_allocator_id(domain_idx),
                     fch_addr);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[SCMI-Power-Capping-Fast-Channel] Error getting cap.");
    }
}

static void pcapping_fast_channel_set_cap(
    uint32_t domain_idx,
    uint32_t *fch_addr)
{
    int status;

    status = pcapping_fast_channel_global_ctx.power_management_apis
                 ->power_allocator_api->set_cap(
                     pcapping_fast_channel_get_power_allocator_id(domain_idx),
                     *fch_addr);
    if ((status != FWK_SUCCESS) && (status != FWK_PENDING)) {
        FWK_LOG_ERR("[SCMI-Power-Capping-Fast-Channel] Error setting cap.");
    }
}

static void pcapping_fast_channel_get_pai(
    uint32_t domain_idx,
    uint32_t *fch_addr)
{
    int status;

    status = pcapping_fast_channel_global_ctx.power_management_apis
                 ->power_coordinator_api->get_coordinator_period(
                     pcapping_fast_channel_get_power_coordinator_id(domain_idx),
                     fch_addr);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[SCMI-Power-Capping-Fast-Channel] Error getting PAI.");
    }
}

static void pcapping_fast_channel_set_pai(
    uint32_t domain_idx,
    uint32_t *fch_addr)
{
    int status;

    status = pcapping_fast_channel_global_ctx.power_management_apis
                 ->power_coordinator_api->set_coordinator_period(
                     pcapping_fast_channel_get_power_coordinator_id(domain_idx),
                     *fch_addr);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[SCMI-Power-Capping-Fast-Channel] Error setting PAI.");
    }
}

static void (
    *pcapping_fast_channel_handler[MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT])(
    uint32_t domain_idx,
    uint32_t *fch_addr) = {
    [MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_GET] = pcapping_fast_channel_get_cap,
    [MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_SET] = pcapping_fast_channel_set_cap,
    [MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_GET] = pcapping_fast_channel_get_pai,
    [MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_SET] = pcapping_fast_channel_set_pai,
};

#ifdef BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS
static struct pcapping_fast_channel_ctx *get_channel_ctx(
    uint32_t domain_idx,
    uint32_t message_id)
{
    int status = FWK_SUCCESS;

    uint32_t channel_index;

    enum scmi_power_capping_command_id message_enu;

    message_enu = (enum scmi_power_capping_command_id)message_id;

    switch (message_enu) {
    case MOD_SCMI_POWER_CAPPING_CAP_GET:
        channel_index = (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_GET;
        break;
    case MOD_SCMI_POWER_CAPPING_CAP_SET:
        channel_index = (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_SET;
        break;
    case MOD_SCMI_POWER_CAPPING_PAI_GET:
        channel_index = (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_GET;
        break;
    case MOD_SCMI_POWER_CAPPING_PAI_SET:
        channel_index = (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_SET;
        break;
    default:
        status = FWK_E_RANGE;
        break;
    }

    if (status != FWK_SUCCESS) {
        return NULL;
    }

    channel_index +=
        (domain_idx * (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT);

    if (channel_index >= pcapping_fast_channel_global_ctx.fch_count) {
        return NULL;
    }

    return &(pcapping_fast_channel_global_ctx.fch_ctx_table[channel_index]);
}

int pcapping_fast_channel_get_info(
    uint32_t domain_idx,
    uint32_t message_id,
    struct pcapping_fast_channel_info *info)
{
    struct pcapping_fast_channel_ctx *fch_ctx;

    fch_ctx = get_channel_ctx(domain_idx, message_id);

    if (fch_ctx == NULL) {
        return FWK_E_RANGE;
    }

    if (!fch_ctx->fch_config->fch_support) {
        return FWK_E_SUPPORT;
    }

    info->fch_address = fch_ctx->fch_address.target_view_address;
    info->fch_channel_size = fch_ctx->fch_address.length;
    info->fch_attributes = fch_ctx->fch_attributes;
    info->fch_rate_limit = fch_ctx->fch_rate_limit;

    return FWK_SUCCESS;
}

bool pcapping_fast_channel_get_msg_supp(uint32_t message_id)
{
    uint32_t domain_idx = 0;
    uint32_t domain_count;
    struct pcapping_fast_channel_ctx *fch_ctx;

    bool fast_channel_support = false;

    domain_count = pcapping_fast_channel_global_ctx.fch_count /
        (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;

    fch_ctx = get_channel_ctx(domain_idx++, message_id);

    if (fch_ctx == NULL) {
        return false;
    }

    fast_channel_support = fch_ctx->fch_config->fch_support;

    while ((domain_idx < domain_count) && (!fast_channel_support)) {
        fch_ctx = get_channel_ctx(domain_idx, message_id);
        fast_channel_support = fch_ctx->fch_config->fch_support;
        domain_idx++;
    }

    return fast_channel_support;
}

bool pcapping_fast_channel_get_domain_supp(uint32_t domain_idx)
{
    bool fast_channel_support = false;

    uint32_t channel_index;
    uint32_t channel_index_max;

    channel_index =
        (domain_idx * (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT);

    channel_index_max = channel_index + MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;

    while ((channel_index < channel_index_max) && (!fast_channel_support)) {
        fast_channel_support =
            pcapping_fast_channel_global_ctx.fch_ctx_table[channel_index]
                .fch_config->fch_support;
        channel_index++;
    }

    return fast_channel_support;
}
#endif

static void pcapping_fast_channel_process_command(uint32_t fch_idx)
{
    uint32_t domain_idx;
    enum mod_scmi_power_capping_fast_channels_cmd_handler_index
        cmd_handler_index;
    struct pcapping_fast_channel_ctx *fch_ctx;

    domain_idx = fch_idx / (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
    cmd_handler_index =
        fch_idx - (domain_idx * (uint32_t)MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT);
    fch_ctx = &(pcapping_fast_channel_global_ctx.fch_ctx_table[fch_idx]);
    if (fch_ctx->fch_config->fch_support == true) {
        pcapping_fast_channel_handler[cmd_handler_index](
            domain_idx, (uint32_t *)fch_ctx->fch_address.local_view_address);
    }
}

static void pcapping_fast_channel_setup_interrupt(
    uint32_t fch_idx,
    const struct scmi_pcapping_fch_config *fch_config,
    struct pcapping_fast_channel_ctx *fch_ctx)
{
    if (pcapping_fast_channel_global_ctx.interrupt_type ==
        MOD_TRANSPORT_FCH_INTERRUPT_TYPE_TIMER) {
        /*
         * For polled fast channels, we need to register one single
         * call back for all channels so register this only once.
         */
        if (!pcapping_fast_channel_global_ctx.callback_registered) {
            fch_ctx->transport_fch_api->transport_fch_register_callback(
                fch_config->transport_id,
                (uintptr_t)NULL,
                pcapping_fast_channel_callback);
            pcapping_fast_channel_global_ctx.callback_registered = true;
        }
    } else {
        fch_ctx->transport_fch_api->transport_fch_register_callback(
            fch_config->transport_id,
            (uintptr_t)fch_idx,
            pcapping_fast_channel_callback);
    }
}

int pcapping_fast_channel_process_event(const struct fwk_event *event)
{
    uint32_t fch_idx;

    if (pcapping_fast_channel_global_ctx.interrupt_type ==
        MOD_TRANSPORT_FCH_INTERRUPT_TYPE_HW) {
        fch_idx = *(uint32_t *)event->params;
        pcapping_fast_channel_process_command(fch_idx);
    } else {
        for (fch_idx = 0; fch_idx < pcapping_fast_channel_global_ctx.fch_count;
             fch_idx++) {
            pcapping_fast_channel_process_command(fch_idx);
        }
    }

    return FWK_SUCCESS;
}

void pcapping_fast_channel_ctx_init(struct mod_scmi_power_capping_context *ctx)
{
    pcapping_fast_channel_global_ctx.fch_count =
        ctx->domain_count * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;

    pcapping_fast_channel_global_ctx.power_capping_domain_ctx_table =
        ctx->power_capping_domain_ctx_table;

    pcapping_fast_channel_global_ctx.fch_ctx_table = fwk_mm_calloc(
        pcapping_fast_channel_global_ctx.fch_count,
        sizeof(struct pcapping_fast_channel_ctx));
}

void pcapping_fast_channel_set_domain_config(
    uint32_t domain_idx,
    const struct mod_scmi_power_capping_domain_config *config)
{
    uint32_t fch_index;
    uint32_t cmd_handler_index;

    fch_index = domain_idx * MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
    for (cmd_handler_index = 0;
         cmd_handler_index < MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT;
         cmd_handler_index++) {
        pcapping_fast_channel_global_ctx
            .fch_ctx_table[fch_index + cmd_handler_index]
            .fch_config = &(config->fch_config[cmd_handler_index]);
    }
}

int pcapping_fast_channel_bind(void)
{
    int status;
    uint32_t fch_idx;
    struct pcapping_fast_channel_ctx *fch_ctx;

    for (fch_idx = 0; fch_idx < pcapping_fast_channel_global_ctx.fch_count;
         fch_idx++) {
        fch_ctx = &(pcapping_fast_channel_global_ctx.fch_ctx_table[fch_idx]);
        if (fch_ctx->fch_config->fch_support) {
            status = fwk_module_bind(
                fch_ctx->fch_config->transport_id,
                fch_ctx->fch_config->transport_api_id,
                &(fch_ctx->transport_fch_api));
            if (status != FWK_SUCCESS) {
                return FWK_E_PANIC;
            }
        }
    }

    return FWK_SUCCESS;
}

void pcapping_fast_channel_set_power_apis(
    const struct mod_scmi_power_capping_power_apis *power_management_apis)
{
    pcapping_fast_channel_global_ctx.power_management_apis =
        power_management_apis;
}

void pcapping_fast_channel_start(void)
{
    uint32_t fch_idx;
    const struct scmi_pcapping_fch_config *fch_config;
    struct pcapping_fast_channel_ctx *fch_ctx;

    for (fch_idx = 0; fch_idx < pcapping_fast_channel_global_ctx.fch_count;
         fch_idx++) {
        fch_ctx = &(pcapping_fast_channel_global_ctx.fch_ctx_table[fch_idx]);
        fch_config = fch_ctx->fch_config;
        if (fch_config->fch_support) {
            fch_ctx->transport_fch_api->transport_get_fch_address(
                fch_config->transport_id, &(fch_ctx->fch_address));

            fch_ctx->fch_attributes = 0x0u;

            fch_ctx->transport_fch_api->transport_get_fch_rate_limit(
                fch_config->transport_id, &(fch_ctx->fch_rate_limit));

            fch_ctx->transport_fch_api->transport_get_fch_interrupt_type(
                fch_config->transport_id,
                &(pcapping_fast_channel_global_ctx.interrupt_type));

            pcapping_fast_channel_setup_interrupt(fch_idx, fch_config, fch_ctx);
        }
    }
}
