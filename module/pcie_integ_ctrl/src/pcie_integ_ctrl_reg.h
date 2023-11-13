/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Register definitions for PCIe integration control module.
 */

#ifndef PCIE_INTEG_CTRL_REG
#define PCIE_INTEG_CTRL_REG

#include <mod_pcid.h>

#include <fwk_macros.h>

#include <stdint.h>

struct pcie_ctrl_reg_set {
    FWK_RW uint32_t ECAM1_START_ADDR;
    FWK_RW uint32_t ECAM1_END_ADDR;
    FWK_RW uint32_t MMIOL_START_ADDR;
    FWK_RW uint32_t MMIOL_END_ADDR;
    FWK_RW uint32_t MMIOH_START_ADDR;
    FWK_RW uint32_t MMIOH_END_ADDR;
    FWK_RW uint32_t MMIOH2L_TR_START_ADDR;
    FWK_RW uint32_t MMIOH2L_TR_END_ADDR;
    FWK_RW uint32_t CFG_START_ADDR;
    FWK_RW uint32_t CFG_END_ADDR;
    FWK_RW uint32_t ECAM2_START_ADDR;
    FWK_RW uint32_t ECAM2_END_ADDR;
    FWK_RW uint32_t ECAM2_ADDR_CTRL;
};

struct pcie_ctrl_reg {
    struct pcie_ctrl_reg_set pcie_ctrl_x4_0;
    uint8_t RESERVED0[0x100 - 0x34];
    struct pcie_ctrl_reg_set pcie_ctrl_x4_1;
    uint8_t RESERVED1[0x200 - 0x134];
    struct pcie_ctrl_reg_set pcie_ctrl_x8;
    uint8_t RESERVED2[0x300 - 0x234];
    struct pcie_ctrl_reg_set pcie_ctrl_x16;
    uint8_t RESERVED3[0x400 - 0x334];
    uint32_t NCI_PMU_CONS_INT_STATUS;
    uint8_t RESERVED4[0xFD0 - 0x404];
    struct mod_pcid_registers pcid;
};

#define PCIE_INTEG_CTRL_REG_ADDR_POS             1
#define PCIE_INTEG_CTRL_REG_SEC_ACC_CTRL_DIS_POS 29
#define PCIE_INTEG_CTRL_REG_EN                   1

#define PCIE_INTEG_CTRL_REG_START_ADDR(addr) \
    (((addr) >> 20) << PCIE_INTEG_CTRL_REG_ADDR_POS)

#define PCIE_INTEG_CTRL_REG_START_ADDR_EN(addr, non_secure_access) \
    (PCIE_INTEG_CTRL_REG_START_ADDR(addr) | PCIE_INTEG_CTRL_REG_EN | \
     ((non_secure_access) << PCIE_INTEG_CTRL_REG_SEC_ACC_CTRL_DIS_POS))

#define PCIE_INTEG_CTRL_REG_END_ADDR(addr) ((addr) >> 20)

#endif /* PCIE_INTEG_CTRL_REG */
