/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_PPU_H
#define JUNO_PPU_H

#include <stdint.h>
#include <fwk_macros.h>
#include <mod_juno_ppu.h>

struct ppu_reg {
    FWK_RW uint32_t POWER_POLICY;
    FWK_R  uint32_t POWER_STATUS;
           uint32_t RESERVED;
    FWK_RW uint32_t POWER_CONFIG;
};

#define PPU_REG_PPR_PSR UINT32_C(0x0000001F)

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

    /* Power domain API */
    const struct mod_pd_driver_input_api *pd_api;

    /* PSU API */
    const struct mod_psu_device_api *psu_api;
};

struct module_ctx {
    struct ppu_ctx *ppu_ctx_table;
};

#endif /* JUNO_PPU_H */
