/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_CSS_MMAP_H
#define SCP_CSS_MMAP_H

#include "scp_mmap.h"

#define SCP_UART_BASE                (SCP_PERIPHERAL_BASE + 0x2000)

#define SCP_PIK_SCP_BASE             (SCP_ELEMENT_MANAGEMENT_PERIPHERAL_BASE)
#define SCP_PIK_CLUSTER_BASE(n)      ((SCP_ELEMENT_MANAGEMENT_PERIPHERAL_BASE \
                                        + 0x60000) + ((n) * 0x20000))
#define SCP_PIK_SYSTEM_BASE          (SCP_ELEMENT_MANAGEMENT_PERIPHERAL_BASE \
                                       + 0x40000)

#define SCP_PPU_CLUSTER_BASE(n)      (SCP_PIK_CLUSTER_BASE((n)) + 0x1000)
#define SCP_PPU_CORE_BASE(n, m)      (SCP_PPU_CLUSTER_BASE((n)) + \
                                       ((m) + 1) * 0x1000)

#define SCP_PPU_SYS0_BASE            (SCP_PIK_SYSTEM_BASE + 0x1000)
#define SCP_PPU_SYS1_BASE            (SCP_PIK_SYSTEM_BASE + 0x5000)

#define SCP_PPU_BASE(n)              (SCP_PIK_CLUSTER_BASE(n) + 0x2000)
#endif /* SCP_CSS_MMAP_H */
