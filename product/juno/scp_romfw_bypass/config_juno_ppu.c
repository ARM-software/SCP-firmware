/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_ppu_idx.h"
#include "system_mmap.h"

#include <mod_juno_ppu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stddef.h>

static struct fwk_element element_table[] = {
    [JUNO_PPU_DEV_IDX_BIG_SSTOP] = {
        .name = "BIG_SSTOP",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_BIG_SSTOP_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_BIG_CPU0] = {
        .name = "BIG_CPU0",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_BIG_CPU0_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_BIG_CPU1] = {
        .name = "BIG_CPU1",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_BIG_CPU1_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_LITTLE_SSTOP] = {
        .name = "LITTLE_SSTOP",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_SSTOP_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_LITTLE_CPU0] = {
        .name = "LITTLE_CPU0",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_CPU0_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_LITTLE_CPU1] = {
        .name = "LITTLE_CPU1",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_CPU1_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_LITTLE_CPU2] = {
        .name = "LITTLE_CPU2",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_CPU2_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_LITTLE_CPU3] = {
        .name = "LITTLE_CPU3",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_CPU3_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_GPUTOP] = {
        .name = "GPUTOP",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_GPUTOP_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_SYSTOP] = {
        .name = "SYSTOP",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_SYSTOP_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_DBGSYS] = {
        .name = "DBGSYS",
        .data = &(struct mod_juno_ppu_element_config) {
            .reg_base = PPU_DBGSYS_BASE,
            .timer_id = FWK_ID_NONE_INIT,
        },
    },

    [JUNO_PPU_DEV_IDX_COUNT] = {0},
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_juno_ppu = {
    .get_element_table = get_element_table,
    .data = NULL,
};
