/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RAM firmware memory layout for the linker script.
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "scp_mmap.h"

#define FMW_MEM_MODE ARCH_MEM_MODE_DUAL_REGION_RELOCATION

/*
 * RAM instruction memory
 */
#define FMW_MEM0_SIZE SCP_ITC_RAM_SIZE
#define FMW_MEM0_BASE SCP_ITC_RAM_BASE

/*
 * RAM data memory
 */
#define FMW_MEM1_SIZE SCP_DTC_RAM_SIZE
#define FMW_MEM1_BASE SCP_DTC_RAM_BASE

#endif /* FMW_MEMORY_H */
