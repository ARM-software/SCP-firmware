/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <n1sdp_fip.h>

#include <mod_fip.h>
#include <mod_n1sdp_rom.h>

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

/*
 * Module context
 */
struct mod_n1sdp_rom_ctx {
    /* ROM configuration structure */
    const struct n1sdp_rom_config *rom_config;

    /* Pointer to FIP API */
    struct mod_fip_api *fip_api;
};

enum rom_event {
    ROM_EVENT_RUN,
    ROM_EVENT_COUNT
};

static struct mod_n1sdp_rom_ctx n1sdp_rom_ctx;

static void jump_to_ramfw(void)
{
    uintptr_t ramfw_base = n1sdp_rom_ctx.rom_config->ramfw_base;
    uintptr_t const *reset_base = (uintptr_t *)(ramfw_base + 0x4);
    void (*ramfw_reset_handler)(void);

    /*
     * Disable interrupts for the duration of the ROM firmware to RAM firmware
     * transition.
     */
    (void)fwk_interrupt_global_disable();

    ramfw_reset_handler = (void (*)(void))*reset_base;

    /* Set the vector table offset register */
    SCB->VTOR = ramfw_base;

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
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_FIP),
            FWK_ID_API(FWK_MODULE_IDX_FIP, 0),
            &n1sdp_rom_ctx.fip_api);
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

    return fwk_put_event(&event);
}

static const char *get_image_type_str(enum mod_fip_toc_entry_type type)
{
    if (type == (enum mod_fip_toc_entry_type)MOD_N1SDP_FIP_TOC_ENTRY_MCP_BL2)
        return "MCP";
    if (type == MOD_FIP_TOC_ENTRY_SCP_BL2)
        return "SCP";
    return "???";
}

static int n1sdp_rom_process_event(const struct fwk_event *event,
    struct fwk_event *resp)
{
    struct mod_fip_entry_data entry;
    int status = n1sdp_rom_ctx.fip_api->get_entry(
        n1sdp_rom_ctx.rom_config->image_type,
        &entry,
        n1sdp_rom_ctx.rom_config->fip_base_address,
        n1sdp_rom_ctx.rom_config->fip_nvm_size);

    const char *image_type =
        get_image_type_str(n1sdp_rom_ctx.rom_config->image_type);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[ROM] Failed to locate %s_BL2, error: %d", image_type, status);
        return status;
    }
#else
    (void)status;
    (void)image_type;
#endif

    FWK_LOG_INFO("[ROM] Located %s_BL2:", image_type);
    FWK_LOG_INFO("[ROM]   address: %p", entry.base);
    FWK_LOG_INFO("[ROM]   size   : %u", entry.size);
    FWK_LOG_INFO(
        "[ROM]   flags  : 0x%08" PRIX32 "%08" PRIX32 "",
        (uint32_t)(entry.flags >> 32),
        (uint32_t)entry.flags);
    FWK_LOG_INFO("[ROM] Copying %s_BL2 to ITCRAM...!", image_type);

    memcpy(
        (void *)n1sdp_rom_ctx.rom_config->ramfw_base, entry.base, entry.size);
    FWK_LOG_INFO("[ROM] Done!");

    FWK_LOG_INFO("[ROM] Jumping to %s_BL2", image_type);

    jump_to_ramfw();
    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_n1sdp_rom = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = ROM_EVENT_COUNT,
    .init = n1sdp_rom_init,
    .bind = n1sdp_rom_bind,
    .start = n1sdp_rom_start,
    .process_event = n1sdp_rom_process_event,
};
