/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_n1sdp_flash.h>
#include <mod_n1sdp_rom.h>

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdint.h>
#include <string.h>

/*
 * Module context
 */
struct mod_n1sdp_rom_ctx {
    /* ROM configuration structure */
    const struct n1sdp_rom_config *rom_config;

    /* Pointer to n1sdp_flash API */
    struct mod_n1sdp_flash_api *flash_api;
};

enum rom_event {
    ROM_EVENT_RUN,
    ROM_EVENT_COUNT
};

static struct mod_n1sdp_rom_ctx n1sdp_rom_ctx;

static void jump_to_ramfw(void)
{
    uintptr_t const *reset_base =
        (uintptr_t *)(n1sdp_rom_ctx.rom_config->ramfw_base + 0x4);
    void (*ramfw_reset_handler)(void);

    /*
     * Disable interrupts for the duration of the ROM firmware to RAM firmware
     * transition.
     */
    fwk_interrupt_global_disable();

    ramfw_reset_handler = (void (*)(void))*reset_base;

    /*
     * Execute the RAM firmware's reset handler to pass control from ROM
     * firmware to the RAM firmware.
     */
    ramfw_reset_handler();
}

/*
 * Framework handlers
 */
static int n1sdp_rom_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    if ((data == NULL) || (element_count > 0))
        return FWK_E_PANIC;

    n1sdp_rom_ctx.rom_config = data;

    return FWK_SUCCESS;
}

static int n1sdp_rom_bind(fwk_id_t id, unsigned int round)
{
    int status;

    /* Use second round only (round numbering is zero-indexed) */
    if (round == 1) {
        /* Bind to the n1sdp_flash component */
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_FLASH),
                                 FWK_ID_API(FWK_MODULE_IDX_N1SDP_FLASH, 0),
                                 &n1sdp_rom_ctx.flash_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int n1sdp_rom_start(fwk_id_t id)
{
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_ROM),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_ROM),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_N1SDP_ROM, ROM_EVENT_RUN),
    };

    return fwk_thread_put_event(&event);
}

static int n1sdp_rom_process_event(const struct fwk_event *event,
    struct fwk_event *resp)
{
    struct mod_n1sdp_fip_descriptor *fip_desc_table = NULL;
    struct mod_n1sdp_fip_descriptor *fip_desc = NULL;
    unsigned int fip_count = 0;
    unsigned int i;
    int status;

    status = n1sdp_rom_ctx.flash_api->get_n1sdp_fip_descriptor_count(
                 FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_ROM),
                 &fip_count);
    if (status != FWK_SUCCESS)
        return status;
    status = n1sdp_rom_ctx.flash_api->get_n1sdp_fip_descriptor_table(
                 FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_ROM),
                 &fip_desc_table);
    if (status != FWK_SUCCESS)
        return status;

    for (i = 0; i < fip_count; i++) {
        fip_desc = &fip_desc_table[i];
        if (fip_desc->type != n1sdp_rom_ctx.rom_config->image_type)
            continue;

        if (fip_desc->size == 0)
            return FWK_E_DATA;

        if (fip_desc->type == MOD_N1SDP_FIP_TYPE_MCP_BL2) {
            FWK_LOG_INFO(
                "[ROM] Found MCP RAM Firmware at address: 0x%x,"
                " size: %d bytes, flags: 0x%x",
                fip_desc->address,
                fip_desc->size,
                fip_desc->flags);
            FWK_LOG_INFO("[ROM] Copying MCP RAM Firmware to ITCRAM...!");
        } else {
            FWK_LOG_INFO(
                "[ROM] Found SCP BL2 RAM Firmware at address: 0x%x,"
                " size: %d bytes, flags: 0x%x",
                fip_desc->address,
                fip_desc->size,
                fip_desc->flags);
            FWK_LOG_INFO("[ROM] Copying SCP RAM Firmware to ITCRAM...!");
        }
        break;
    }

    if (i >= fip_count)
        return FWK_E_DATA;

    memcpy((void *)n1sdp_rom_ctx.rom_config->ramfw_base,
        (uint8_t *)fip_desc->address, fip_desc->size);
    FWK_LOG_INFO("[ROM] Done!");

    FWK_LOG_INFO("[ROM] Jumping to RAM Firmware");

    jump_to_ramfw();

    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_n1sdp_rom = {
    .name = "N1SDP SCP ROM",
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = ROM_EVENT_COUNT,
    .init = n1sdp_rom_init,
    .bind = n1sdp_rom_bind,
    .start = n1sdp_rom_start,
    .process_event = n1sdp_rom_process_event,
};
