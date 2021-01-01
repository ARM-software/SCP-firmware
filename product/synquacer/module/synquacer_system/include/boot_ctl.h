/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOOT_CTL_H
#define BOOT_CTL_H

#include <stdint.h>

uint8_t get_dsw3_status(uint8_t bit_mask);
void set_memory_remap(uint32_t value);

#endif /* BOOT_CTL_H */
