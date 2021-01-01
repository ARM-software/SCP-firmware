/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_COMMON_H
#define RCAR_COMMON_H

#include <stdint.h>

void mstpcr_write(uint32_t mstpcr, uint32_t mstpsr, uint32_t target_bit);
void cpg_write(uintptr_t regadr, uint32_t regval);

void udelay(unsigned long usec);
void mdelay(unsigned long msecs);

#endif /* RCAR_COMMON_H */
