/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_scp_pik.h"

#include <morello_fip.h>

#include <mod_fip.h>
#include <mod_morello_rom.h>

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
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * Module context
 */
struct mod_morello_rom_ctx {
    /* ROM configuration structure */
    const struct morello_rom_config *rom_config;

    /* Pointer to FIP API */
    struct mod_fip_api *fip_api;
};

enum rom_event { ROM_EVENT_RUN, ROM_EVENT_COUNT };

static struct mod_morello_rom_ctx morello_rom_ctx;

static void jump_to_ramfw(void)
{
    uintptr_t ramfw_base = morello_rom_ctx.rom_config->ramfw_base;
    uintptr_t const *reset_base = (uintptr_t *)(ramfw_base + 0x4);
    void (*ramfw_reset_handler)(void);

    /*
     * Disable interrupts for the duration of the ROM firmware to RAM firmware
     * transition.
     */
    (void)fwk_interrupt_global_disable();

    ramfw_reset_handler = (void (*)(void)) * reset_base;

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
static int morello_rom_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if ((data == NULL) || (element_count > 0))
        return FWK_E_PANIC;

    morello_rom_ctx.rom_config = data;

    return FWK_SUCCESS;
}

static int morello_rom_bind(fwk_id_t id, unsigned int round)
{
    int status;

    /* Use second round only (round numbering is zero-indexed) */
    if (round == 1) {
        /* Bind to the fip parser module */
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_FIP),
            FWK_ID_API(FWK_MODULE_IDX_FIP, 0),
            &morello_rom_ctx.fip_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int morello_rom_start(fwk_id_t id)
{
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_ROM),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_ROM),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_MORELLO_ROM, ROM_EVENT_RUN),
    };

    return fwk_put_event(&event);
}

static const char *get_image_type_str(enum mod_fip_toc_entry_type type)
{
    if (type == (enum mod_fip_toc_entry_type)MOD_MORELLO_FIP_TOC_ENTRY_MCP_BL2)
        return "MCP";
    if (type == MOD_FIP_TOC_ENTRY_SCP_BL2)
        return "SCP";
    return "???";
}

static int morello_rom_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    struct mod_fip_entry_data entry;
    uintptr_t fip_base;
    size_t fip_size;
    int status;

    /*
     * SCP ROM has the flexibility to choose where to look for FIP header
     * based on the SCC BOOT_GPR0 register. If SCC BOOT_GPR0 has non-zero
     * value use that as FIP storage else use the base address passed from
     * config file.
     */
    if ((morello_rom_ctx.rom_config->image_type == MOD_FIP_TOC_ENTRY_SCP_BL2) &&
        (SCC->BOOT_GPR0 != 0x0)) {
        fip_base = SCC->BOOT_GPR0;
        /* Assume maximum size limit */
        fip_size = 0xFFFFFFFF;
    } else {
        fip_base = morello_rom_ctx.rom_config->fip_base_address;
        fip_size = morello_rom_ctx.rom_config->fip_nvm_size;
    }

    FWK_LOG_INFO("[ROM] Trying to identify FIP at 0x%X", fip_base);

    status = morello_rom_ctx.fip_api->get_entry(
        morello_rom_ctx.rom_config->image_type, &entry, fip_base, fip_size);
    const char *image_type =
        get_image_type_str(morello_rom_ctx.rom_config->image_type);
    (void)image_type;

    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            "[ROM] Failed to locate %s_BL2, error: %d", image_type, status);
        return status;
    }

    FWK_LOG_INFO("[ROM] Located %s_BL2:", image_type);
    FWK_LOG_INFO("[ROM]   address: %p", entry.base);
    FWK_LOG_INFO("[ROM]   size   : %u", entry.size);
    FWK_LOG_INFO(
        "[ROM]   flags  : 0x%08" PRIX32 "%08" PRIX32,
        (uint32_t)(entry.flags >> 32),
        (uint32_t)entry.flags);
    FWK_LOG_INFO("[ROM] Copying %s_BL2 to ITCRAM...!", image_type);

    memcpy(
        (void *)morello_rom_ctx.rom_config->ramfw_base, entry.base, entry.size);
    FWK_LOG_INFO("[ROM] Done!");

    FWK_LOG_INFO("[ROM] Jumping to %s_BL2", image_type);
    jump_to_ramfw();

    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_morello_rom = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = ROM_EVENT_COUNT,
    .init = morello_rom_init,
    .bind = morello_rom_bind,
    .start = morello_rom_start,
    .process_event = morello_rom_process_event,
};
