/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCP_RDDANIEL_MMAP_H
#define MCP_RDDANIEL_MMAP_H

#include <stdint.h>

/*
 * Top-level base addresses
 */
#define MCP_PERIPH_BASE               UINT32_C(0x4C000000)

/*
 * Peripherals
 */
#define MCP_UART0_BASE                (MCP_PERIPH_BASE + 0x2000)

#endif /* MCP_RDDANIEL_MMAP_H */
