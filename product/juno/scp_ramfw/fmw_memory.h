/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_MEMORY_H
#define FMW_MEMORY_H

#include "scp_mmap.h"

#define FIRMWARE_MEM_MODE FWK_MEM_MODE_SINGLE_REGION

/* RAM */
#define FIRMWARE_MEM0_SIZE SCP_RAM_SIZE
#define FIRMWARE_MEM0_BASE SCP_RAM_BASE

#endif /* FMW_MEMORY_H */
