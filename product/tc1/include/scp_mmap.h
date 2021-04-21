/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_MMAP_H
#define SCP_MMAP_H

#define SCP_SYSTEM_ACCESS_PORT0_BASE UINT32_C(0x60000000)

#define SCP_UART_BOARD_BASE (SCP_SYSTEM_ACCESS_PORT0_BASE + 0x3FF70000)

#endif /* SCP_MMAP_H */
