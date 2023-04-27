/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     ROM firmware memory layout for the linker script.
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "scp_mmap.h"

#define FMW_MEM_MODE ARCH_MEM_MODE_DUAL_REGION_RELOCATION

/*
 * ROM memory
 */
#define FMW_MEM0_SIZE SCP_BOOT_ROM_SIZE
#define FMW_MEM0_BASE SCP_BOOT_ROM_BASE

/*
 * RAM memory for scp_romfw (16 KiB block at the top of the RAM)
 *
 * The last 16 KiB of SCP RAM are used for scp_romfw data regions.
 * The start of the RAM is where the scp bootloader places the scp_ramfw
 * so the 2 areas don't overlap.
 */
#define FMW_MEM1_SIZE (16 * 1024)
#define FMW_MEM1_BASE (SCP_RAM_BASE + SCP_RAM_SIZE - FMW_MEM1_SIZE)

#endif /* FMW_MEMORY_H */
