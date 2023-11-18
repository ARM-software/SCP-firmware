/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Software defined memory map shared between MCP and AP cores.
 */

#ifndef MCP_SOFTWARE_MMAP_H
#define MCP_SOFTWARE_MMAP_H

#include "mcp_rdn1e1_mmap.h"

#include <fwk_macros.h>

#define MCP_NOR_FLASH_BASE 0x08000000
#define MCP_NOR_BASE       (MCP_NOR_FLASH_BASE + 0x03F80000)
#define MCP_IMAGE_SIZE     (256 * FWK_KIB)

#endif /* MCP_SOFTWARE_MMAP_H */
