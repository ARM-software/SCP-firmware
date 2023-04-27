/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <scp_tc_mhu.h>

#include <mod_bootloader.h>
#include <mod_ppu_v1.h>
#include <mod_tc2_bl1.h>
#include <mod_transport.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <string.h>

struct tc2_bl1_ctx {
    const struct mod_tc2_bl1_config *bl1_config;
    struct ppu_v1_boot_api *ppu_boot_api;
    struct mod_bootloader_api *bootloader_api;
    struct mod_transport_firmware_api *firmware_api;
    unsigned int notification_count; /* Notifications awaiting a response */
} ctx;

enum mod_bl1_event {
    MOD_BL1_EVENT_RUN,
    MOD_BL1_EVENT_RSS_HANDSHAKE,
    MOD_BL1_EVENT_COUNT
};

static int tc2_signal_error(fwk_id_t service_id)
{
    fwk_unexpected();
    return FWK_E_PANIC;
}

bool fwk_is_interrupt_context(void);

static int tc2_signal_message(fwk_id_t service_id)
{
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_TC2_BL1),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_TC2_BL1),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_TC2_BL1, MOD_BL1_EVENT_RSS_HANDSHAKE),
    };

    return fwk_put_event(&event);
}

static const struct mod_transport_firmware_signal_api
    tc2_system_transport_firmware_signal_api = {
        .signal_error = tc2_signal_error,
        .signal_message = tc2_signal_message,
    };

static int bl1_deferred_setup(void)
{
    /* Initialize the AP context area by zeroing it */
    memset(
        (void *)ctx.bl1_config->ap_context_base,
        0,
        ctx.bl1_config->ap_context_size);

    FWK_LOG_INFO("[TC2_BL1] Pinging RSS");
    ctx.firmware_api->trigger_interrupt(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TRANSPORT, 0));

    return FWK_SUCCESS;
}

/*
 * Functions fulfilling the framework's module interface
 */

static int tc2_bl1_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    ctx.bl1_config = data;

    if ((ctx.bl1_config->ap_context_base == 0) ||
        (ctx.bl1_config->ap_context_size == 0))
        return FWK_E_RANGE;

    return FWK_SUCCESS;
}

static int tc2_bl1_bind(fwk_id_t id, unsigned int round)
{
    int status;

    /* Use second round only (round numbering is zero-indexed) */
    if (round == 1) {
        /* Bind to the PPU module */
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_PPU_V1),
            FWK_ID_API(FWK_MODULE_IDX_PPU_V1, MOD_PPU_V1_API_IDX_BOOT),
            &ctx.ppu_boot_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;

        /* Bind to the bootloader module */
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_BOOTLOADER),
            FWK_ID_API(FWK_MODULE_IDX_BOOTLOADER, 0),
            &ctx.bootloader_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;

        status = fwk_module_bind(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TRANSPORT, 0),
            FWK_ID_API(
                FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_FIRMWARE),
            &ctx.firmware_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int tc2_bl1_start(fwk_id_t id)
{
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_TC2_BL1),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_TC2_BL1),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_TC2_BL1, MOD_BL1_EVENT_RUN),
    };

    return fwk_put_event(&event);
}

static int tc2_bl1_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    int status;

    if (fwk_id_get_event_idx(event->id) == MOD_BL1_EVENT_RUN) {
        struct mod_pd_power_state_transition_notification_params
            *notification_params;
        struct fwk_event systop_on_event = {
            .response_requested = true,
            .id = mod_tc2_bl1_notification_id_systop,
            .source_id = FWK_ID_NONE
        };

        /* Notify any subscribers of the SYSTOP power domain state change */
        notification_params =
            (struct mod_pd_power_state_transition_notification_params *)
                systop_on_event.params;
        notification_params->state = MOD_PD_STATE_ON;

        status =
            fwk_notification_notify(&systop_on_event, &ctx.notification_count);
        if (status != FWK_SUCCESS)
            return status;

        return FWK_SUCCESS;
    } else if (fwk_id_get_event_idx(event->id) == MOD_BL1_EVENT_RSS_HANDSHAKE) {
        FWK_LOG_INFO("[TC2_BL1] Got ACK from RSS");

        /* Power on the primary cluster and cpu */
        ctx.ppu_boot_api->power_mode_on(ctx.bl1_config->id_primary_cluster);
        ctx.ppu_boot_api->power_mode_on(ctx.bl1_config->id_primary_core);

        status = ctx.bootloader_api->load_image();

#if !(FWK_LOG_LEVEL < FWK_LOG_LEVEL_DISABLED)
        (void)status;
#endif

        FWK_LOG_CRIT(
            "[TC2_BL1] Failed to load RAM firmware image: %s",
            fwk_status_str(status));

        return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

static int tc2_bl1_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    fwk_assert(fwk_id_is_equal(event->id, mod_tc2_bl1_notification_id_systop));
    fwk_assert(event->is_response == true);

    /* At least one notification response must be outstanding */
    if (ctx.notification_count == 0) {
        fwk_unexpected();
        return FWK_E_PANIC;
    }

    /* Complete remaining setup now that all subscribers have responded */
    if ((--ctx.notification_count) == 0) {
        return bl1_deferred_setup();
    }

    return FWK_SUCCESS;
}

static int tc2_bl1_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &tc2_system_transport_firmware_signal_api;
    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_tc2_bl1 = {
    .api_count = MOD_TC2_BL1_API_IDX_COUNT,
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = MOD_BL1_EVENT_COUNT,
    .notification_count = MOD_TC2_BL1_NOTIFICATION_COUNT,
    .init = tc2_bl1_init,
    .bind = tc2_bl1_bind,
    .start = tc2_bl1_start,
    .process_bind_request = tc2_bl1_process_bind_request,
    .process_event = tc2_bl1_process_event,
    .process_notification = tc2_bl1_process_notification,
};
