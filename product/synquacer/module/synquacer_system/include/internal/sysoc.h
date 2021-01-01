/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_SYSOC_H
#define INTERNAL_SYSOC_H

#include <stdbool.h>
#include <stdint.h>

void sysoc_set_reset(uint32_t sysoc_addr, uint32_t value);

void sysoc_clr_reset(uint32_t sysoc_addr, uint32_t value);

uint32_t sysoc_read_reset_status(uint32_t sysoc_addr);

int sysoc_wait_status_change(
    uint32_t sysoc_addr,
    bool reset_set_flag,
    uint32_t set_bit);

#endif /* INTERNAL_SYSOC_H */
