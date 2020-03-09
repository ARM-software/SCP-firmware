/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_SYNQUACER_PPU_DRIVER_H
#define INTERNAL_SYNQUACER_PPU_DRIVER_H

#include <internal/reg_PPU.h>

#include <stdint.h>

#define PPU_INT_PPU0 20
#define PPU_INT_PPU1 21
#define PPU_INT_PPU2 22
#define PPU_INT_PPU3 23
#define PPU_INT_PPU4 24
#define PPU_INT_PPU5 25
#define PPU_INT_PPU6 26
#define PPU_INT_PPU7 27
#define PPU_INT_PPU8 28

#define PPU_PP_OFF 0x01
#define PPU_PP_MEM_RET 0x02
#define PPU_PP_ON 0x10

#define PPU_STATUS_MASK 0x1F

#define PPU_Error 1
#define PPU_NoError 0

uint32_t get_domain_base_address(int domain);

int change_power_state(
    int domain,
    int next_power_policy,
    int hwcactiveen,
    int hwcsysreqen,
    int reten);
int read_power_status(int domain);

#endif /* INTERNAL_SYNQUACER_PPU_DRIVER_H */
