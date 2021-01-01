/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "scp_mmap.h"

#define FMW_MEM_MODE ARCH_MEM_MODE_DUAL_REGION_RELOCATION

/* ROM */
#define FMW_MEM0_SIZE SCP_ROM_SIZE
#define FMW_MEM0_BASE SCP_ROM_BASE

/* RAM */
#define FMW_MEM1_SIZE (16 * 1024)
#define FMW_MEM1_BASE (SCP_RAM_BASE + SCP_RAM_SIZE - FMW_MEM1_SIZE)

#endif /* FMW_MEMORY_H */
