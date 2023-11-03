/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Register definitions for the CMN-Cyprus module.
 */

#ifndef CMN_CYPRUS_REG_INTERNAL_H
#define CMN_CYPRUS_REG_INTERNAL_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * Global Configuration Completer register.
 */
// clang-format off
struct cmn_cyprus_cfgm_reg {
    FWK_R   uint64_t  NODE_INFO;
    FWK_RW  uint64_t  PERIPH_ID[4];
    FWK_RW  uint64_t  COMPONENT_ID[2];
            uint8_t   RESERVED0[0x80 - 0x38];
    FWK_R   uint64_t  CHILD_INFO;
            uint8_t   RESERVED1[0x100 - 0x88];
    FWK_R   uint64_t  CHILD_POINTER[256];
};

/*!
 * Node Configuration register.
 */
struct cmn_cyprus_node_cfg_reg {
    FWK_R   uint64_t  NODE_INFO;
            uint8_t   RESERVED0[0x80 - 0x8];
    FWK_R   uint64_t  CHILD_INFO;
            uint8_t   RESERVED1[0x100 - 0x88];
};

/*!
 * Mesh Crosspoint (MXP) registers.
 */
struct cmn_cyprus_mxp_reg {
    FWK_R   uint64_t  NODE_INFO;
    FWK_R   uint64_t  PORT_CONNECT_INFO[6];
    FWK_R   uint64_t  PORT_CONNECT_INFO_EAST;
    FWK_R   uint64_t  PORT_CONNECT_INFO_NORTH;
            uint8_t   RESERVED0[0x80 - 0x48];
    FWK_R   uint64_t  CHILD_INFO;
            uint8_t   RESERVED1[0x100 - 0x88];
    FWK_R   uint64_t  CHILD_POINTER[32];
            uint8_t   RESERVED2[0xA70 - 0x200];
    FWK_RW  uint64_t  PORT_DISABLE;
};

#define HNS_RN_CLUSTER_MAX    128
#define HNS_RN_PHYIDS_REG_MAX 4

/*!
 * Fully Coherent Home Node (HN-S) registers.
 */
struct cmn_cyprus_hns_reg {
    FWK_R   uint64_t  NODE_INFO;
            uint8_t   RESERVED0[0x80 - 0x8];
    FWK_R   uint64_t  CHILD_INFO;
            uint8_t   RESERVED1[0x900 - 0x88];
    FWK_R   uint64_t  UNIT_INFO[2];
            uint8_t   RESERVED2[0xD00 - 0x910];
    FWK_RW  uint64_t  SAM_CONTROL;
    FWK_RW  uint64_t  SAM_MEMREGION[2];
            uint8_t   RESERVED3[0xD28 - 0xD18];
    FWK_RW  uint64_t  SAM_CONTROL_2;
            uint8_t   RESERVED4[0xD38 - 0xD30];
    FWK_RW  uint64_t  SAM_MEMREGION_END_ADDR[2];
            uint8_t   RESERVED5[0xFB0 - 0xD48];
    FWK_RW  uint64_t  CML_PORT_AGGR_GRP_REG[2];
            uint8_t   RESERVED6[0xFD0 - 0xFC0];
    FWK_RW  uint64_t  CML_PORT_AGGR_CTRL_REG;
            uint8_t   RESERVED7[0x1900 - 0xFD8];
    FWK_RW  uint64_t  PPU_PWPR;
            uint8_t   RESERVED8[0x3C00 - 0x1908];
    FWK_RW  uint64_t
        HNS_RN_CLUSTER_PHYSID[HNS_RN_CLUSTER_MAX][HNS_RN_PHYIDS_REG_MAX];
            uint8_t   RESERVED9[0x7000 - 0x4C00];
    FWK_RW  uint64_t  LCN_HASHED_TGT_GRP_CFG1_REGION[32];
    FWK_RW  uint64_t  LCN_HASHED_TGT_GRP_CFG2_REGION[32];
    FWK_RW  uint64_t  LCN_HASHED_TGT_GRP_SEC_CFG1_REGION[32];
    FWK_RW  uint64_t  LCN_HASHED_TGT_GRP_SEC_CFG2_REGION[32];
    FWK_RW  uint64_t  LCN_HASHED_TARGET_GRP_HASH_CNTL_REG[32];
    FWK_RW  uint64_t  LCN_HASHED_TARGET_GROUP_HN_COUNT_REG[4];
    FWK_RW  uint64_t  LCN_HASHED_TARGET_GRP_CAL_MODE_REG[8];
    FWK_RW  uint64_t  LCN_HASHED_TARGET_GRP_HNF_CPA_EN_REG[2];
            uint8_t   RESERVED10[0x7580 - 0x7570];
    FWK_RW  uint64_t  LCN_HASHED_TARGET_GRP_CPAG_PERHNF_REG[16];
};

/* Non-hashed memory region register count in first group */
#define RNSAM_NON_HASH_REG_COUNT 24
/* Non-hashed memory region register count in second group */
#define RNSAM_NON_HASH_REG_GRP2_COUNT 40
/* Hashed memory region register count in first group */
#define RNSAM_HTG_REG_COUNT 4
/* Hashed memory region register count in second group */
#define RNSAM_HTG_REG_GRP2_COUNT 4
/* SCG HN Node ID register count */
#define RNSAM_SCG_HN_NODEID_REG_COUNT 32

/*!
 * Request Node System Address Map (RNSAM) registers.
 */
struct cmn_cyprus_rnsam_reg {
    FWK_R   uint64_t  NODE_INFO;
            uint8_t   RESERVED0[0x80 - 0x8];
    FWK_R   uint64_t  CHILD_INFO;
            uint8_t   RESERVED1[0x900 - 0x88];
    FWK_R   uint64_t  UNIT_INFO[2];
            uint8_t   RESERVED2[0xC00 - 0x910];
    FWK_RW  uint64_t  NON_HASH_MEM_REGION[RNSAM_NON_HASH_REG_COUNT];
    FWK_RW  uint64_t  NON_HASH_MEM_REGION_CFG2[RNSAM_NON_HASH_REG_COUNT];
    FWK_RW  uint64_t  NON_HASH_TGT_NODEID[16];
    FWK_RW  uint64_t  SYS_CACHE_GRP_REGION[RNSAM_HTG_REG_COUNT];
    FWK_RW  uint64_t  HASHED_TGT_GRP_CFG1_REGION[RNSAM_HTG_REG_GRP2_COUNT];
    FWK_RW  uint64_t  SYS_CACHE_GRP_SECONDARY_REGION[4];
            uint8_t   RESERVED5[0xEA0 - 0xE60];
    FWK_RW  uint64_t  SYS_CACHE_GROUP_HN_COUNT;
            uint8_t   RESERVED6[0xEB0 - 0xEA8];
    FWK_RW  uint64_t  SYS_CACHE_GRP_SN_ATTR[2];
            uint8_t   RESERVED7[0xF00 - 0xEC0];
    FWK_RW  uint64_t  SYS_CACHE_GRP_HN_NODEID[RNSAM_SCG_HN_NODEID_REG_COUNT];
    FWK_RW  uint64_t  SYS_CACHE_GRP_SN_NODEID[32];
    FWK_RW  uint64_t  STATUS;
    FWK_RW  uint64_t  GIC_MEM_REGION;
            uint8_t   RESERVED8[0x1120 - 0x1110];
    FWK_RW  uint64_t  SYS_CACHE_GRP_CAL_MODE;
    FWK_RW  uint64_t  HASHED_TARGET_GRP_CAL_MODE[3];
    FWK_RW  uint64_t  SYS_CACHE_GRP_SN_SAM_CFG[8];
            uint8_t   RESERVED9[0x11A0 - 0x1180];
    FWK_RW  uint64_t  CML_PORT_AGGR_MODE_CTRL_REG[4];
            uint8_t   RESERVED10[0x11F0 - 0x11C0];
    FWK_RW  uint64_t  CML_PORT_AGGR_GRP_REG[3];
    FWK_RW  uint64_t  CML_PORT_AGGR_CTRL_REG[16];
            uint8_t   RESERVED11[0x20C0 - 0x1288];
    FWK_RW  uint64_t  NON_HASH_MEM_REGION_GRP2[RNSAM_NON_HASH_REG_GRP2_COUNT];
            uint8_t   RESERVED12[0x24C0 - 0x2200];
    FWK_RW  uint64_t
        NON_HASH_MEM_REGION_CFG2_GRP2[RNSAM_NON_HASH_REG_GRP2_COUNT];
            uint8_t   RESERVED13[0x2B00 - 0x2600];
    FWK_RW  uint64_t  CML_CPAG_BASE_INDX_GRP_REG[8];
            uint8_t   RESERVED14[0x3100 - 0x2B40];
    FWK_RW  uint64_t  HASHED_TGT_GRP_CFG2_REGION[32];
            uint8_t   RESERVED15[0x3400 - 0x3200];
    FWK_RW  uint64_t  HASHED_TARGET_GRP_HASH_CNTL[32];
};
// clang-format on

#endif /* CMN_CYPRUS_REG_INTERNAL_H */
