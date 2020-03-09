/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_CRG11_H
#define INTERNAL_CRG11_H

#include "synquacer_mmap.h"

#include <cmsis_os2.h>

#include <stdbool.h>
#include <stdint.h>

/* CRG11 is only used to reboot system */
#define CRG11_CRSWR(crg11_top_reg_addr) (crg11_top_reg_addr + 0x0024U)

#define INVALID_CRG11_ID (-1)
#define INVALID_LCRG (-1)

typedef struct crg11_state_s {
    unsigned int ps_mode : 4;
    unsigned int fb_mode : 6;
    uint32_t src_frequency;
    int8_t lcrg_numerator;
    uint32_t reg_addr;
    uint16_t clock_domain_div_modifiable_mask;
    uint8_t clock_domain_div[16];
    uint8_t port_gate[16];
    uint8_t gate_enable[16];
    uint8_t ref_count;
    unsigned int ap_change_allowed_flag : 1;
} crg11_state_t;

#endif /* INTERNAL_CRG11_H */
