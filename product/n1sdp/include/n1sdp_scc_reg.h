/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Configuration Controller (SCC) registers.
 */

#ifndef N1SDP_SCC_REG_H
#define N1SDP_SCC_REG_H

#include <fwk_macros.h>

#include <stdint.h>

struct scc_pcid_registers {
    FWK_R uint32_t PID4;
    FWK_R uint32_t PID0;
    FWK_R uint32_t PID1;
    FWK_R uint32_t PID2;
    FWK_R uint32_t PID3;
    FWK_R uint32_t CID0;
    FWK_R uint32_t CID1;
    FWK_R uint32_t CID2;
    FWK_R uint32_t CID3;
};

struct scc_reg {
            uint32_t  RESERVED0;
    FWK_RW  uint32_t  PMCLK_DIV;
            uint32_t  RESERVED1;
    FWK_RW  uint32_t  SYSAPBCLK_CTRL;
    FWK_RW  uint32_t  SYSAPBCLK_DIV;
            uint32_t  RESERVED2;
    FWK_RW  uint32_t  IOFPGA_TMIF2XCLK_CTRL;
    FWK_RW  uint32_t  IOFPGA_TMIF2XCLK_DIV;
            uint32_t  RESERVED3;
    FWK_RW  uint32_t  IOFPGA_TSIF2XCLK_CTRL;
    FWK_RW  uint32_t  IOFPGA_TSIF2XCLK_DIV;
            uint32_t  RESERVED4;
    FWK_RW  uint32_t  SCPNICCLK_CTRL;
    FWK_RW  uint32_t  SCPNICCLK_DIV;
            uint32_t  RESERVED5;
    FWK_RW  uint32_t  SCPI2CCLK_CTRL;
    FWK_RW  uint32_t  SCPI2CCLK_DIV;
            uint32_t  RESERVED6;
    FWK_RW  uint32_t  SCPQSPICLK_CTRL;
    FWK_RW  uint32_t  SCPQSPICLK_DIV;
            uint32_t  RESERVED7;
    FWK_RW  uint32_t  SENSORCLK_CTRL;
    FWK_RW  uint32_t  SENSORCLK_DIV;
            uint32_t  RESERVED8;
    FWK_RW  uint32_t  MCPNICCLK_CTRL;
    FWK_RW  uint32_t  MCPNICCLK_DIV;
            uint32_t  RESERVED9;
    FWK_RW  uint32_t  MCPI2CCLK_CTRL;
    FWK_RW  uint32_t  MCPI2CCLK_DIV;
            uint32_t  RESERVED10;
    FWK_RW  uint32_t  MCPQSPICLK_CTRL;
    FWK_RW  uint32_t  MCPQSPICLK_DIV;
            uint32_t  RESERVED11;
    FWK_RW  uint32_t  PCIEAXICLK_CTRL;
    FWK_RW  uint32_t  PCIEAXICLK_DIV;
            uint32_t  RESERVED12;
    FWK_RW  uint32_t  CCIXAXICLK_CTRL;
    FWK_RW  uint32_t  CCIXAXICLK_DIV;
            uint32_t  RESERVED13;
    FWK_RW  uint32_t  PCIEAPBCLK_CTRL;
    FWK_RW  uint32_t  PCIEAPBCLK_DIV;
            uint32_t  RESERVED14;
    FWK_RW  uint32_t  CCIXAPBCLK_CTRL;
    FWK_RW  uint32_t  CCIXAPBCLK_DIV;
            uint8_t   RESERVED15[0xF0 - 0xB0];
    FWK_RW  uint32_t  SYS_CLK_EN;
            uint8_t   RESERVED16[0x100 - 0xF4];

    /* PLL Control Registers */
    FWK_RW  uint32_t  CPU0_PLL_CTRL0;
    FWK_RW  uint32_t  CPU0_PLL_CTRL1;
    FWK_RW  uint32_t  CPU1_PLL_CTRL0;
    FWK_RW  uint32_t  CPU1_PLL_CTRL1;
    FWK_RW  uint32_t  CLUS_PLL_CTRL0;
    FWK_RW  uint32_t  CLUS_PLL_CTRL1;
    FWK_RW  uint32_t  SYS_PLL_CTRL0;
    FWK_RW  uint32_t  SYS_PLL_CTRL1;
    FWK_RW  uint32_t  DMC_PLL_CTRL0;
    FWK_RW  uint32_t  DMC_PLL_CTRL1;
    FWK_RW  uint32_t  INT_PLL_CTRL0;
    FWK_RW  uint32_t  INT_PLL_CTRL1;
            uint8_t   RESERVED17[0x150 - 0x130];

    /* Reset Control Registers */
    FWK_RW  uint32_t  SYS_MAN_RESET;
            uint8_t   RESERVED18[0x160 - 0x154];

    /* Boot Control Registers */
    FWK_RW  uint32_t  BOOT_CTRL;
    FWK_RW  uint32_t  BOOT_CTRL_STA;
    FWK_RW  uint32_t  SCP_BOOT_ADR;
    FWK_RW  uint32_t  MCP_BOOT_ADR;
    FWK_RW  uint32_t  PLATFORM_CTRL;
    FWK_RW  uint32_t  TARGET_ID_APP;
    FWK_RW  uint32_t  TARGET_ID_SCP;
    FWK_RW  uint32_t  TARGET_ID_MCP;
    FWK_RW  uint32_t  BOOT_GPR0;
    FWK_RW  uint32_t  BOOT_GPR1;
    FWK_RW  uint32_t  BOOT_GPR2;
    FWK_RW  uint32_t  BOOT_GPR3;
    FWK_RW  uint32_t  BOOT_GPR4;
    FWK_RW  uint32_t  BOOT_GPR5;
    FWK_RW  uint32_t  BOOT_GPR6;
    FWK_RW  uint32_t  BOOT_GPR7;
    FWK_RW  uint32_t  INSTANCE_ID;
    FWK_RW  uint32_t  PCIE_BOOT_CTRL;
    FWK_RW  uint32_t  MISC_BOOT_CTRL;
            uint32_t  RESERVED19;

    /* Component Control Registers */
    FWK_RW  uint32_t  TEST_MUX_CTRL;
    FWK_RW  uint32_t  DBG_AUTHN_CTRL;
    FWK_RW  uint32_t  CTI0_CTRL;
    FWK_RW  uint32_t  CTI1_CTRL;
    FWK_RW  uint32_t  CTI0TO3_CTRL;
    FWK_RW  uint32_t  MCP_WDOGCTI_CTRL;
    FWK_RW  uint32_t  SCP_WDOGCTI_CTRL;
    FWK_RW  uint32_t  DBGEXPCTI_CTRL;
    FWK_RW  uint32_t  PCIE_PM_CTRL;
    FWK_RW  uint32_t  CCIX_PM_CTRL;
    FWK_RW  uint32_t  SCDBG_CTRL;
    FWK_RW  uint32_t  EXP_IF_CTRL;
    FWK_RW  uint32_t  PCIE_TEST_MUX_CTRL;
    FWK_RW  uint32_t  RO_CTRL;
    FWK_RW  uint32_t  CMN_CCIX_CTRL;
    FWK_RW  uint32_t  STM_CTRL;
    FWK_RW  uint32_t  AXI_OVRD_PCIE;
    FWK_RW  uint32_t  AXI_OVRD_CCIX;
    FWK_RW  uint32_t  AXI_OVRD_TSIF;
            uint32_t  RESERVED20;

    /* Pad Control Registers */
    FWK_RW  uint32_t  TRACE_PAD_CTRL0;
    FWK_RW  uint32_t  TRACE_PAD_CTRL1;
    FWK_RW  uint32_t  IOFPGA_TMIF_PAD_CTRL;
    FWK_RW  uint32_t  IOFPGA_TSIF_PAD_CTRL;
            uint8_t   RESERVED21[0x260 - 0x210];

    /* EMA Registers */
    FWK_RW  uint32_t  CPU0_FCISRAMSPUHD_EMA_CTRL;
    FWK_RW  uint32_t  CPU0_CORINTH_FCIRFSPHD_HS_EMA_CTRL;
            uint32_t  RESERVED22;
    FWK_RW  uint32_t  CPU0_FCIRFSPHD_EMA_CTRL;
    FWK_RW  uint32_t  CPU0_RFSPHDS_EMA_CTRL;
    FWK_RW  uint32_t  CPU0_FCIRF2PHS_EMA_CTRL;
    FWK_RW  uint32_t  CPU1_FCISRAMSPUHD_EMA_CTRL;
    FWK_RW  uint32_t  CPU1_CORINTH_FCIRFSPHD_HS_EMA_CTRL;
            uint32_t  RESERVED23;
    FWK_RW  uint32_t  CPU1_FCIRFSPHD_EMA_CTRL;
    FWK_RW  uint32_t  CPU1_RFSPHDS_EMA_CTRL;
    FWK_RW  uint32_t  CPU1_FCIRF2PHS_EMA_CTRL;
    FWK_RW  uint32_t  CPU0_CORINTH_FCIRF2PHS_EMA_CTRL;
    FWK_RW  uint32_t  CPU0_CORINTH_FCISRAMSPUHD_EMA_CTRL;
    FWK_RW  uint32_t  CPU0_CORINTH_FCIRFSPHD_ARES_EMA_CTRL;
    FWK_RW  uint32_t  CPU0_CORINTH_FCIRFSPHD_EMA_CTRL;
    FWK_RW  uint32_t  CPU0_CORINTH_RFSPHDS_EMA_CTRL;
    FWK_RW  uint32_t  CPU1_CORINTH_FCIRF2PHS_EMA_CTRL;
    FWK_RW  uint32_t  CPU1_CORINTH_FCISRAMSPUHD_EMA_CTRL;
    FWK_RW  uint32_t  CPU1_CORINTH_FCIRFSPHD_ARES_EMA_CTRL;
    FWK_RW  uint32_t  CPU1_CORINTH_FCIRFSPHD_EMA_CTRL;
    FWK_RW  uint32_t  CPU1_CORINTH_RFSPHDS_EMA_CTRL;
    FWK_RW  uint32_t  DMC_RFSPHDE_EMA_CTRL;
    FWK_RW  uint32_t  DEBUG_SRAMSPHDE_EMA_CTRL;
    FWK_RW  uint32_t  MSCP_RFSPHDE_EMA_CTRL;
    FWK_RW  uint32_t  MSCP_SRAMSPHDE_EMA_CTRL;
    FWK_RW  uint32_t  MSCP_ROM_EMA_CTRL;
    FWK_RW  uint32_t  MMU_RFSPHDE_EMA_CTRL;
    FWK_RW  uint32_t  BASE_SRAMSPHDE_EMA_CTRL;
    FWK_RW  uint32_t  BASE_ROM_EMA_CTRL;
            uint32_t  RESERVED24;
            uint32_t  RESERVED25;
    FWK_RW  uint32_t  GIC_RFSPHDE_EMA_CTRL;
    FWK_RW  uint32_t  PCIE_RFSPHDE_EMA_CTRL;
    FWK_RW  uint32_t  PCIE_RF2PHSC_EMA_CTRL;
    FWK_RW  uint32_t  CCIX_RFSPHDE_EMA_CTRL;
    FWK_RW  uint32_t  CCIX_RF2PHSC_EMA_CTRL;
    FWK_RW  uint32_t  POR_RF2PHSC_EMA_CTRL;
    FWK_RW  uint32_t  POR_FCISRAMSPUHD_FCM_EMA_CTRL;
    FWK_RW  uint32_t  POR_FCIRFSPHD_FCM_EMA_CTRL;
    FWK_RW  uint32_t  POR_FCIRFSPHD_ANANKE_EMA_CTRL;
            uint8_t   RESERVED26[0xE00 - 0x304];

    /* APB Magic Registers */
    FWK_RW  uint32_t  APB_CTRL_CLR;
            uint8_t   RESERVED27[0xFD0 - 0xE04];

    /* PID/CID Magic Registers */
    const struct scc_pcid_registers PCID;
};

#define SCC_PLATFORM_CTRL_MULTI_CHIP_MODE_POS 8
#define SCC_PLATFORM_CTRL_CHIPID_POS          0

#define SCC_PLATFORM_CTRL_MULTI_CHIP_MODE_MASK \
    (UINT32_C(0x1) << SCC_PLATFORM_CTRL_MULTI_CHIP_MODE_POS)
#define SCC_PLATFORM_CTRL_CHIPID_MASK \
    (UINT32_C(0x3F) << SCC_PLATFORM_CTRL_CHIPID_POS)

#define SCC_CCIX_PM_CTRL_PWR_REQ_POS     UINT32_C(1)
#define SCC_PCIE_PM_CTRL_PWR_REQ_POS     UINT32_C(1)

#define SCC_CCIX_PM_CTRL_PWR_ACK_MASK    UINT32_C(0x2)
#define SCC_PCIE_PM_CTRL_PWR_ACK_MASK    UINT32_C(0x2)

#define SCC_SYS_MAN_RESET_CCIX_POS       UINT32_C(11)
#define SCC_SYS_MAN_RESET_PCIE_POS       UINT32_C(10)

#endif /* N1SDP_SCC_REG_H */
