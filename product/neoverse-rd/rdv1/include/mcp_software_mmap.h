/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCP_SOFTWARE_MMAP_MCP_H
#define MCP_SOFTWARE_MMAP_MCP_H

#include <mcp_mmap.h>

#include <fwk_macros.h>

/* MCP ROM and RAM firmware size loaded on main memory */
#define MCP_BOOT_ROM_SIZE (64 * 1024)
#define MCP_DTC_RAM_SIZE  (256 * 1024)
#define MCP_ITC_RAM_SIZE  (256 * 1024)

/* MCP RAM firmware base and size on the flash */
#define MCP_NOR0_FLASH_BASE        (MCP_SOC_EXPANSION1_BASE + 0x07000000)
#define MCP_RAMFW_IMAGE_FLASH_BASE (MCP_NOR0_FLASH_BASE + 0x03F80000)
#define MCP_RAMFW_IMAGE_FLASH_SIZE (256 * FWK_KIB)

#endif /* MCP_SOFTWARE_MMAP_MCP_H */
