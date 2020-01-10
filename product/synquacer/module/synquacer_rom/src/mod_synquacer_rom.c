/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_synquacer_rom.h>

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

void synquacer_system_init(void);

static const struct synquacer_rom_config *rom_config;

enum rom_event { ROM_EVENT_RUN, ROM_EVENT_COUNT };

/*
 * This function assumes that the RAM firmware image is located at the beginning
 * of the SCP SRAM. The reset handler will be at offset 0x4 (the second entry of
 * the vector table).
 */
static void jump_to_ramfw(void)
{
    uintptr_t const *reset_base = (uintptr_t *)(rom_config->ramfw_base + 0x4);
    void (*ramfw_reset_handler)(void);

    /*
     * Disable interrupts for the duration of the ROM firmware to RAM firmware
     * transition.
     */
    fwk_interrupt_global_disable();

    ramfw_reset_handler = (void (*)(void))(*reset_base);

    /*
     * Execute the RAM firmware's reset handler to pass control from ROM
     * firmware to the RAM firmware.
     */
    ramfw_reset_handler();
}

/*
 * Framework API
 */
static int synquacer_rom_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    rom_config = data;

    synquacer_system_init();

    return FWK_SUCCESS;
}

static int synquacer_rom_start(fwk_id_t id)
{
    int status;
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_SYNQUACER_ROM),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_SYNQUACER_ROM),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_SYNQUACER_ROM, ROM_EVENT_RUN),
    };

    status = fwk_thread_put_event(&event);

    return status;
}

static int synquacer_rom_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    FWK_LOG_INFO("[scp_romfw] Launch scp_ramfw");

    if (rom_config->load_ram_size != 0) {
        memcpy(
            (void *)rom_config->ramfw_base,
            (uint8_t *)rom_config->nor_base,
            rom_config->load_ram_size);
    }

    jump_to_ramfw();

    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_synquacer_rom = {
    .name = "synquacer_ROM",
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = ROM_EVENT_COUNT,
    .init = synquacer_rom_init,
    .start = synquacer_rom_start,
    .process_event = synquacer_rom_process_event,
};
