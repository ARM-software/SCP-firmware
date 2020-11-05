/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_CSS_MMAP_H
#define SCP_CSS_MMAP_H

#include "scp_mmap.h"

#define SCP_SID_BASE (SCP_SYSTEM_ACCESS_PORT1_BASE + 0x2A4A0000)

#define SCP_UART_BASE (SCP_PERIPHERAL_BASE + 0x2000)

#define SCP_PIK_SCP_BASE (SCP_PIK_BASE)

#define SCP_PIK_SYSTEM_BASE (SCP_PIK_SCP_BASE + 0x40000)

#define SCP_PIK_CLUSTER_BASE(n) \
    (SCP_SYSTEM_ACCESS_PORT1_BASE + 0x20000000 + (n * 0x100000) + 0x60000)

#define SCP_PPU_SCP_BASE (SCP_PIK_SCP_BASE + 0x1000)

#define SCP_PPU_SYS0_BASE (SCP_PIK_SYSTEM_BASE + 0x1000)

#define SCP_CLUSTER_PPU_BASE(n) \
    (SCP_SYSTEM_ACCESS_PORT1_BASE + 0x20000000 + (n * 0x100000) + 0x30000)

#define SCP_CORE_PPU_BASE(n) \
    (SCP_SYSTEM_ACCESS_PORT1_BASE + 0x20000000 + (n * 0x100000) + 0x80000)

#endif /* SCP_CSS_MMAP_H */
