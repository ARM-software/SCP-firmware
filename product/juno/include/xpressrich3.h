/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     PLDA XpressRICH3 PCIe Controller definitions.
 */

#ifndef XPRESSRICH3_H
#define XPRESSRICH3_H

#include "juno_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

/* Controller Core Registers (ARM defined) */
struct xpressrich3_core_reg {
            uint8_t     RESERVED1[0x1000 - 0x0];
    FWK_RW  uint32_t    PLL_CONFIG;
    FWK_W   uint32_t    RESET_CONTROL;
    FWK_R   uint32_t    RESET_STATUS;
    FWK_RW  uint32_t    CLOCK_CONTROL;
            uint8_t     RESERVED2[0x3000 - 0x1010];
    FWK_RW  uint32_t    SECURE;
            uint8_t     RESERVED3[0xFFD0 - 0x3004];
    FWK_R   uint32_t    PID4;
            uint8_t     RESERVED4[0xFFE0-0xFFD4];
    FWK_R   uint32_t    PID0;
    FWK_R   uint32_t    PID1;
    FWK_R   uint32_t    PID2;
    FWK_R   uint32_t    PID3;
    FWK_R   uint32_t    ID0;
    FWK_R   uint32_t    ID1;
    FWK_R   uint32_t    ID2;
    FWK_R   uint32_t    ID3;
};

/* Root Complex Address Translation Registers */
struct xpressrich3_addr_trsl_reg {
    FWK_RW  uint32_t    SRC_ADDR_LO;
    FWK_RW  uint32_t    SRC_ADDR_UP;
    FWK_RW  uint32_t    TRSL_ADDR_LO;
    FWK_RW  uint32_t    TRSL_ADDR_UP;
    FWK_RW  uint32_t    TRSL_PARAM;
            uint8_t     RESERVED[0x18 - 0x14];
    FWK_R   uint64_t    TRSL_MASK;
};

/* Root Complex - Control and Status Registers */
struct xpressrich3_root_ctrl_status_reg {
    FWK_R   uint32_t    BRIDGE_VER;
    FWK_R   uint32_t    BRIDGE_BUS;
            uint8_t     RESERVED1[0x10 - 0x08];
    FWK_R   uint32_t    PCIE_IF_CONF;
    FWK_R   uint32_t    PCIE_BASIC_CONF;
    FWK_R   uint32_t    PCIE_BASIC_STATUS;
            uint8_t     RESERVED2[0x80 - 0x1C];
    FWK_R   uint32_t    GEN_SETTINGS;
            uint8_t     RESERVED3[0x90 - 0x84];
    FWK_RW  uint32_t    PCIE_VC_CRED_0;
    FWK_RW  uint32_t    PCIE_VC_CRED_1;
    FWK_RW  uint32_t    PCIE_PCI_IDS_0;
    FWK_RW  uint32_t    PCIE_PCI_IDS_1;
    FWK_RW  uint32_t    PCIE_PCI_IDS_2;
    FWK_RW  uint32_t    PCIE_PCI_LPM;
    FWK_RW  uint32_t    PCIE_PCI_IRQ_0;
    FWK_RW  uint32_t    PCIE_PCI_IRQ_1;
    FWK_RW  uint32_t    PCIE_PCI_IRQ_2;
            uint8_t     RESERVED4[0xC0 - 0xB4];
    FWK_RW  uint32_t    PCIE_PEX_DEV;
            uint8_t     RESERVED5[0xC8 - 0xC4];
    FWK_RW  uint32_t    PCIE_PEX_LINK;
            uint8_t     RESERVED6[0xD4 - 0xCC];
    FWK_RW  uint32_t    PCIE_PEX_SPC;
    FWK_RW  uint32_t    PCIE_PEX_SPC2;
    FWK_RW  uint32_t    PCIE_PEX_NTFS;
            uint8_t     RESERVED7[0xFC - 0xE0];
    FWK_RW  uint32_t    PCIE_BAR_WIN;
    FWK_RW  uint32_t    PCIE_EQ_PRESET_LANE_0_1;
    FWK_RW  uint32_t    PCIE_EQ_PRESET_LANE_2_3;
            uint8_t     RESERVED8[0x140 - 0x108];
    FWK_R   uint32_t    PCIE_CFGNUM;
            uint8_t     RESERVED9[0x174 - 0x144];
    FWK_RW  uint32_t    PM_CONF_0;
    FWK_RW  uint32_t    PM_CONF_1;
    FWK_RW  uint32_t    PM_CONF_2;
};

/* Root Complex - Interrupt and Event Registers */
struct xpressrich3_root_irq_event_reg {
    FWK_RW  uint32_t    IMASK_LOCAL;
    FWK_RW  uint32_t    ISTATUS_LOCAL;
            uint8_t     RESERVED1[0x190 - 0x188];
    FWK_R   uint32_t    IMSI_ADDR;
    FWK_RW  uint32_t    ISTATUS_MSI;
    FWK_RW  uint32_t    ICMD_PM;
            uint8_t     RESERVED2[0x1D8 - 0x19C];
    FWK_R   uint32_t    ISTATUS_P_ADT_WIN0;
    FWK_R   uint32_t    ISTATUS_P_ADT_WIN1;
    FWK_R   uint32_t    ISTATUS_P_ADT_SLV0;
            uint8_t     RESERVED3[0x600 - 0x1E4];
};

/* Root Complex Registers */
struct  xpressrich3_root_reg {
    struct xpressrich3_root_ctrl_status_reg CS;
    struct xpressrich3_root_irq_event_reg   IE;
    struct xpressrich3_addr_trsl_reg        ATR_PCIE_WIN0[8];
    struct xpressrich3_addr_trsl_reg        ATR_PCIE_WIN1[8];
    struct xpressrich3_addr_trsl_reg        ATR_AXI4_SLV0[8];
};

#define PCIE ((struct xpressrich3_core_reg *) PCIE_CONTROLLER_BASE)
#define PCIE_ROOT ((struct xpressrich3_root_reg *) PCIE_ROOT_PORT_BASE)

/* Field Definitions for PCIe Root Address Translation Registers */
#define PCIE_TRSL_PARAM_PCIE_MEMORY  UINT32_C(0x00000000)
#define PCIE_TRSL_PARAM_PCIE_CONF    UINT32_C(0x00000001)
#define PCIE_TRSL_PARAM_PCIE_IO      UINT32_C(0x00020000)
#define PCIE_TRSL_PARAM_AXIDEVICE    UINT32_C(0x00420004)
#define PCIE_TRSL_PARAM_AXIMEMORY    UINT32_C(0x004E0004)

#define PCIE_BASIC_STATUS_NEG_LINK_WIDTH_MASK  UINT32_C(0x000000FF)

#define PCIE_BAR_WIN_IO_WIN_ENABLE          UINT32_C(0x00000001)
#define PCIE_BAR_WIN_IO_WIN32_ENABLE        UINT32_C(0x00000002)
#define PCIE_BAR_WIN_PREFETCH_WIN_ENABLE    UINT32_C(0x00000004)
#define PCIE_BAR_WIN_PREFETCH_WIN64_ENABLE  UINT32_C(0x00000008)

#define PCIE_INT_LINE_A  UINT32_C(0x01000000)
#define PCIE_INT_LINE_B  UINT32_C(0x02000000)
#define PCIE_INT_LINE_C  UINT32_C(0x04000000)
#define PCIE_INT_LINE_D  UINT32_C(0x08000000)
#define PCIE_INT_MSI     UINT32_C(0x10000000)
#define PCIE_INT_LINES   (PCIE_INT_LINE_A | PCIE_INT_LINE_B |\
                          PCIE_INT_LINE_C | PCIE_INT_LINE_D)

#define PCIE_CONTROL_RESET_CONTROL_RCPHY_REL    UINT32_C(0x00000003)

#define PCIE_CONTROL_RESET_STATUS_RCPHYPLL_OUT  UINT32_C(0x00000007)

#endif /* XPRESSRICH3_H */
