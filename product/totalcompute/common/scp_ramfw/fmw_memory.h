/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RAM firmware memory layout for the linker script.
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "scp_mmap.h"

#define FMW_MEM_MODE ARCH_MEM_MODE_SINGLE_REGION

/* RAM */
#define FMW_MEM0_BASE SCP_RAM_BASE
#define FMW_MEM0_SIZE SCP_RAM_SIZE

#endif /* FMW_MEMORY_H */
