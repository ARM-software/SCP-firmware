/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Base address definitions for the SCP's sub-system and access extending
 *     into the rest of the CSS.
 */

#ifndef SCP_CSS_MMAP_H
#define SCP_CSS_MMAP_H

/* Base address and size of SCP's ITCM */
#define SCP_ITC_RAM_BASE (0x00000000)
#define SCP_ITC_RAM_SIZE (256 * 1024)

/* Base address and size of SCP's DTCM */
#define SCP_DTC_RAM_BASE (0x20000000)
#define SCP_DTC_RAM_SIZE (256 * 1024)

/* SCP sub-system peripherals */
#define SCP_SID_BASE (0x2A4A0000)
#define SCP_UART_BASE (0x44002000)

/* SCP addresses mapped via ATU into address translation windows */
#define SCP_ADDRESS_TRANSLATION_WINDOW0_BASE (0x60000000)

/* Offsets within SCP's Address Translation Window0 */
#define SCP_ATW0_AP_PERIPHERAL_SRAM_OFFSET (0x10000000)

#endif /* SCP_CSS_MMAP_H */
