/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCP_CSS_MMAP_H
#define MCP_CSS_MMAP_H

#include "mcp_mmap.h"

#define MCP_REFCLK_CNTCTL_BASE     (MCP_PERIPHERAL_BASE + 0x0000)
#define MCP_REFCLK_CNTBASE0_BASE   (MCP_PERIPHERAL_BASE + 0x1000)
#define MCP_REFCLK_CNTCONTROL_BASE (MCP_SYSTEM_ACCESS_PORT1_BASE + 0x2A430000)

#endif /* MCP_CSS_MMAP_H */
