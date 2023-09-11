/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP Power Control registers
 */

#ifndef SCP_PWRCTRL_H
#define SCP_PWRCTRL_H

#include "scp_css_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief SCP Power Control register definitions
 */

// clang-format off
struct scp_power_control_reg {
            uint8_t   RESERVED0[0x10 - 0x0];
    FWK_RW  uint32_t  RESET_SYNDROME;
            uint8_t   RESERVED1[0x18 - 0x14];
    FWK_RW  uint32_t  LCP2SCP_INT_SOURCE_GROUP0;
    FWK_RW  uint32_t  LCP2SCP_INT_SOURCE_GROUP1;
    FWK_RW  uint32_t  LCP2SCP_INT_SOURCE_GROUP2;
    FWK_RW  uint32_t  LCP2SCP_INT_SOURCE_GROUP3;
    FWK_RW  uint32_t  LCP2SCP_INT_SOURCE_GROUP4;
    FWK_RW  uint32_t  LCP2SCP_INT_SOURCE_GROUP5;
    FWK_RW  uint32_t  LCP2SCP_INT_SOURCE_GROUP6;
    FWK_RW  uint32_t  LCP2SCP_INT_SOURCE_GROUP7;
            uint8_t   RESERVED2[0x200 - 0x38];
    FWK_R   uint32_t  SCP2LCP_MHU_PBX_INT_SOURCE_GROUP;
    FWK_R   uint32_t  SCP2LCP_MHU_PBX_INT_SOURCE_GROUP0;
    FWK_R   uint32_t  SCP2LCP_MHU_PBX_INT_SOURCE_GROUP1;
    FWK_R   uint32_t  SCP2LCP_MHU_PBX_INT_SOURCE_GROUP2;
    FWK_R   uint32_t  SCP2LCP_MHU_PBX_INT_SOURCE_GROUP3;
    FWK_R   uint32_t  SCP2LCP_MHU_PBX_INT_SOURCE_GROUP4;
    FWK_R   uint32_t  SCP2LCP_MHU_PBX_INT_SOURCE_GROUP5;
    FWK_R   uint32_t  SCP2LCP_MHU_PBX_INT_SOURCE_GROUP6;
    FWK_R   uint32_t  SCP2LCP_MHU_PBX_INT_SOURCE_GROUP7;
    FWK_R   uint32_t  LCP2SCP_MHU_MBX_INT_SOURCE_GROUP;
    FWK_R   uint32_t  LCP2SCP_MHU_MBX_INT_SOURCE_GROUP0;
    FWK_R   uint32_t  LCP2SCP_MHU_MBX_INT_SOURCE_GROUP1;
    FWK_R   uint32_t  LCP2SCP_MHU_MBX_INT_SOURCE_GROUP2;
    FWK_R   uint32_t  LCP2SCP_MHU_MBX_INT_SOURCE_GROUP3;
    FWK_R   uint32_t  LCP2SCP_MHU_MBX_INT_SOURCE_GROUP4;
    FWK_R   uint32_t  LCP2SCP_MHU_MBX_INT_SOURCE_GROUP5;
    FWK_R   uint32_t  LCP2SCP_MHU_MBX_INT_SOURCE_GROUP6;
    FWK_R   uint32_t  LCP2SCP_MHU_MBX_INT_SOURCE_GROUP7;
            uint8_t   RESERVED3[0x810 - 0x248];
    FWK_RW  uint32_t  CORECLK_CTRL;
    FWK_RW  uint32_t  CORECLK_DIV1;
            uint8_t   RESERVED4[0x820 - 0x818];
    FWK_RW  uint32_t  ACLK_CTRL;
    FWK_RW  uint32_t  ACLK_DIV1;
            uint8_t   RESERVED5[0x830 - 0x828];
    FWK_RW  uint32_t  GTSYNCCLK_CTRL;
    FWK_RW  uint32_t  GTSYNCCLK_DIV1;
            uint8_t   RESERVED6[0x840 - 0x838];
    FWK_RW  uint32_t  LCPCLK_CTRL;
    FWK_RW  uint32_t  LCPCLK_DIV1;
            uint8_t   RESERVED7[0xA00 - 0x848];
    FWK_R   uint32_t  CLKFORCE_STATUS;
    FWK_W   uint32_t  CLKFORCE_SET;
    FWK_W   uint32_t  CLKFORCE_CLEAR;
            uint8_t   RESERVED8[0xA50 - 0xA0C];
    FWK_R   uint32_t  CONS_MMUTCU_INT_STATUS;
    FWK_W   uint32_t  CONS_MMUTCU_INT_CLR;
    FWK_R   uint32_t  CONS_MMUTCU1_INT_STATUS;
    FWK_W   uint32_t  CONS_MMUTCU1_INT_CLR;
            uint8_t   RESERVED9[0xA68 - 0xA60];
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS0;
    FWK_W   uint32_t  CONS_MMUTBU_INT_CLR0;
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS1;
    FWK_W   uint32_t  CONS_MMUTBU_INT_CLR1;
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS2;
    FWK_W   uint32_t  CONS_MMUTBU_INT_CLR2;
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS3;
    FWK_W   uint32_t  CONS_MMUTBU_INT_CLR3;
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS4;
    FWK_W   uint32_t  CONS_MMUTBU_INT_CLR4;
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS5;
    FWK_W   uint32_t  CONS_MMUTBU_INT_CLR5;
            uint8_t   RESERVED10[0xB20 - 0xA98];
    FWK_R   uint32_t  CPU_PPU_INT_STATUS[8];
    FWK_R   uint32_t  CLUS_PPU_INT_STATUS[8];
    FWK_R   uint32_t  PROC_PEX_INT_STATUS[8];
    FWK_RW  uint32_t  CPU_PLL_LOCK_STATUS[8];
            uint8_t   RESERVED11[0xBC0 - 0xBA0];
    FWK_RW  uint32_t  CPU_PLL_UNLOCK_STATUS[7];
            uint8_t   RESERVED12[0xC10 - 0xBE0];
    FWK_W   uint32_t  SMCF_MGI_TRIGGER;
            uint8_t   RESERVED13[0xC20 - 0xC14];
    FWK_R   uint32_t  SRAMECC_ERRFR;
    FWK_R   uint32_t  SRAMECC_ERRFR_H;
    FWK_RW  uint32_t  SRAMECC_ERRCTRL;
    FWK_RW  uint32_t  SRAMECC_ERRCTRL_H;
    FWK_RW  uint32_t  SRAMECC_ERRSTATUS;
    FWK_RW  uint32_t  SRAMECC_ERRSTATUS_H;
    FWK_RW  uint32_t  SRAMECC_ERRADDR;
    FWK_RW  uint32_t  SRAMECC_ERRADDR_H;
            uint8_t   RESERVED14[0xC48 - 0xC40];
    FWK_RW  uint32_t  SRAMECC_ERRMISC1;
    FWK_RW  uint32_t  SRAMECC_ERRMISC1_H;
            uint8_t   RESERVED15[0xC60 - 0xC50];
    FWK_R   uint32_t  SYSNCI_PMU_CONS_INT_STATUS;
    FWK_R   uint32_t  SYSNCI_CONS_INT_STATUS;
    FWK_R   uint32_t  INTNCI_PMU_CONS_INT_STATUS;
    FWK_R   uint32_t  INTNCI_CONS_INT_STATUS;
    FWK_R   uint32_t  PERIPHNCI_PMU_CONS_INT_STATUS;
    FWK_R   uint32_t  PERIPHNCI_CONS_INT_STATUS;
            uint8_t   RESERVED16[0xFC0 - 0xC78];
    FWK_R   uint32_t  PWR_CTRL_CONFIG;
            uint8_t   RESERVED17[0xFD0 - 0xFC4];
    FWK_R   uint32_t  PERIPHERAL_ID4;
    FWK_R   uint32_t  PERIPHERAL_ID5;
    FWK_R   uint32_t  PERIPHERAL_ID6;
    FWK_R   uint32_t  PERIPHERAL_ID7;
    FWK_R   uint32_t  PERIPHERAL_ID0;
    FWK_R   uint32_t  PERIPHERAL_ID1;
    FWK_R   uint32_t  PERIPHERAL_ID2;
    FWK_R   uint32_t  PERIPHERAL_ID3;
    FWK_R   uint32_t  COMPONENT_ID0;
    FWK_R   uint32_t  COMPONENT_ID1;
    FWK_R   uint32_t  COMPONENT_ID2;
    FWK_R   uint32_t  COMPONENT_ID3;
};
// clang-format on

/* Pointer to SCP Power Control register block */
#define SCP_PWRCTRL_PTR ((struct scp_power_control_reg *)SCP_POWER_CONTROL_BASE)

#endif /* SCP_PWRCTRL_H */
