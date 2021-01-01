/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     ROM firmware memory layout for the linker script.
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "system_mmap_scp.h"

#define FMW_MEM_MODE ARCH_MEM_MODE_DUAL_REGION_RELOCATION

/*
 * ROM memory
 */
#define FMW_MEM0_SIZE SCP_ROM_SIZE
#define FMW_MEM0_BASE SCP_ROM_BASE

/*
 * RAM memory (16 KiB block at the top of the RAM)
 *
 * Note: The sections that must go into the RAM memory (i.e. stack, heap, bss
 * and data) are placed at the end of the RAM memory to avoid being overwritten
 * by the bootloader when loading the RAM firmware image.
 */
#define FMW_MEM1_SIZE (16 * 1024)
#define FMW_MEM1_BASE (SCP_RAM_BASE + SCP_RAM_SIZE - FMW_MEM1_SIZE)

#endif /* FMW_MEMORY_H */
