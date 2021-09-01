/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "scp_mmap.h"

#define FMW_MEM_MODE ARCH_MEM_MODE_SINGLE_REGION

/* RAM */
#define FMW_MEM0_SIZE SCP_RAM_SIZE
#define FMW_MEM0_BASE SCP_RAM_BASE

#endif /* FMW_MEMORY_H */
