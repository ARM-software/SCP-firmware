/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_bootloader.h>

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

struct mod_isys_rom_ctx {
    const struct mod_bootloader_api *bootloader_api;
} ctx;

enum mod_isys_rom_event {
    MOD_ISYS_ROM_EVENT_RUN,
    MOD_ISYS_ROM_EVENT_COUNT,
};

static int mod_isys_init(
    fwk_id_t id,
    unsigned int element_count,
    const void *data)
{
    return FWK_SUCCESS;
}

static int mod_isys_rom_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;

    if (round > 0)
        goto exit;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_BOOTLOADER),
        FWK_ID_API(FWK_MODULE_IDX_BOOTLOADER, 0),
        &ctx.bootloader_api);
    if (status != FWK_SUCCESS)
        status = FWK_E_PANIC;

exit:
    return status;
}

static int mod_isys_rom_start(fwk_id_t id)
{
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_ISYS_ROM),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_ISYS_ROM),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_ISYS_ROM, MOD_ISYS_ROM_EVENT_RUN),
    };

    return fwk_put_event(&event);
}

static int mod_isys_rom_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    int status;

    status = ctx.bootloader_api->load_image();

#if !(FWK_LOG_LEVEL < FWK_LOG_LEVEL_DISABLED)
    (void)status;
#endif

    FWK_LOG_CRIT(
        "[ISYS-ROM] Failed to load RAM firmware image: %s",
        fwk_status_str(status));

    fwk_trap();

    return FWK_E_PANIC;
}

const struct fwk_module module_isys_rom = {
    .type = FWK_MODULE_TYPE_SERVICE,

    .init = mod_isys_init,
    .bind = mod_isys_rom_bind,
    .start = mod_isys_rom_start,

    .process_event = mod_isys_rom_process_event,
    .event_count = MOD_ISYS_ROM_EVENT_COUNT,
};

const struct fwk_module_config config_isys_rom = { 0 };
