/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the CMN-700 module.
 */

/* The use of "manager" may not be in sync with older versions of TRM */

#ifndef CMN700_H
#define CMN700_H

#include <mod_cmn700.h>

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

/* Max Node Counts */
#define MAX_HNF_COUNT 128
#define MAX_RND_COUNT 40
#define MAX_RNI_COUNT 40
#define MAX_RNF_COUNT 256

/* Maximum System Cache Group regions supported by CMN-700 */
#define MAX_SCG_COUNT 4
/* Maximum Non Hash Mem Group regions supported by CMN-700 */
#define MAX_NON_HASH_MEM_COUNT 60
/* Maximum Non Hash Mem Group regions in first group */
#define NON_HASH_MEM_REG_COUNT 24
/* Maximum Non Hash Mem Group regions in second group */
#define NON_HASH_MEM_REG_GRP2_COUNT 40

/* SAM Granularity of RN-SAM and HN-F SAM */
#define SAM_GRANULARITY (64 * FWK_MIB)

/* External nodes that require RN-SAM mapping during run-time */
struct external_rnsam_tuple {
    unsigned int node_id;
    struct cmn700_rnsam_reg *node;
};

/* Pair of CCG Request Agent (CCG_RA) register and its node-id */
struct ccg_ra_reg_tuple {
    unsigned int node_id;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;
};

/* Pair of CCG Request Agent (CCG_HA) register and its node-id */
struct ccg_ha_reg_tuple {
    unsigned int node_id;
    struct cmn700_ccg_ha_reg *ccg_ha_reg;
};

/* Pair of CCG Link Agent (CCLA) register and its node-id */
struct ccla_reg_tuple {
    unsigned int node_id;
    struct cmn700_ccla_reg *ccla_reg;
};

enum node_type {
    NODE_TYPE_INVALID   = 0x0,
    NODE_TYPE_DVM       = 0x1,
    NODE_TYPE_CFG       = 0x2,
    NODE_TYPE_DTC       = 0x3,
    NODE_TYPE_HN_I      = 0x4,
    NODE_TYPE_HN_F      = 0x5,
    NODE_TYPE_XP        = 0x6,
    NODE_TYPE_SBSX      = 0x7,
    NODE_TYPE_MPAM_S    = 0x8,
    NODE_TYPE_MPAM_NS   = 0x9,
    NODE_TYPE_RN_I      = 0xA,
    NODE_TYPE_RN_D      = 0xD,
    NODE_TYPE_RN_SAM    = 0xF,
    NODE_TYPE_HN_P      = 0x11,
    /* Coherent Multichip Link (CML) node types */
    NODE_TYPE_CML_BASE  = 0x100,
    NODE_TYPE_CXRA      = 0x100,
    NODE_TYPE_CXHA      = 0x101,
    NODE_TYPE_CXLA      = 0x102,
    NODE_TYPE_CCRA      = 0x103,
    NODE_TYPE_CCHA      = 0x104,
    NODE_TYPE_CCLA      = 0x105,
};

enum device_type {
    DEVICE_TYPE_RN_F_CHIB_ESAM  = 0x5, // 0b00101
    DEVICE_TYPE_CXHA            = 0x11, // 0b10001
    DEVICE_TYPE_CXRA            = 0x12, // 0b10010
    DEVICE_TYPE_CXRH            = 0x13, // 0b10011
    DEVICE_TYPE_CCG             = 0x1E, // 0b11110
    DEVICE_TYPE_RN_F_CHID_ESAM  = 0x15, // 0b10101
    DEVICE_TYPE_RN_F_CHIC_ESAM  = 0x17, // 0b10111
    DEVICE_TYPE_RN_F_CHIE_ESAM  = 0x19, // 0b11001
};

/* Common node header */
struct node_header {
    FWK_R uint64_t NODE_INFO;
          uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R uint64_t CHILD_INFO;
          uint8_t  RESERVED1[0x100 - 0x88];
    FWK_R uint64_t CHILD_POINTER[256];
};

enum sam_type {
    SAM_TYPE_NON_HASH_MEM_REGION,
    SAM_TYPE_SYS_CACHE_GRP_REGION,
};

enum sam_node_type {
    SAM_NODE_TYPE_HN_F = 0,
    SAM_NODE_TYPE_HN_I = 1,
    SAM_NODE_TYPE_CXRA = 2,
    SAM_NODE_TYPE_COUNT
};

enum sam_scg_index {
    SAM_SCG0 = 0,
    SAM_SCG1,
    SAM_SCG2,
    SAM_SCG3,
    SAM_SCG_COUNT
};

/*
 * Request Node System Address Map (RN-SAM) registers
 */
struct cmn700_rnsam_reg {
    FWK_R   uint64_t  NODE_INFO;
            uint8_t   RESERVED0[0x80 - 0x8];
    FWK_R   uint64_t  CHILD_INFO;
            uint8_t   RESERVED1[0x900 - 0x88];
    FWK_R   uint64_t  UNIT_INFO[2];
            uint8_t   RESERVED2[0xC00 - 0x910];
    FWK_RW  uint64_t  NON_HASH_MEM_REGION[NON_HASH_MEM_REG_COUNT];
    FWK_RW  uint64_t  NON_HASH_MEM_REGION_CFG2[NON_HASH_MEM_REG_COUNT];
    FWK_RW  uint64_t  NON_HASH_TGT_NODEID[16];
    FWK_RW  uint64_t  SYS_CACHE_GRP_REGION[4];
    FWK_RW  uint64_t  HASHED_TGT_GRP_CFG1_REGION[4];
            uint8_t   RESERVED5[0xEA0 - 0xE40];
    FWK_RW  uint64_t  SYS_CACHE_GRP_HN_COUNT;
            uint8_t   RESERVED6[0xEB0 - 0xEA8];
    FWK_RW  uint64_t  SYS_CACHE_GRP_SN_ATTR[2];
            uint8_t   RESERVED7[0xF00 - 0xEC0];
    FWK_RW  uint64_t  SYS_CACHE_GRP_HN_NODEID[16];
            uint8_t   RESERVED8[0x1000 - 0xF80];
    FWK_RW  uint64_t  SYS_CACHE_GRP_SN_NODEID[32];
    FWK_RW  uint64_t  STATUS;
            uint64_t  GIC_MEM_REGION;
            uint8_t   RESERVED9[0x1120 - 0x1110];
    FWK_RW  uint64_t  SYS_CACHE_GRP_CAL_MODE;
    FWK_RW  uint64_t  HASHED_TARGET_GRP_CAL_MODE[3];
    FWK_RW  uint64_t  SYS_CACHE_GRP_SN_SAM_CFG[4];
            uint8_t   RESERVED10[0x20C0 - 0x1160];
    FWK_RW  uint64_t  NON_HASH_MEM_REGION_GRP2[NON_HASH_MEM_REG_GRP2_COUNT];
            uint8_t   RESERVED11[0x24C0 - 0x2200];
    FWK_RW  uint64_t  NON_HASH_MEM_REGION_CFG2_GRP2[NON_HASH_MEM_REG_GRP2_COUNT];
            uint8_t   RESERVED12[0x3100 - 0x2600];
    FWK_RW  uint64_t  HASHED_TGT_GRP_CFG2_REGION[32];
            uint8_t   RESERVED13[0x3400 - 0x3200];
    FWK_RW  uint64_t  HASHED_TARGET_GRP_HASH_CNTL[32];
};

#define HNF_RN_CLUSTER_MAX    128
#define HNF_RN_PHYIDS_REG_MAX 4

/*
 * Fully Coherent Home Node (HN-F) registers
 */
struct cmn700_hnf_reg {
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
            uint8_t   RESERVED5[0x1C00 - 0xD48];
    FWK_RW  uint64_t  PPU_PWPR;
            uint8_t   RESERVED6[0x3C00 - 0x1C08];
    FWK_RW  uint64_t
        HNF_RN_CLUSTER_PHYSID[HNF_RN_CLUSTER_MAX][HNF_RN_PHYIDS_REG_MAX];
};

/*
 * CCG Gateway (CCG) protocol link control & status registers
 */
struct ccg_link_regs {
    FWK_RW uint64_t CCG_CCPRTCL_LINK_CTRL;
    FWK_R uint64_t CCG_CCPRTCL_LINK_STATUS;
};

/*
 * CCG Requesting Agent (RA) registers
 */
struct cmn700_ccg_ra_reg {
    FWK_R  uint64_t CCG_RA_NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CCG_RA_CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t CCG_RA_UNIT_INFO;
           uint8_t  RESERVED2[0x980 - 0x908];
    FWK_RW uint64_t CCG_RA_SEC_REG_GRP_OVERRIDE;
           uint8_t  RESERVED3[0xA00 - 0x988];
    FWK_RW uint64_t CCG_RA_CFG_CTRL;
    FWK_RW uint64_t CCG_RA_AUX_CTRL;
           uint8_t  RESERVED4[0xC00 - 0xA10];
    FWK_RW uint64_t CCG_RA_SAM_ADDR_REGION_REG[8];
           uint8_t  RESERVED5[0xD00 - 0xC40];
    FWK_RW uint64_t CCG_RA_AGENTID_TO_LINKID_VAL;
           uint8_t  RESERVED6[0xD10 - 0xD08];
    FWK_RW uint64_t CCG_RA_AGENTID_TO_LINKID_REG[8];
           uint8_t  RESERVED7[0xE00 - 0xD50];
    FWK_RW uint64_t CCG_RA_RNI_LDID_TO_EXP_RAID_REG[10];
           uint8_t  RESERVED8[0xF00 - 0xE50];
    FWK_RW uint64_t CCG_RA_RND_LDID_TO_EXP_RAID_REG[10];
           uint8_t  RESERVED9[0x1000 - 0xF50];
    FWK_RW uint64_t CCG_RA_RNF_LDID_TO_EXP_RAID_REG[128];
    FWK_RW uint64_t CCG_RA_RNF_LDID_TO_NODEID_REG[128];
    FWK_RW uint64_t CCG_RA_RNF_LDID_TO_OVRD_LDID_REG[128];
           struct ccg_link_regs LINK_REGS[3];
           uint8_t  RESERVED10[0x2000 - 0x1C30];
    FWK_RW uint64_t CCG_RA_PMU_EVENT_SEL;
};

/*
 * CCG Gateway (CCG) Home Agent (HA) registers
 */
struct cmn700_ccg_ha_reg {
    FWK_R  uint64_t CCG_HA_NODE_INFO;
    FWK_RW uint64_t CCG_HA_ID;
           uint8_t  RESERVED0[0x80 - 0x10];
    FWK_R  uint64_t CCG_HA_CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t CCG_HA_UNIT_INFO[2];
           uint8_t  RESERVED2[0x980 - 0x910];
    FWK_RW uint64_t CCG_HA_SEC_REG_GRP_OVERRIDE;
           uint8_t  RESERVED3[0xA00 - 0x988];
    FWK_RW uint64_t CCG_HA_CFG_CTRL;
    FWK_RW uint64_t CCG_HA_AUX_CTRL;
           uint8_t  RESERVED4[0xC00 - 0xA10];
    FWK_RW uint64_t CCG_HA_RNF_EXP_RAID_TO_LDID_REG[256];
           uint8_t  RESERVED5[0x1C00 - 0x1400];
           struct ccg_link_regs LINK_REGS[3];
           uint8_t  RESERVED6[0x1F00 - 0x1C30];
    FWK_RW uint64_t CCG_HA_AGENTID_TO_LINKID_REG[8];
           uint8_t  RESERVED7[0x1FF8 - 0x1F40];
    FWK_RW uint64_t CCG_HA_AGENTID_TO_LINKID_VAL;
    FWK_RW uint64_t CCG_HA_PMU_EVENT_SEL;
};

/*
 * CCG Gateway (CCG) Link Agent (LA) registers
 */
struct cmn700_ccla_reg {
    FWK_R  uint64_t CCLA_NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CCLA_CHILD_INFO;
           uint8_t  RESERVED1[0x910 - 0x88];
    FWK_R  uint64_t CCLA_UNIT_INFO;
           uint8_t  RESERVED2[0x988 - 0x918];
    FWK_RW uint64_t CCLA_SEC_REG_GRP_OVERRIDE;
           uint8_t  RESERVED3[0xB00 - 0x990];
    FWK_RW uint64_t CCLA_CFG_CTL;
    FWK_RW uint64_t CCLA_AUX_CTRL;
           uint8_t  RESERVED4[0xC00 - 0xB10];
    FWK_R  uint64_t CCLA_CCIX_PROP_CAPABILITIES;
    FWK_RW uint64_t CCLA_CXS_ATTR_CAPABILITIES;
           uint8_t  RESERVED5[0xD00 - 0xC10];
    FWK_RW uint64_t CCLA_PERMSG_PYLD_0_63;
    FWK_RW uint64_t CCLA_PERMSG_PYLD_64_127;
    FWK_RW uint64_t CCLA_PERMSG_PYLD_128_191;
    FWK_RW uint64_t CCLA_PERMSG_PYLD_192_255;
    FWK_RW uint64_t CCLA_PERMSG_CTL;
    FWK_RW uint64_t CCLA_ERR_AGENT_ID;
    FWK_RW uint64_t CCLA_AGENTID_TO_PORTID_REG[8];
    FWK_RW uint64_t CCLA_AGENTID_TO_PORTID_VAL;
           uint8_t  RESERVED6[0xE00 - 0xD88];
    FWK_RW uint64_t CCLA_PORTFWD_CTL;
    FWK_R  uint64_t CCLA_PORTFWD_STATUS;
    FWK_RW uint64_t CCLA_CXL_LINK_RX_CREDIT_CTL;
    FWK_R  uint64_t CCLA_CXL_LINK_RX_CREDIT_RETURN_STAT;
    FWK_R  uint64_t CCLA_CXL_LINK_TX_CREDIT_STAT;
    FWK_RW uint64_t CCLA_CXL_LINK_LAYER_DEFEATURE;
    FWK_RW uint64_t CCLA_ULL_CTL;
    FWK_R  uint64_t CCLA_ULL_STATUS;
    FWK_RW uint64_t CCLA_CXL_LL_ERRINJECT_CTL;
    FWK_R  uint64_t CCLA_CXL_LL_ERRINJECT_STAT;
           uint8_t  RESERVED7[0x2008 - 0xE40];
    FWK_RW uint64_t CCLA_PMU_EVENT_SEL;
           uint8_t  RESERVED8[0x3000 - 0x2010];
    FWK_R  uint64_t CCLA_ERRFR;
    FWK_RW uint64_t CCLA_ERRCTLR;
    FWK_W  uint64_t CCLA_ERRSTATUS;
    FWK_RW uint64_t CCLA_ERRADDR;
    FWK_RW uint64_t CCLA_ERRMISC;
           uint8_t  RESERVED9[0x3100 - 0x3028];
    FWK_R  uint64_t CCLA_ERRFR_NS;
    FWK_RW uint64_t CCLA_ERRCTLR_NS;
    FWK_W  uint64_t CCLA_ERRSTATUS_NS;
    FWK_RW uint64_t CCLA_ERRADDR_NS;
    FWK_RW uint64_t CCLA_ERRMISC_NS;
};

/*
 * Configuration manager registers
 */
struct cmn700_cfgm_reg {
    FWK_R   uint64_t  NODE_INFO;
    FWK_RW  uint64_t  PERIPH_ID[4];
    FWK_RW  uint64_t  COMPONENT_ID[2];
            uint8_t   RESERVED0[0x80 - 0x38];
    FWK_R   uint64_t  CHILD_INFO;
};

/*
 * Crosspoint (XP) registers
 */
struct cmn700_mxp_reg {
    FWK_R  uint64_t  NODE_INFO;
    FWK_R  uint64_t  PORT_CONNECT_INFO[6];
    FWK_R  uint64_t  PORT_CONNECT_INFO_EAST;
    FWK_R  uint64_t  PORT_CONNECT_INFO_NORTH;
           uint8_t   RESERVED0[0x80 - 0x28];
    FWK_R  uint64_t  CHILD_INFO;
           uint8_t   RESERVED1[0x100 - 0x88];
    FWK_R  uint64_t  CHILD_POINTER[32];
};

#define CMN700_NODE_INFO_TYPE           UINT64_C(0x000000000000FFFF)
#define CMN700_NODE_INFO_ID             UINT64_C(0x00000000FFFF0000)
#define CMN700_NODE_INFO_ID_POS         16
#define CMN700_NODE_INFO_LOGICAL_ID     UINT64_C(0x0000FFFF00000000)
#define CMN700_NODE_INFO_LOGICAL_ID_POS 32

#define CMN700_CHILD_INFO_COUNT     UINT64_C(0x000000000000FFFF)
#define CMN700_CHILD_POINTER_OFFSET UINT64_C(0x000000003FFFFFFF)
#define CMN700_CHILD_POINTER_EXT    UINT64_C(0x0000000080000000)

/* External child node */
#define CMN700_CHILD_POINTER_EXT_REGISTER_OFFSET  UINT64_C(0x00003FFF)
#define CMN700_CHILD_POINTER_EXT_NODE_POINTER     UINT64_C(0x3FFF0000)
#define CMN700_CHILD_POINTER_EXT_NODE_POINTER_POS 16

#define CMN700_RNSAM_UNIT_INFO_HTG_RCOMP_LSB_PARAM_MASK     UINT64_C(0x1F)
#define CMN700_RNSAM_UNIT_INFO_HTG_RANGE_COMP_EN_MASK       UINT64_C(0x8000000)
#define CMN700_RNSAM_UNIT_INFO_HTG_RANGE_COMP_EN_POS        27
#define CMN700_RNSAM_UNIT_INFO_NONHASH_RCOMP_LSB_PARAM_MASK UINT64_C(0x3E0)
#define CMN700_RNSAM_UNIT_INFO_NONHASH_RCOMP_LSB_PARAM_POS  5
#define CMN700_RNSAM_UNIT_INFO_NONHASH_RANGE_COMP_EN_MASK   UINT64_C(0x80000000)
#define CMN700_RNSAM_UNIT_INFO_NONHASH_RANGE_COMP_EN_POS    31

/* Used by NON_HASH_MEM_REGIONx and SYS_CACHE_GRP_REGIONx group registers */
#define CMN700_RNSAM_REGION_ENTRY_TYPE_POS                     2
#define CMN700_RNSAM_REGION_ENTRY_SIZE_POS                     56
#define CMN700_RNSAM_REGION_ENTRY_BASE_POS                     26
#define CMN700_RNSAM_REGION_ENTRY_BITS_WIDTH                   64
#define CMN700_RNSAM_REGION_ENTRY_VALID                        UINT64_C(0x01)
#define CMN700_RNSAM_REGION_ENTRIES_PER_GROUP                  1
#define CMN700_RNSAM_SYS_CACHE_GRP_SN_NODEID_ENTRIES_PER_GROUP 4
#define CMN700_RNSAM_SYS_CACHE_GRP_HN_CNT_POS(scg_grp)         (8 * (scg_grp))
#define CMN700_RNSAM_SCG_HNF_CAL_MODE_EN                       UINT64_C(0x01)
#define CMN700_RNSAM_SCG_HNF_CAL_MODE_SHIFT                    16
#define CMN700_RNSAM_STATUS_UNSTALL                            UINT64_C(0x02)
#define CMN700_RNSAM_STATUS_USE_DEFAULT_TARGET_ID              UINT64_C(0x01)
#define CMN700_RNSAM_STATUS_DEFAULT_NODEID_POS                 48
#define CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH      12
#define CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK            UINT64_C(0x0FFF)
#define CMN700_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP     4

/* Used by RNSAM Hierarchical hashing registers */
#define CMN700_RNSAM_HIERARCHICAL_HASH_EN_POS         2
#define CMN700_RNSAM_HIERARCHICAL_HASH_EN_MASK        UINT64_C(0x01)
#define CMN700_RNSAM_HIER_ENABLE_ADDRESS_STRIPING_POS 3
#define CMN700_RNSAM_HIER_HASH_CLUSTERS_POS           8
#define CMN700_RNSAM_HIER_HASH_NODES_POS              16
#define CMN700_RNSAM_SN_MODE_SYS_CACHE_POS(scg_grp)   ((4 + ((scg_grp)*16)) % 64)
#define CMN700_RNSAM_TOP_ADDRESS_BIT0_POS(scg_grp)    ((0 + ((scg_grp)*24)) % 64)
#define CMN700_RNSAM_TOP_ADDRESS_BIT1_POS(scg_grp)    ((8 + ((scg_grp)*24)) % 64)
#define CMN700_RNSAM_TOP_ADDRESS_BIT2_POS(scg_grp)    ((16 + ((scg_grp)*24)) % 64)

#define CMN700_RNSAM_SYS_CACHE_GRP_SN_ATTR_ENTRIES_PER_GRP    4
#define CMN700_RNSAM_SYS_CACHE_GRP_SN_SAM_CFG_ENTRIES_PER_GRP 2

#define CMN700_HNF_UNIT_INFO_HNSAM_RCOMP_EN_MASK 0x10000000
#define CMN700_HNF_UNIT_INFO_HNSAM_RCOMP_EN_POS  28
#define CMN700_HNF_SAM_MEMREGION_SIZE_POS        12
#define CMN700_HNF_SAM_MEMREGION_BASE_POS        26
#define CMN700_HNF_SAM_MEMREGION_VALID           UINT64_C(0x8000000000000000)

/* Used by HN-F SAM_CONTROL register */
#define CMN700_HNF_SAM_CONTROL_SN_MODE_POS(sn_mode)   (36 + sn_mode - 1)
#define CMN700_HNF_SAM_CONTROL_TOP_ADDR_BIT0_POS      40
#define CMN700_HNF_SAM_CONTROL_TOP_ADDR_BIT1_POS      48
#define CMN700_HNF_SAM_CONTROL_TOP_ADDR_BIT2_POS      56
#define CMN700_HNF_SAM_CONTROL_SN_NODE_ID_POS(sn_idx) (sn_idx * 12)

#define CMN700_HNF_CACHE_GROUP_ENTRIES_MAX       128
#define CMN700_HNF_CACHE_GROUP_ENTRIES_PER_GROUP 4
#define CMN700_HNF_CACHE_GROUP_ENTRY_BITS_WIDTH  12

#define CMN700_PPU_PWPR_POLICY_OFF      UINT64_C(0x0000000000000000)
#define CMN700_PPU_PWPR_POLICY_MEM_RET  UINT64_C(0x0000000000000002)
#define CMN700_PPU_PWPR_POLICY_FUNC_RET UINT64_C(0x0000000000000007)
#define CMN700_PPU_PWPR_POLICY_ON       UINT64_C(0x0000000000000008)
#define CMN700_PPU_PWPR_OPMODE_NOSFSLC  UINT64_C(0x0000000000000000)
#define CMN700_PPU_PWPR_OPMODE_SFONLY   UINT64_C(0x0000000000000010)
#define CMN700_PPU_PWPR_OPMODE_HAM      UINT64_C(0x0000000000000020)
#define CMN700_PPU_PWPR_OPMODE_FAM      UINT64_C(0x0000000000000030)
#define CMN700_PPU_PWPR_DYN_EN          UINT64_C(0x0000000000000100)

/* Mesh and Node ID mapping */
#define CMN700_MESH_X_MAX 12
#define CMN700_MESH_Y_MAX 12

#define CMN700_NODE_ID_PORT_POS  2
#define CMN700_NODE_ID_PORT_MASK 0x1
#define CMN700_NODE_ID_Y_POS     3

/* For XP with 3 or 4 ports */
#define CMN700_MULTI_PORTS_NODE_ID_PORT_POS  1
#define CMN700_MULTI_PORTS_NODE_ID_PORT_MASK 0x3

#define CMN700_MXP_NODE_INFO_NUM_DEVICE_PORT_MASK UINT64_C(0xF000000000000)
#define CMN700_MXP_NODE_INFO_NUM_DEVICE_PORT_POS  48

#define CMN700_MXP_PORT_CONNECT_INFO_CAL_CONNECTED_MASK UINT64_C(0x80)
#define CMN700_MXP_PORT_CONNECT_INFO_CAL_CONNECTED_POS  7
#define CMN700_MXP_PORT_CONNECT_INFO_DEVICE_TYPE_MASK   UINT64_C(0x1F)

#define CMN700_ROOT_NODE_OFFSET_PORT_POS     16
#define CMN700_ROOT_NODE_4_BIT_ENCODING_MASK 0x30
#define CMN700_ROOT_NODE_OFFSET_Y_POS        22

/* Peripheral ID Revision Numbers */
#define CMN700_PERIPH_ID_2_REV_R0_P0 (0x00)
#define CMN700_PERIPH_ID_2_REV_R1_P0 (0x01)
#define CMN700_PERIPH_ID_2_REV_R1_P1 (0x02)
#define CMN700_PERIPH_ID_2_REV_R2_P0 (0x03)
#define CMN700_PERIPH_ID_UNKNOWN_REV (CMN700_PERIPH_ID_2_REV_R2_P0 + 1)

/* Peripheral ID Revision Numbers */
#define CMN700_PERIPH_ID_2_MASK    UINT64_C(0xFF)
#define CMN700_PERIPH_ID_2_REV_POS 4

/*
 * Retrieve the number of device ports connected to the cross point
 *
 * \param xp_base Pointer to the cross point (xp)
 *      \pre The xp pointer must be valid
 *
 * \return Number of device ports connected to the cross point
 */
unsigned int get_node_device_port_count(void *xp_base);

/*
 * Retrieve the number of child nodes of a given node
 *
 * \param node_base Pointer to the node descriptor
 *      \pre The node pointer must be valid
 *
 * \return Number of child nodes
 */
unsigned int get_node_child_count(void *node_base);

/*
 * Retrieve node type identifier
 *
 * \param node_base Pointer to the node descriptor
 *      \pre The node pointer must be valid
 *
 * \return Node's type identifier
 */
enum node_type get_node_type(void *node_base);

/*
 * Retrieve the physical identifier of a node from its hardware node descriptor.
 * This identifier encodes the node's position in the mesh.
 *
 * Note: Multiple node descriptors can share the same identifier if they are
 * related to the same device node in the mesh.
 *
 * \param node_base Pointer to the node descriptor
 *      \pre The node pointer must be valid
 *
 * \return Node's physical identifier
 */
unsigned int get_node_id(void *node_base);

/*
 * Retrieve the logical identifier of a node from its hardware node descriptor.
 * This is an unique identifier (index) among nodes of the same type in the
 * system.
 *
 * \param node_base Pointer to the node base address
 *      \pre The node pointer must be valid
 *
 * \return An integer representing the node's logical identifier
 */
unsigned int get_node_logical_id(void *node_base);

/*
 * Retrieve a child node given a node and child index
 *
 * \param node_base Pointer to the node descriptor
 *      \pre The node pointer must be valid
 * \param child_index Child index
 *      \pre The child index must be valid
 *
 * \return Pointer to the child's node descriptor
 */
void *get_child_node(uintptr_t base, void *node_base, unsigned int child_index);

/*
 * Retrieve the physical identifier of a node using its child pointer in the
 * parent's node hardware descriptor
 *
 * This function is used to extract a node's identifier without accessing the
 * node descriptor. This is specially useful for external nodes that are in an
 * unavailable power or clock domain.
 *
 * \param node_base Pointer to the parent node descriptor
 *      \pre The node pointer must be valid
 * \param child_index Child index
 *      \pre The child index must be valid
 *
 * \return Physical child node identifier
 */
unsigned int get_child_node_id(void *node_base, unsigned int child_index);

/*
 * Retrieve the revision name of CMN-700.
 *
 * \param root Pointer to the CMN-700 configuration master register base.
 *
 * \return Pointer to the CMN-700 revision name string.
 */
const char *get_cmn700_revision_name(struct cmn700_cfgm_reg *root);

/*
 * Verify if a child node (given a parent node base and child index) is an
 * external node from the CMN-700 instance point of view.
 *
 * \param node_base Pointer to the parent node descriptor
 *      \pre The node pointer must be valid
 * \param child_index Child index
 *      \pre The child index must be valid
 *
 * \retval true if the node is external
 * \retval false if the node is internal
 */
bool is_child_external(void *node_base, unsigned int child_index);

/*
 * Returns the port number from the child node id.
 *
 * \param child_node_id Child node id calculated from the child pointer.
 * \param xp_port_cnt Number of ports in the XP.
 *
 * \retval port number
 */
unsigned int get_port_number(
    unsigned int child_node_id,
    unsigned int xp_port_cnt);

/*
 * Returns the device type from the MXP's port connect info register.
 *
 * \param mxp_base Pointer to the cross point node descriptor
 *      \pre The cross point node pointer must be valid
 * \param port Port number
 *      \pre The port number should be either 0, 1 or 2.
 *
 * \retval device type (por_mxp_por_mxp_device_port_connect_info_p[port] & 0x1F)
 */
unsigned int get_device_type(void *mxp_base, int port);

/*
 * Verify if the MXP port has CAL connected to it.
 *
 * \param mxp_base Pointer to the cross point node descriptor
 *      \pre The cross point node pointer must be valid
 * \param port Port number
 *      \pre The port number should be either 0 or 1.
 *
 * \retval true if CAL is connected to \param port
 * \retval false if CAL is non connected to \param port
 */
bool is_cal_connected(void *mxp_base, bool port);

/*
 * Verify if the device type connected to the MXP's port is of one of the RN-F
 * type.
 *
 * \param mxp_base Pointer to the cross point node descriptor
 *      \pre The cross point node pointer must be valid
 * \param port Port number
 *      \pre The port number should be either 0 or 1.
 *
 * \retval true if the device connected to \param port is one of the RN-F types
 * \retval false if the device connected to \param port not an RN-F type
 */
bool is_device_type_rnf(void *mxp_base, bool port);

/*
 * Returns if the rnsam nonhash memory region programming requires start and end
 * address programming
 *
 * \param rnsam_reg Pointer to the RNSAM register
 *      \pre RNSAM register pointer must be valid
 *
 * \retval true if rnsam non-hashed memory region requires start and end address
 * programing
 * \retval false if rnsam non-hashed memory region requires start and region
 * size programming
 */
bool get_rnsam_nonhash_range_comp_en_mode(void *rnsam_reg);

/*
 * Returns if the rnsam hashed target memory region programming requires start
 * and end address programming
 *
 * \param rnsam_reg Pointer to the RNSAM register
 *      \pre RNSAM register pointer must be valid
 *
 * \retval true if rnsam hashed target memory region requires start and end
 * address programing
 * \retval false if rnsam hashed target memory region requires start and region
 * size programming
 */
bool get_rnsam_htg_range_comp_en_mode(void *rnsam_reg);

/*
 * Returns if the hnsam memory region programming requires start and end address
 * programming
 *
 * \param hnf_reg Pointer to the HN-F register
 *      \pre HN-F register pointer must be valid
 *
 * \retval true if hnsam memory region requires start and end address programing
 * \retval false if hnsam memory region requires start and region size
 * programming
 */
bool get_hnsam_range_comp_en_mode(void *hnf_reg);

/*
 * Convert a memory region size into a size format used by the CMN-700
 * registers. The format is the binary logarithm of the memory region size
 * represented as blocks multiple of the CMN-700's granularity:
 * n =  log2(size / SAM_GRANULARITY)
 *
 * \param size Memory region size to be converted
 *      \pre size must be a multiple of SAM_GRANULARITY
 *
 * \return log2(size / SAM_GRANULARITY)
 */
uint64_t sam_encode_region_size(uint64_t size);

/*
 * \brief Check's alignment of a region's base address and size.
 *
 * \param rnsam rnsam node
 * \param mmap Region memory map information
 * \param sam_type Type of the region register to program (NON-HASH or
 * SYS-CACHE)
 *
 * \return true if region is aligned
 * \return false if region is not aligned
 */
bool is_region_aligned(
    struct cmn700_rnsam_reg *rnsam,
    struct mod_cmn700_mem_region_map *mmap,
    enum sam_type sam_type);

/*
 * \brief Checks if a non-hashed region is already mapped by comparing the
 * base address and node id with the values programmed in the registers
 *
 * \param rnsam rnsam node
 * \param region_io_count Number of region mapped
 * \param mmap Region memory map information
 * \param region_index Placeholder to put region index where the region is
 *                     mapped
 *
 * \return true if region is found
 * \return false if region is not found
 */
bool is_non_hash_region_mapped(
    struct cmn700_rnsam_reg *rnsam,
    uint32_t region_io_count,
    struct mod_cmn700_mem_region_map *mmap,
    uint32_t *region_index);

/*
 * Configure a NON-HASH or SYS-CACHE memory region
 *
 * \param reg Pointer to the RNSAM register
 * \param region_idx Index of the memory region
 * \param base Region base address
 * \param size Region size
 * \param node_type Type of the target node
 * \param sam_type Type of the region register to program (NON-HASH or
 * SYS-CACHE)
 *
 * \return None
 */
void configure_region(
    void *rnsam_reg,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size,
    enum sam_node_type node_type,
    enum sam_type sam_type);

/*
 * Retrieve the node type name
 *
 * \param node_type Node type
 *
 * \return Pointer to the node type name string
 */
const char *get_node_type_name(enum node_type node_type);

/*
 * Retrieve the node's position in the mesh along the X-axis
 *
 * \param node_base Pointer to the node descriptor
 *
 * \return Zero-indexed position along the X-axis
 */
unsigned int get_node_pos_x(void *node_base);

/*
 * Retrieve the node's position in the mesh along the Y-axis
 *
 * \param node_base Pointer to the node descriptor
 *
 * \return Zero-indexed position along the Y-axis
 */
unsigned int get_node_pos_y(void *node_base);

/*
 * Set encoding and masking bits based on the mesh size. These are used while
 * calculating the x and y pos based on the node_id.
 *
 * \param config Config data of this module.
 *
 * \return None
 */
void set_encoding_and_masking_bits(const struct mod_cmn700_config *config);

#endif /* CMN700_H */
