/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description: Module dedicated to the ROM firmware of the mobile platforms to
 *     initiate the boot of the primary AP core and then to jump to the SCP RAM
 *     firmware.
 */

#include <mod_bootloader.h>
#include <mod_msys_rom.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <string.h>

struct msys_rom_ctx {
    const struct msys_rom_config *rom_config;
    struct ppu_v1_boot_api *ppu_boot_api;
    struct mod_bootloader_api *bootloader_api;
    unsigned int notification_count; /* Notifications awaiting a response */
} ctx;

enum rom_event {
    ROM_EVENT_RUN,
    ROM_EVENT_COUNT
};

static int msys_deferred_setup(void)
{
    int status;

    /* Initialize the AP context area by zeroing it */
    memset((void *)ctx.rom_config->ap_context_base,
           0,
           ctx.rom_config->ap_context_size);

    /* Power on the primary cluster and cpu */
    ctx.ppu_boot_api->power_mode_on(ctx.rom_config->id_primary_cluster);
    ctx.ppu_boot_api->power_mode_on(ctx.rom_config->id_primary_core);

    FWK_LOG_INFO("[SYSTEM] Primary CPU powered");

    status = ctx.bootloader_api->load_image();

    FWK_LOG_ERR("[SYSTEM] Failed to load RAM firmware image: %d", status);

    return FWK_E_DATA;
}

/*
 * Functions fulfilling the framework's module interface
 */

static int msys_rom_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    ctx.rom_config = data;

    if ((ctx.rom_config->ap_context_base == 0) ||
        (ctx.rom_config->ap_context_size == 0))
        return FWK_E_RANGE;

    return FWK_SUCCESS;
}

static int msys_rom_bind(fwk_id_t id, unsigned int round)
{
    int status;

    /* Use second round only (round numbering is zero-indexed) */
    if (round == 1) {

        /* Bind to the PPU module */
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_PPU_V1),
                                 FWK_ID_API(FWK_MODULE_IDX_PPU_V1,
                                            MOD_PPU_V1_API_IDX_BOOT),
                                 &ctx.ppu_boot_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;

        /* Bind to the bootloader module */
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_BOOTLOADER),
                                 FWK_ID_API(FWK_MODULE_IDX_BOOTLOADER, 0),
                                 &ctx.bootloader_api);
        if (status != FWK_SUCCESS)
        return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int msys_rom_start(fwk_id_t id)
{
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_MSYS_ROM),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_MSYS_ROM),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_MSYS_ROM, ROM_EVENT_RUN),
    };

    return fwk_thread_put_event(&event);
}

static int msys_rom_process_event(const struct fwk_event *event,
    struct fwk_event *resp)
{
    int status;
    struct mod_pd_power_state_transition_notification_params
        *notification_params;
    struct fwk_event systop_on_event = {
        .response_requested = true,
        .id = mod_msys_rom_notification_id_systop,
        .source_id = FWK_ID_NONE
    };

    /* Notify any subscribers of the SYSTOP power domain state change */
    notification_params =
        (struct mod_pd_power_state_transition_notification_params *)
            systop_on_event.params;
    notification_params->state = MOD_PD_STATE_ON;

    status = fwk_notification_notify(&systop_on_event, &ctx.notification_count);
    if (status != FWK_SUCCESS)
        return status;

    /* Complete remaining setup now if there are no subscribers to respond */
    if (ctx.notification_count == 0)
        return msys_deferred_setup();

    return FWK_SUCCESS;
}

static int msys_rom_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    assert(fwk_id_is_equal(event->id, mod_msys_rom_notification_id_systop));
    assert(event->is_response == true);

    /* At least one notification response must be outstanding */
    if (ctx.notification_count == 0) {
        assert(false);
        return FWK_E_PANIC;
    }

    /* Complete remaining setup now that all subscribers have responded */
    if ((--ctx.notification_count) == 0)
        return msys_deferred_setup();

    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_msys_rom = {
    .name = "MSYS_ROM",
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = ROM_EVENT_COUNT,
    .notification_count = MOD_MSYS_ROM_NOTIFICATION_COUNT,
    .init = msys_rom_init,
    .bind = msys_rom_bind,
    .start = msys_rom_start,
    .process_event = msys_rom_process_event,
    .process_notification = msys_rom_process_notification,
};
