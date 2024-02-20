/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
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
/* CML Port Aggregation Group register count */
#define HNS_CPA_GRP_REG_COUNT 2

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
    FWK_RW  uint64_t  CML_PORT_AGGR_GRP_REG[HNS_CPA_GRP_REG_COUNT];
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

/*
 * CCG Gateway (CCG) protocol link control & status registers.
 */
struct ccg_link_regs {
    FWK_RW  uint64_t  CCPRTCL_LINK_CTL;
    FWK_R   uint64_t  CCPRTCL_LINK_STATUS;
};

#define CCG_RA_SAM_ADDR_REG_COUNT 8
#define CCG_RA_RNI_LDID_TO_EXP_RAID_REG_COUNT 32
#define CCG_RA_RND_LDID_TO_EXP_RAID_REG_COUNT 32
#define CCG_RA_RNF_LDID_TO_EXP_RAID_REG_COUNT 128

/*
 * CCG Requesting Agent (RA) registers
 */
struct cmn_cyprus_ccg_ra_reg {
    FWK_R   uint64_t  CCG_RA_NODE_INFO;
            uint8_t   RESERVED0[0x80 - 0x8];
    FWK_R   uint64_t  CCG_RA_CHILD_INFO;
            uint8_t   RESERVED1[0x900 - 0x88];
    FWK_R   uint64_t  CCG_RA_UNIT_INFO;
            uint8_t   RESERVED2[0x980 - 0x908];
    FWK_RW  uint64_t  CCG_RA_SCR;
            uint8_t   RESERVED3[0xA00 - 0x988];
    FWK_RW  uint64_t  CCG_RA_CFG_CTL;
    FWK_RW  uint64_t  CCG_RA_AUX_CTL;
            uint8_t   RESERVED4[0xC00 - 0xA10];
    FWK_RW  uint64_t  CCG_RA_SAM_ADDR_REGION_REG[CCG_RA_SAM_ADDR_REG_COUNT];
            uint8_t   RESERVED5[0xD00 - 0xC40];
    FWK_RW  uint64_t  CCG_RA_AGENTID_TO_LINKID_VAL;
            uint8_t   RESERVED6[0xD10 - 0xD08];
    FWK_RW  uint64_t  CCG_RA_AGENTID_TO_LINKID_REG[8];
            uint8_t   RESERVED7[0xE00 - 0xD50];
    FWK_RW  uint64_t
        CCG_RA_RNI_LDID_TO_EXP_RAID_REG[CCG_RA_RNI_LDID_TO_EXP_RAID_REG_COUNT];
    FWK_RW  uint64_t
        CCG_RA_RND_LDID_TO_EXP_RAID_REG[CCG_RA_RND_LDID_TO_EXP_RAID_REG_COUNT];
    FWK_RW  uint64_t
        CCG_RA_RNF_LDID_TO_EXP_RAID_REG[CCG_RA_RNF_LDID_TO_EXP_RAID_REG_COUNT];
    FWK_RW  uint64_t  CCG_RA_HA_LDID_TO_EXP_RAID_REG[64];
    FWK_RW  uint64_t  CCG_RA_HNS_LDID_TO_EXP_RAID_REG;
            uint8_t   RESERVED8[0x1680 - 0x1608];
    FWK_RW  uint64_t  CCG_RA_RNF_LDID_TO_OVRD_LDID_REG[128];
    FWK_RW  uint64_t  CCG_RA_CML_PORT_AGGR_GRP_ADDR_MASK[32];
    FWK_RW  uint64_t  CCG_RA_CML_PORT_AGGR_GRP_REG[13];
    FWK_RW  uint64_t  CCG_RA_CML_PORT_AGGR_CTRL_REG[7];
    FWK_RW  uint64_t  CCG_RA_HA_LDID_TO_OVRD_LDID_REG[64];
            uint8_t   RESERVED9[0x1E38 - 0x1E20];
    FWK_RW  uint64_t  CCG_RA_HA_LDID_TO_EXP_RAID_CPAGEN_REG[64];
            uint8_t   RESERVED10[0x4000 - 0x2038];
            struct    ccg_link_regs LINK_REGS[3];
            uint8_t   RESERVED11[0xD900 - 0x4030];
    FWK_RW  uint64_t  CCG_RA_PMU_EVENT_SEL;
};

#define CCG_HA_EXP_RAID_TO_LDID_REG_COUNT 256

/*
 * CCG Gateway (CCG) Home Agent (HA) registers
 */
struct cmn_cyprus_ccg_ha_reg {
    FWK_R   uint64_t  CCG_HA_NODE_INFO;
    FWK_RW  uint64_t  CCG_HA_ID;
            uint8_t   RESERVED0[0x80 - 0x10];
    FWK_R   uint64_t  CCG_HA_CHILD_INFO;
            uint8_t   RESERVED1[0x900 - 0x88];
    FWK_R   uint64_t  CCG_HA_UNIT_INFO[3];
            uint8_t   RESERVED2[0x980 - 0x918];
    FWK_RW  uint64_t  CCG_HA_SCR;
    FWK_RW  uint64_t  CCG_HA_RCR;
            uint8_t   RESERVED3[0xA00 - 0x990];
    FWK_RW  uint64_t  CCG_HA_CFG_CTL;
    FWK_RW  uint64_t  CCG_HA_AUX_CTL;
    FWK_R   uint64_t  CCG_HA_MPAM_CONTROL_LINK[3];
            uint8_t   RESERVED4[0xC00 - 0xA28];
    FWK_RW  uint64_t
        CCG_HA_RNF_EXP_RAID_TO_LDID_REG[CCG_HA_EXP_RAID_TO_LDID_REG_COUNT];
            uint8_t   RESERVED5[0x1900 - 0x1400];
            struct    ccg_link_regs LINK_REGS[3];
            uint8_t   RESERVED6[0x1C00 - 0x1930];
    FWK_RW  uint64_t  CCG_HA_AGENTID_TO_LINKID_REG[8];
            uint8_t   RESERVED7[0x1CF8 - 0x1C40];
    FWK_RW  uint64_t  CCG_HA_AGENTID_TO_LINKID_VAL;
            uint8_t   RESERVED8[0xD900 - 0x1D00];
    FWK_RW  uint64_t  CCG_HA_PMU_EVENT_SEL;
};

/*
 * CCG Gateway (CCG) Link Agent (LA) registers
 */
struct cmn_cyprus_ccla_reg {
    FWK_R   uint64_t  CCLA_NODE_INFO;
            uint8_t   RESERVED0[0x80 - 0x8];
    FWK_R   uint64_t  CCLA_CHILD_INFO;
            uint8_t   RESERVED1[0x910 - 0x88];
    FWK_R   uint64_t  CCLA_UNIT_INFO;
            uint8_t   RESERVED2[0x980 - 0x918];
    FWK_RW  uint64_t  CCLA_SCR;
    FWK_RW  uint64_t  CCLA_RCR;
            uint8_t   RESERVED3[0xB00 - 0x990];
    FWK_RW  uint64_t  CCLA_CFG_CTL;
    FWK_RW  uint64_t  CCLA_AUX_CTL;
            uint8_t   RESERVED4[0xC00 - 0xB10];
    FWK_R   uint64_t  CCLA_CCIX_PROP_CAPABILITIES;
    FWK_R   uint64_t  CCLA_CXS_ATTR_CAPABILITIES;
            uint8_t   RESERVED5[0xD28 - 0xC10];
    FWK_RW  uint64_t  CCLA_ERR_AGENT_ID;
    FWK_RW  uint64_t  CCLA_AGENTID_TO_PORTID_REG[8];
    FWK_RW  uint64_t  CCLA_AGENTID_TO_PORTID_VAL;
    FWK_RW  uint64_t  CCLA_PORTFWD_EN;
    FWK_R   uint64_t  CCLA_PORTFWD_STATUS;
    FWK_RW  uint64_t  CCLA_PORTFWD_REQ;
    FWK_RW  uint64_t  CCLA_LINKID_TO_HOPS;
            uint8_t   RESERVED6[0xE00 - 0xD98];
    FWK_RW  uint64_t  CCLA_CXL_LINK_RX_CREDIT_CTL;
    FWK_R   uint64_t  CCLA_CXL_LINK_RX_CREDIT_RETURN_STAT;
    FWK_R   uint64_t  CCLA_CXL_LINK_TX_CREDIT_STAT;
    FWK_RW  uint64_t  CCLA_CXL_LINK_LAYER_DEFEATURE;
    FWK_RW  uint64_t  CCLA_ULL_CTL;
    FWK_R   uint64_t  CCLA_ULL_STATUS;
    FWK_RW  uint64_t  CCLA_CXL_LL_ERRINJECT_CTL;
    FWK_R   uint64_t  CCLA_CXL_LL_ERRINJECT_STAT;
    FWK_RW  uint64_t  CCLA_CXL_VIRAL_PROP_EN;
            uint8_t   RESERVED7[0xE50 - 0xE48];
    FWK_RW  uint64_t  CCLA_CXL_SECURITY_POLICY;
            uint8_t   RESERVED8[0xE78 - 0xE58];
    FWK_R   uint64_t  CCLA_CXL_HDM_DECODER_CAPABILITY;
    FWK_RW  uint64_t  CCLA_CXL_HDM_DECODER_GLOBAL_CONTROL;
            uint8_t   RESERVED9[0xED0 - 0xE88];
    FWK_RW  uint64_t  CCLA_SNOOP_FILTER_GROUP_ID;
    FWK_RW  uint64_t  CCLA_SNOOP_FILTER_EFFECTIVE_SIZE;
            uint8_t   RESERVED10[0xF08 - 0xEF0];
    FWK_RW  uint64_t  CCLA_DVSEC_CXL_CONTROL2;
    FWK_RW  uint64_t  CCLA_DVSEC_CXL_LOCK;
    FWK_RW  uint64_t  CCLA_DVSEC_FLEX_BUS_PORT_CONTROL;
            uint8_t   RESERVED11;
    FWK_RW  uint64_t  CCLA_ROOT_PORT_N_SECURITY_POLICY;
    FWK_RW  uint64_t  POR_CCLA_ROOT_PORT_N_ID;
            uint8_t   RESERVED12[0xF40 - 0xF38];
    FWK_RW  uint64_t  CCLA_ERR_CAPABILITIES_CONTROL;
            uint8_t   RESERVED13[0xF58 - 0xF48];
    FWK_RW  uint64_t  CCLA_IDE_KEY_REFRESH_TIME_CONTROL;
    FWK_RW  uint64_t  CCLA_IDE_TRUNCATION_TRANSMIT_DELAY_CONTROL;
            uint8_t   RESERVED14;
    FWK_RW  uint64_t  CCLA_LL_TO_ULL_MSG;
    FWK_RW  uint64_t  CCLA_CXL_TIMEOUT_ISO_STATUS;
    FWK_RW  uint64_t  CCLA_CXL_TIMEOUT_ISO_CTL;
    FWK_RW  uint64_t  CCLA_CXL_TIMEOUT_ISO_CAPABILITY;
    FWK_R   uint64_t  CCLA_IDE_OUTBOUND_STATE;
    FWK_R   uint64_t  CCLA_IDE_INBOUND_STATE;
            uint8_t   RESERVED15[0x1000 - 0xFA0];
    FWK_RW  uint64_t  CCLA_DVSEC_CXL_RANGE_1_SIZE_HIGH;
    FWK_RW  uint64_t  CCLA_DVSEC_CXL_RANGE_1_SIZE_LOW;
    FWK_RW  uint64_t  CCLA_DVSEC_CXL_RANGE_2_SIZE_HIGH;
    FWK_RW  uint64_t  CCLA_DVSEC_CXL_RANGE_2_SIZE_LOW;
    FWK_RW  uint64_t  CCLA_TIMEOUT_BASE_VALUE_CYCLES;
            uint8_t   RESERVED16[0x1450 - 0x1028];
    FWK_RW  uint64_t  CCLA_CXL_IDE_PYLD;
    FWK_RW  uint64_t  CCLA_ULL_IDLE_COUNTER;
            uint8_t   RESERVED17[0x1500 - 0x1460];
    FWK_RW  uint64_t  CCLA_IDE_FREQ_FACTOR;
            uint8_t   RESERVED18[0x1F90 - 0x1508];
    FWK_RW  uint64_t  CCLA_RAS_ERR_EN;
    FWK_RW  uint64_t  CCLA_RAS_VIRAL_EN;
            uint8_t   RESERVED19[0xD908 - 0x1FA0];
    FWK_RW  uint64_t  CCLA_PMU_EVENT_SEL;
            uint8_t   RESERVED20[0xE000 - 0xD910];
    FWK_R   uint64_t  CCLA_ERRFR;
    FWK_RW  uint64_t  CCLA_ERRCTLR;
    FWK_W   uint64_t  CCLA_ERRSTATUS;
    FWK_RW  uint64_t  CCLA_ERRADDR;
            uint8_t   RESERVED21;
    FWK_RW  uint64_t  CCLA_ERRMISC1;
            uint8_t   RESERVED22[0xE040 - 0xE030];
    FWK_R   uint64_t  CCLA_ERRFR_NS;
    FWK_RW  uint64_t  CCLA_ERRCTLR_NS;
    FWK_W   uint64_t  CCLA_ERRSTATUS_NS;
    FWK_RW  uint64_t  CCLA_ERRADDR_NS;
            uint8_t   RESERVED23;
    FWK_RW  uint64_t  CCLA_ERRMISC1_NS;
            uint8_t   RESERVED24[0xE800 - 0xE070];
    FWK_R   uint64_t  CCLA_ERRPFGF;
    FWK_RW  uint64_t  CCLA_ERRPFGCTL;
    FWK_RW  uint64_t  CCLA_ERRPFGCDN;
            uint8_t   RESERVED25[0xE840 - 0xE818];
    FWK_R   uint64_t  CCLA_ERRPFGF_NS;
    FWK_RW  uint64_t  CCLA_ERRPFGCTL_NS;
    FWK_RW  uint64_t  CCLA_ERRPFGCDN_NS;
            uint8_t   RESERVED26[0xED00 - 0xE858];
    FWK_RW  uint64_t  CCLA_ERRCAPCTL;
            uint8_t   RESERVED27[0xEE00 - 0xED08];
    FWK_R   uint64_t  CCLA_ERRGSR;
            uint8_t   RESERVED28;
    FWK_R   uint64_t  CCLA_ERRIIDR;
            uint8_t   RESERVED29[0xEFA8 - 0xEE18];
    FWK_R   uint64_t  CCLA_ERRDEVAF;
            uint8_t   RESERVED30;
    FWK_R   uint64_t  CCLA_ERRDEVARCH;
            uint8_t   RESERVED31;
    FWK_R   uint64_t  CCLA_ERRDEVID;
    FWK_R   uint64_t  CCLA_ERRPIDR45;
            uint8_t   RESERVED32;
    FWK_R   uint64_t  CCLA_ERRPIDR01;
    FWK_R   uint64_t  CCLA_ERRPIDR23;
    FWK_R   uint64_t  CCLA_ERRCIDR01;
    FWK_R   uint64_t  CCLA_ERRCIDR23;
};
// clang-format on

#endif /* CMN_CYPRUS_REG_INTERNAL_H */
