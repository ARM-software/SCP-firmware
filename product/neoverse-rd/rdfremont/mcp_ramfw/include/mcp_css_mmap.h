/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Base address definitions for the MCP's sub-system and access extending
 *     into the rest of the CSS.
 */

#ifndef MCP_CSS_MMAP_H
#define MCP_CSS_MMAP_H

/* Base address and size of MCP's ITCM */
#define MCP_ITC_RAM_BASE (0x00000000)
#define MCP_ITC_RAM_SIZE (256 * 1024)

/* Base address and size of MCP's DTCM */
#define MCP_DTC_RAM_BASE (0x20000000)
#define MCP_DTC_RAM_SIZE (256 * 1024)

/* SCP sub-system peripherals */
#define MCP_UART_BASE (0x4C002000)

#endif /* MCP_CSS_MMAP_H */
