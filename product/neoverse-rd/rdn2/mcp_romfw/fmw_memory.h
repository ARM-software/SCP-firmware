/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     ROM firmware memory layout for the linker script.
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "mcp_software_mmap.h"

#define FMW_MEM_MODE ARCH_MEM_MODE_DUAL_REGION_RELOCATION

/*
 * ROM memory
 */
#define FMW_MEM0_SIZE MCP_BOOT_ROM_SIZE
#define FMW_MEM0_BASE MCP_BOOT_ROM_BASE

/*
 * RAM memory
 */
#define FMW_MEM1_SIZE MCP_DTC_RAM_SIZE
#define FMW_MEM1_BASE MCP_DTC_RAM_BASE

#endif /* FMW_MEMORY_H */
