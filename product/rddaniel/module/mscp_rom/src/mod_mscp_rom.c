/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mscp_rom.h>

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

static const struct mscp_rom_config *rom_config;

enum rom_event {
    ROM_EVENT_RUN,
    ROM_EVENT_COUNT
};

/*
 * This function assumes that the RAM firmware image is located at the beginning
 * of the SCP/MCP RAM. The reset handler will be at offset 0x4 (the second entry
 * of the vector table).
 */
static void jump_to_ramfw(void)
{
    uintptr_t const *reset_base =
        (uintptr_t *)(rom_config->ramfw_ram_base + 0x4);
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

static int mscp_rom_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    rom_config = data;

    return FWK_SUCCESS;
}

static int mscp_rom_start(fwk_id_t id)
{
    int status;
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_MSCP_ROM),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_MSCP_ROM),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_MSCP_ROM, ROM_EVENT_RUN),
    };

    status = fwk_thread_put_event(&event);

    return status;
}

static int mscp_rom_process_event(const struct fwk_event *event,
                                      struct fwk_event *resp)
{
    FWK_LOG_INFO("[ROM] Loading RAM firmware");

    if (rom_config->ramfw_flash_size != 0) {
        memcpy((void *)rom_config->ramfw_ram_base,
            (uint8_t *)rom_config->ramfw_flash_base,
            rom_config->ramfw_flash_size);
    }

    FWK_LOG_INFO("[ROM] Starting RAM firmware execution");
    jump_to_ramfw();

    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_mscp_rom = {
    .name = "MSCP_ROM",
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = ROM_EVENT_COUNT,
    .init = mscp_rom_init,
    .start = mscp_rom_start,
    .process_event = mscp_rom_process_event,
};
