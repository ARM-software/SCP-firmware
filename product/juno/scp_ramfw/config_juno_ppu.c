/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_alarm_idx.h"
#include "juno_irq.h"
#include "juno_ppu_idx.h"
#include "system_mmap.h"

#include <mod_juno_ppu.h>
#include <mod_power_domain.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static struct fwk_element element_table[] = {
    [JUNO_PPU_DEV_IDX_BIG_SSTOP] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_BIG_SSTOP_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_CLUSTER,
        },
    },
    [JUNO_PPU_DEV_IDX_BIG_CPU0] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_BIG_CPU0_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_CORE,
            .wakeup_irq = BIG_0_IRQ_WAKEUP_IRQ,
            .wakeup_fiq = BIG_0_FIQ_WAKEUP_IRQ,
            .warm_reset_irq = BIG_0_WARM_RST_REQ_IRQ,
        },
    },
    [JUNO_PPU_DEV_IDX_BIG_CPU1] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_BIG_CPU1_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_CORE,
            .wakeup_irq = BIG_1_IRQ_WAKEUP_IRQ,
            .wakeup_fiq = BIG_1_FIQ_WAKEUP_IRQ,
            .warm_reset_irq = BIG_1_WARM_RST_REQ_IRQ,
        },
    },
    [JUNO_PPU_DEV_IDX_LITTLE_SSTOP] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_SSTOP_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_CLUSTER,
        },
    },
    [JUNO_PPU_DEV_IDX_LITTLE_CPU0] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_CPU0_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_CORE,
            .wakeup_irq = LITTLE_0_IRQ_WAKEUP_IRQ,
            .wakeup_fiq = LITTLE_0_FIQ_WAKEUP_IRQ,
            .warm_reset_irq = LITTLE_0_WARM_RST_REQ_IRQ,
        },
    },
    [JUNO_PPU_DEV_IDX_LITTLE_CPU1] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_CPU1_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_CORE,
            .wakeup_irq = LITTLE_1_IRQ_WAKEUP_IRQ,
            .wakeup_fiq = LITTLE_1_FIQ_WAKEUP_IRQ,
            .warm_reset_irq = LITTLE_1_WARM_RST_REQ_IRQ,
        },
    },
    [JUNO_PPU_DEV_IDX_LITTLE_CPU2] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_CPU2_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_CORE,
            .wakeup_irq = LITTLE_2_IRQ_WAKEUP_IRQ,
            .wakeup_fiq = LITTLE_2_FIQ_WAKEUP_IRQ,
            .warm_reset_irq = LITTLE_2_WARM_RST_REQ_IRQ,
        },
    },
    [JUNO_PPU_DEV_IDX_LITTLE_CPU3] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_LITTLE_CPU3_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_CORE,
            .wakeup_irq = LITTLE_3_IRQ_WAKEUP_IRQ,
            .wakeup_fiq = LITTLE_3_FIQ_WAKEUP_IRQ,
            .warm_reset_irq = LITTLE_3_WARM_RST_REQ_IRQ,
        },
    },
    [JUNO_PPU_DEV_IDX_GPUTOP] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_GPUTOP_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_DEVICE,
        },
    },
    [JUNO_PPU_DEV_IDX_SYSTOP] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_SYSTOP_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_SYSTEM,
        },
    },
    [JUNO_PPU_DEV_IDX_DBGSYS] = {
        .name = "",
        .data = &(const struct mod_juno_ppu_element_config) {
            .reg_base = PPU_DBGSYS_BASE,
            .timer_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0),
            .pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
        },
    },
    [JUNO_PPU_DEV_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_juno_ppu = {
    .get_element_table = get_element_table,
    .data = &((struct mod_juno_ppu_config) {
        .timer_alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER,
                                                  0,
                                                  JUNO_PPU_ALARM_IDX),
    }),
};
