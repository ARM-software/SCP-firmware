/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MORELLO_SCP_SYSTEM_MMAP_H
#define MORELLO_SCP_SYSTEM_MMAP_H

/*
 * External QSPI flash memory - mapped address
 */
#define SCP_QSPI_FLASH_BASE_ADDR 0x30000000
#define SCP_QSPI_FLASH_BASE_ADDR_ALT 0x00840000
#define SCP_QSPI_FLASH_SIZE 0x02000000

/*
 * Internal SCP's ROM/RAM base address
 */
#define SCP_ROM_BASE 0x00000000
#define SCP_RAM0_BASE 0x00800000
#define SCP_RAM1_BASE 0x20000000

/*
 * Internal SCP's ROM/RAM sizes
 */
#define SCP_ROM_SIZE (128 * 1024)
#define SCP_RAM0_SIZE (512 * 1024)
#define SCP_RAM1_SIZE (256 * 1024)

#endif /* MORELLO_SCP_SYSTEM_MMAP_H*/
