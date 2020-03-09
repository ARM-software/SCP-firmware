/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RAM firmware memory layout for the linker script.
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "n1sdp_mcp_system_mmap.h"

#define FIRMWARE_MEM_MODE FWK_MEM_MODE_DUAL_REGION_RELOCATION

/*
 * RAM instruction memory
 */
#define FIRMWARE_MEM0_SIZE MCP_RAM0_SIZE
#define FIRMWARE_MEM0_BASE MCP_RAM0_BASE

/*
 * RAM data memory
 */
#define FIRMWARE_MEM1_SIZE MCP_RAM1_SIZE
#define FIRMWARE_MEM1_BASE MCP_RAM1_BASE

#endif /* FMW_MEMORY_H */
