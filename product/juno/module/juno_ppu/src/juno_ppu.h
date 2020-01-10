/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_PPU_H
#define JUNO_PPU_H

#include <mod_juno_ppu.h>

#include <fwk_macros.h>

#include <stdint.h>

struct ppu_reg {
    FWK_RW uint32_t POWER_POLICY;
    FWK_R  uint32_t POWER_STATUS;
           uint32_t RESERVED;
    FWK_RW uint32_t POWER_CONFIG;
};

#define PPU_REG_PPR_PSR         UINT32_C(0x0000001F)
#define PPU_PPR_POLICY_OFF      UINT32_C(0x00000001)
#define PPU_PPR_POLICY_ON       UINT32_C(0x00000010)

enum ppu_mode {
    PPU_MODE_OFF = 0x01,
    PPU_MODE_MEM_RET = 0x02,
    PPU_MODE_RESERVED = 0x04,
    PPU_MODE_WARM_RESET = 0x08,
    PPU_MODE_ON = 0x10,
};

struct ppu_ctx {
    /* Pointer to the element's configuration data */
    const struct mod_juno_ppu_element_config *config;

    /* Power Policy Unit base register */
    struct ppu_reg *reg;

    /* Identifier of the entity that bound to this PPU element */
    fwk_id_t bound_id;

    /* Timer API */
    const struct mod_timer_api *timer_api;

    /* Power domain driver input API */
    const struct mod_pd_driver_input_api *pd_api;
};

struct module_ctx {
    /* Table of element context structures */
    struct ppu_ctx *ppu_ctx_table;

    /* Timer alarm API */
    const struct mod_timer_alarm_api *alarm_api;

    /* CSS power state */
    unsigned int css_state;

    /* DBGSYS power state */
    unsigned int dbgsys_state;
};

#endif /* JUNO_PPU_H */
