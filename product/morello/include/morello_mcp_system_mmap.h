/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MORELLO_MCP_SYSTEM_MMAP_H
#define MORELLO_MCP_SYSTEM_MMAP_H

/*
 * External QSPI flash memory - mapped address
 */
#define MCP_QSPI_FLASH_BASE_ADDR     0x30000000
#define MCP_QSPI_FLASH_BASE_ADDR_ALT 0x00840000
#define MCP_QSPI_FLASH_SIZE 0x02000000

/*
 * Internal MCP's ROM/RAM base address
 */
#define MCP_ROM_BASE 0x00000000
#define MCP_RAM0_BASE 0x00800000
#define MCP_RAM1_BASE 0x20000000

/*
 * Internal MCP's ROM/RAM sizes
 */
#define MCP_ROM_SIZE (128 * 1024)
#define MCP_RAM0_SIZE (512 * 1024)
#define MCP_RAM1_SIZE (256 * 1024)

/*
 * MCP_BACKUP_DDR3 memory area where the MCP ROM
 * bypass firmware should be placed. The MCP would
 * be programmed to boot at this address by programming
 * it in the MCP_BOOT_ADR SCC register.
 */
#define MCP_BACKUP_DDR3_BASE 0x16000000
#define MCP_BACKUP_DDR3_SIZE (32 * 1024 * 1024)

#endif /* MORELLO_MCP_SYSTEM_MMAP_H */
