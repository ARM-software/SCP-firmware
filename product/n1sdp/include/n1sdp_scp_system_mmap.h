/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_SCP_SYSTEM_MMAP_H
#define N1SDP_SCP_SYSTEM_MMAP_H

/*
 * External QSPI flash memory - mapped address
 */
#define SCP_QSPI_FLASH_BASE_ADDR     0x30000000
#define SCP_QSPI_FLASH_BASE_ADDR_ALT 0x00840000
#define SCP_QSPI_FLASH_SIZE          0x02000000

/*
 * Internal SCP's ROM/RAM base address
 */
#define SCP_ROM_BASE                 0x00000000
#define SCP_RAM0_BASE                0x00800000
#define SCP_RAM1_BASE                0x20000000

/*
 * Internal SCP's ROM/RAM sizes
 */
#define SCP_ROM_SIZE                 (128 * 1024)
#define SCP_RAM0_SIZE                (512 * 1024)
#define SCP_RAM1_SIZE                (256 * 1024)

/*
 * SCP_BACKUP_DDR3 memory area where the SCP ROM
 * bypass firmware should be placed. The SCP would
 * be programmed to boot at this address by programming
 * it in the SCP_BOOT_ADR SCC register.
 */
#define SCP_BACKUP_DDR3_BASE          0x14000000
#define SCP_BACKUP_DDR3_SIZE          (32 * 1024 * 1024)

#endif /* N1SDP_SCP_SYSTEM_MMAP_H*/
