/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_CSS_MMAP_H
#define SCP_CSS_MMAP_H

#include "scp_mmap.h"

#define SCP_CMN_RHODES_BASE          (SCP_SYSTEM_ACCESS_PORT0_BASE + 0x10000000)

#define SCP_REFCLK_CNTCONTROL_BASE   (SCP_SYSTEM_ACCESS_PORT1_BASE + 0x2A430000)
#define SCP_SID_BASE                 (SCP_SYSTEM_ACCESS_PORT1_BASE + 0x2A4A0000)

#define SCP_REFCLK_CNTCTL_BASE       (SCP_PERIPHERAL_BASE + 0x0000)
#define SCP_REFCLK_CNTBASE0_BASE     (SCP_PERIPHERAL_BASE + 0x1000)
#define SCP_UART_BASE                (SCP_PERIPHERAL_BASE + 0x2000)
#define SCP_MHU_AP_BASE              (SCP_PERIPHERAL_BASE + 0x1000000)

#define SCP_PIK_SCP_BASE             (SCP_ELEMENT_MANAGEMENT_PERIPHERAL_BASE)
#define SCP_PIK_CLUSTER_BASE(n)      ((SCP_ELEMENT_MANAGEMENT_PERIPHERAL_BASE \
                                        + 0x60000) + ((n) * 0x20000))
#define SCP_PIK_SYSTEM_BASE          (SCP_ELEMENT_MANAGEMENT_PERIPHERAL_BASE \
                                       + 0x40000)

#define SCP_PPU_SCP_BASE             (SCP_PIK_SCP_BASE + 0x1000)

#define SCP_PPU_SYS0_BASE            (SCP_PIK_SYSTEM_BASE + 0x1000)
#define SCP_PPU_SYS1_BASE            (SCP_PIK_SYSTEM_BASE + 0x5000)

#define SCP_PPU_BASE(n)              (SCP_PIK_CLUSTER_BASE(n) + 0x2000)

#define SCP_MHU_SCP_AP_SND_S_CLUS0   (SCP_MHU_AP_BASE + 0x400000)
#define SCP_MHU_SCP_AP_RCV_S_CLUS0   (SCP_MHU_AP_BASE + 0x410000)

#endif /* SCP_CSS_MMAP_H */
