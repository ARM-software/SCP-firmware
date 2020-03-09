/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the CMN600 module.
 */

#ifndef CMN600_H
#define CMN600_H

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

#define CMN600_MAX_NUM_RNF    32
#define CMN600_MAX_NUM_HNF    32
#define CMN600_MAX_NUM_SCG    4

#define SAM_GRANULARITY       (64 * FWK_MIB)

enum node_id_size {
    NODE_ID_SIZE_7BITS,
    NODE_ID_SIZE_9BITS,
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
    NODE_TYPE_RN_I      = 0xA,
    NODE_TYPE_RN_D      = 0xD,
    NODE_TYPE_RN_SAM    = 0xF,
    /* Coherent Multichip Link (CML) node types */
    NODE_TYPE_CML_BASE  = 0x100,
    NODE_TYPE_CXRA      = 0x100,
    NODE_TYPE_CXHA      = 0x101,
    NODE_TYPE_CXLA      = 0x102,
};

enum device_type {
    DEVICE_TYPE_CXHA    = 0x11, // 0b10001
    DEVICE_TYPE_CXRA    = 0x12, // 0b10010
    DEVICE_TYPE_CXRH    = 0x13, // 0b10011
};

/* Common node header */
struct node_header {
    FWK_R uint64_t NODE_INFO;
          uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R uint64_t CHILD_INFO;
          uint8_t  RESERVED1[0x100 - 0x88];
    FWK_R uint64_t CHILD_POINTER[256];
};

struct cluster_mapping {
    unsigned int node_id;
    struct node_header *node;
};

enum sam_node_type {
    SAM_NODE_TYPE_HN_F = 0,
    SAM_NODE_TYPE_HN_I = 1,
    SAM_NODE_TYPE_CXRA = 2,
    SAM_NODE_TYPE_COUNT
};

/*
 * Request Node System Address Map (RN-SAM) registers
 */
struct cmn600_rnsam_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t UNIT_INFO;
           uint8_t  RESERVED2[0x980 - 0x908];
    FWK_RW uint64_t SECURE_REGISTER_GROUPS_OVERRIDE;
           uint8_t  RESERVED3[0xC00 - 0x988];
    FWK_RW uint64_t STATUS;
    FWK_RW uint64_t NON_HASH_MEM_REGION[5];
    FWK_RW uint64_t NON_HASH_TGT_NODEID[3];
    FWK_RW uint64_t SYS_CACHE_GRP_REGION[2];
    FWK_RW uint64_t SYS_CACHE_GRP_HN_NODEID[8];
    FWK_RW uint64_t SYS_CACHE_GRP_NOHASH_NODEID;
           uint8_t  RESERVED4[0xD00 - 0xCA0];
    FWK_RW uint64_t SYS_CACHE_GRP_HN_COUNT;
    FWK_RW uint64_t SYS_CACHE_GRP_SN_NODEID[8];
    FWK_RW uint64_t SYS_CACHE_GRP_SN_SAM_CFG[2];
           uint8_t  RESERVED5[0xF10 - 0xD58];
    FWK_RW uint64_t SYS_CACHE_GRP_CAL_MODE;
};

/*
 * Debug and Trace registers
 */
struct cmn600_dt_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x980 - 0x88];
    FWK_RW uint64_t SECURE_ACCESS;
           uint8_t  RESERVED2[0xA00 - 0x988];
    FWK_RW uint64_t DTC_CTL;
           uint8_t  RESERVED3[0xA10 - 0xA08];
    FWK_R  uint64_t TRIGGER_STATUS;
           uint8_t  RESERVED4[0xA20 - 0xA18];
    FWK_W  uint64_t TRIGGER_STATUS_CLR;
           uint8_t  RESERVED5[0xA30 - 0xA28];
    FWK_RW uint64_t TRACE_CONTROL;
           uint8_t  RESERVED6[0xA40 - 0xA38];
    FWK_R  uint64_t DBG_ID;
    FWK_RW uint64_t TRACE_ID;
           uint8_t  RESERVED7[0x2000 - 0xA50];
    FWK_RW uint64_t PMEVCNT[8];
    FWK_RW uint64_t PMCCNTR;
           uint8_t  RESERVED8[0x2050 - 0x2048];
    FWK_RW uint64_t PMEVCNTSR[8];
    FWK_RW uint64_t PMCCNTRSR;
           uint8_t  RESERVED9[0x2100 - 0x2098];
    FWK_RW uint64_t PMCR;
    FWK_RW uint64_t PMEVCNT_LOCALNUM;
           uint8_t  RESERVED10[0x2118 - 0x2110];
    FWK_R  uint64_t PMOVSR;
    FWK_W  uint64_t PMOVSR_CLR;
    FWK_R  uint64_t PMSR;
    FWK_W  uint64_t PMSR_REQ;
    FWK_W  uint64_t PMSR_CLR;
           uint8_t  RESERVED11[0x2DA0 - 0x2140];
    FWK_RW uint64_t CLAIM;
    FWK_R  uint64_t DEVAFF;
    FWK_R  uint64_t LSR;
    FWK_R  uint64_t AUTHSTATUS_DEVARCH;
    FWK_R  uint64_t DEVID;
    FWK_R  uint64_t DEVTYPE;
    FWK_R  uint64_t PIDR45;
    FWK_R  uint64_t PIDR67;
    FWK_R  uint64_t PIDR01;
    FWK_R  uint64_t PIDR23;
    FWK_R  uint64_t CIDR01;
    FWK_R  uint64_t CIDR23;
};

/*
 * Device Node registers
 */
struct cmn600_dn_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t BUILD_INFO;
           uint8_t  RESERVED2[0x980 - 0x908];
    FWK_RW uint64_t SECURE_REGISTER_GROUPS_OVERRIDE;
           uint8_t  RESERVED3[0xA00 - 0x988];
    FWK_RW uint64_t AUX_CTL;
           uint8_t  RESERVED4[0xC00 - 0xA08];
    FWK_RW uint64_t VMF0_CTRL;
    FWK_RW uint64_t VMF0_RNF[4];
           uint8_t  RESERVED5[0xC40 - 0xC28];
    FWK_RW uint64_t VMF0_RND;
           uint8_t  RESERVED6[0xC60 - 0xC48];
    FWK_RW uint64_t VMF0_RNC;
           uint8_t  RESERVED7[0xC80 - 0xC68];
    FWK_RW uint64_t VMF1_CTRL;
    FWK_RW uint64_t VMF1_RNF[4];
           uint8_t  RESERVED8[0xCC0 - 0xCA8];
    FWK_RW uint64_t VMF1_RND;
           uint8_t  RESERVED9[0xCE0 - 0xCC8];
    FWK_RW uint64_t VMF1_RNC;
           uint8_t  RESERVED10[0xD00 - 0xCE8];
    FWK_RW uint64_t VMF2_CTRL;
    FWK_RW uint64_t VMF2_RNF[4];
           uint8_t  RESERVED11[0xD40 - 0xD28];
    FWK_RW uint64_t VMF2_RND;
           uint8_t  RESERVED12[0xD60 - 0xD48];
    FWK_RW uint64_t VMF2_RNC;
           uint8_t  RESERVED13[0xD80 - 0xD68];
    FWK_RW uint64_t VMF3_CTRL;
    FWK_RW uint64_t VMF3_RNF[4];
           uint8_t  RESERVED14[0xDC0 - 0xDA8];
    FWK_RW uint64_t VMF3_RND;
           uint8_t  RESERVED15[0xDE0 - 0xDC8];
    FWK_RW uint64_t VMF3_RNC;
           uint8_t  RESERVED16[0x2000 - 0xDE8];
    FWK_RW uint64_t PMU_EVEN_SEL;
};


/*
 * Quality of Service (QoS) registers
 */
struct cmn600_qos_reg {
    FWK_RW uint64_t QOS_CONTROL;
    FWK_RW uint64_t QOS_LAST_TGT;
    FWK_RW uint64_t QOS_LAST_SCALE;
    FWK_RW uint64_t QOS_LAST_RANGE;
};

/*
 * Request Node Device (RN-D) registers
 */
struct cmn600_rnd_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t UNIT_INFO;
           uint8_t  RESERVED2[0x980 - 0x908];
    FWK_RW uint64_t SECURE_REGISTER_GROUPS_OVERRIDE;
           uint8_t  RESERVED3[0xA00 - 0x988];
    FWK_RW uint64_t CFG_CTL;
    FWK_RW uint64_t AUX_CTL;
    FWK_RW uint64_t S_PORT_CONTROL[3];
           uint8_t  RESERVED4[0xA80 - 0xA28];
    struct cmn600_qos_reg S_QOS[3];
           uint8_t  RESERVED5[0x1000 - 0xAE0];
    FWK_RW uint64_t SYSCOREQ_CTL;
    FWK_R  uint64_t SYSCOACK_STATUS;
           uint8_t  RESERVED6[0x2000 - 0x1010];
    FWK_RW uint64_t PMU_EVENT_SEL;
};

/*
 * Fully Coherent Home Node (HN-F) registers
 */
struct cmn600_hnf_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t UNIT_INFO;
           uint8_t  RESERVED2[0x980 - 0x908];
    FWK_RW uint64_t SECURE_REGISTER_GROUPS_OVERRIDE;
           uint8_t  RESERVED3[0xA00 - 0x988];
    FWK_RW uint64_t CFG_CTL;
    FWK_RW uint64_t AUX_CTL;
           uint8_t  RESERVED4[0xA80 - 0xA10];
    FWK_R  uint64_t QOS_BAND;
    FWK_RW uint64_t QOS_RESERVATION;
    FWK_RW uint64_t RN_STARVATION;
           uint8_t  RESERVED5[0xB80 - 0xA98];
    FWK_W  uint64_t CFG_L3SF_DBGRD;
    FWK_R  uint64_t L3_CACHE_ACCESS_L3_TAG;
    FWK_R  uint64_t L3_CACHE_ACCESS_L3_DATA;
    FWK_R  uint64_t L3_CACHE_ACCESS_SF_DATA;
           uint8_t  RESERVED6[0xC00 - 0xBA0];
    FWK_RW uint64_t L3_LOCK_WAYS;
    FWK_RW uint64_t L3_LOCK_BASE[4];
           uint8_t  RESERVED7[0xC30 - 0xC28];
    FWK_RW uint64_t RNID_REGION_VEC;
    FWK_RW uint64_t RNF_REGION_VEC;
           uint8_t  RESERVED8[0xD00 - 0xC40];
    FWK_RW uint64_t SAM_CONTROL;
    FWK_RW uint64_t SAM_MEMREGION[2];
    FWK_RW uint64_t SAM_SN_PROPERTIES;
    FWK_RW uint64_t SAM_6SN_NODEID;
    FWK_RW uint64_t RN_PHYS_ID[32];
           uint8_t  RESERVED9[0xF00 - 0xE28];
    FWK_RW uint64_t SF_CXG_BLOCKED_WAYS;
           uint8_t  RESERVED10[0xF10 - 0xF08];
    FWK_RW uint64_t CML_PORT_AGGR_GRP0_ADD_MASK;
           uint8_t  RESERVED11[0xF28 - 0xF18];
    FWK_RW uint64_t CML_PORT_AGGR_GRP0_ADD_REG;
           uint8_t  RESERVED12[0x1000 - 0xF30];
    FWK_RW uint64_t PPU_PWPR;
    FWK_R  uint64_t PPU_PWSR;
           uint8_t  RESERVED13[0x1014 - 0x1010];
    FWK_R  uint32_t PPU_MISR;
           uint8_t  RESERVED14[0x1100 - 0x1018];
    FWK_RW uint64_t PPU_DYN_RET_THRESHOLD;
           uint8_t  RESERVED15[0x1FB0 - 0x1108];
    FWK_R  uint32_t PPU_IDR[2];
           uint8_t  RESERVED16[0x1FC8 - 0x1FB8];
    FWK_R  uint32_t PPU_IIDR;
    FWK_R  uint32_t PPU_AIDR;
           uint8_t  RESERVED17[0x2000 - 0x1FD0];
    FWK_RW uint64_t PMU_EVENT_SEL;
           uint8_t  RESERVED18[0x3000 - 0x2008];
    FWK_R  uint64_t ERRFR;
    FWK_RW uint64_t ERRCTLR;
    FWK_RW uint64_t ERRSTATUS;
    FWK_RW uint64_t ERRADDR;
    FWK_RW uint64_t ERRMISC;
           uint8_t  RESERVED19[0x3030 - 0x3028];
    FWK_RW uint64_t RN_ERR_INJ;
    FWK_W  uint64_t RN_BYTE_PAR_ERR_INJ;
           uint8_t  RESERVED20[0x3100 - 0x3040];
    FWK_R  uint64_t ERRFR_NS;
    FWK_RW uint64_t ERRCTLR_NS;
    FWK_RW uint64_t ERRSTATUS_NS;
    FWK_RW uint64_t ERRADDR_NS;
    FWK_RW uint64_t ERRMISC_NS;
};

/*
 * AMBA 5 CHI to AXI bridge registers
 */
struct cmn600_sbsx_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t UNIT_INFO;
           uint8_t  RESERVED2[0xA00 - 0x908];
    FWK_RW uint64_t AUX_CTL;
           uint8_t  RESERVED3[0x2000 - 0xA08];
    FWK_RW uint64_t PMU_EVENT_SEL;
           uint8_t  RESERVED4[0x3000 - 0x2008];
    FWK_R  uint64_t ERRFR;
    FWK_RW uint64_t ERRCTL;
    FWK_RW uint64_t ERRSTATUS;
    FWK_RW uint64_t ERRADDR;
    FWK_RW uint64_t ERRMISC;
           uint8_t  RESERVED5[0x3100 - 0x3028];
    FWK_R  uint64_t ERRFR_NS;
    FWK_RW uint64_t ERRCTL_NS;
    FWK_RW uint64_t ERRSTATUS_NS;
    FWK_RW uint64_t ERRADDR_NS;
    FWK_RW uint64_t ERRMISC_NS;
};

/*
 * Configuration slave registers
 */
struct cmn600_cfgm_reg {
    FWK_R  uint64_t NODE_INFO;
    FWK_RW uint64_t PERIPH_ID[4];
    FWK_RW uint64_t COMPONENT_ID[2];
           uint8_t  RESERVED0[0x80 - 0x38];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x100 - 0x88];
    FWK_R  uint64_t CHILD_POINTER[256];
    FWK_R  uint64_t INFO_GLOBAL;
           uint8_t  RESERVED2[0x980 - 0x908];
    FWK_RW uint64_t SECURE_ACCESS;
    FWK_RW uint64_t SECURE_REGISTER_GROUPS_OVERRIDE;
           uint8_t  RESERVED3[0x1000 - 0x990];
    FWK_RW uint64_t PPU_INT_MASK;
    FWK_RW uint64_t PPU_INT_STATUS;
           uint8_t  RESERVED4[0x3000 - 0x1010];
    FWK_R  uint64_t ERRGSR[8];
           uint8_t  RESERVED5[0x3100 - 0x3040];
    FWK_R  uint64_t ERRGSR_NS[8];
           uint8_t  RESERVED6[0x3FA8 - 0x3140];
    FWK_R  uint64_t ERRDEVAFF;
           uint8_t  RESERVED7[0x3FB8 - 0x3FB0];
    FWK_R  uint64_t ERRDEVARCH;
           uint8_t  RESERVED8[0x3FC8 - 0x3FC0];
    FWK_R  uint64_t ERRIDR;
    FWK_R  uint64_t ERRPIDR45;
    FWK_R  uint64_t ERRPIDR67;
    FWK_R  uint64_t ERRPIDR01;
    FWK_R  uint64_t ERRPIDR23;
    FWK_R  uint64_t ERRCIDR01;
    FWK_R  uint64_t ERRCIDR23;
};

/*
 * Crosspoint (XP) registers
 */
struct cmn600_mxp_reg {
    FWK_R  uint64_t NODE_INFO;
    FWK_R  uint64_t PORT_CONNECT_INFO[2];
           uint8_t  RESERVED0[0x80 - 0x18];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x100 - 0x88];
    FWK_R  uint64_t CHILD_POINTER[16];
           uint8_t  RESERVED2[0x900 - 0x180];
    FWK_R  uint64_t P0_INFO;
    FWK_R  uint64_t P1_INFO;
           uint8_t  RESERVED3[0x980 - 0x910];
    FWK_RW uint64_t SECURE_REGISTER_GROUPS_OVERRIDE;
           uint8_t  RESERVED4[0xA00 - 0x988];
    FWK_RW uint64_t AUX_CTL;
           uint8_t  RESERVED5[0xA80 - 0xA08];
    struct cmn600_qos_reg P_QOS[2];
           uint8_t  RESERVED6[0x1000 - 0xAC0];
    FWK_RW uint64_t P_SYSCOREQ_CTL[2];
    FWK_R  uint64_t P_SYSCOACK_STATUS[2];
           uint8_t  RESERVED7[0x2000 - 0x1020];
    FWK_RW uint64_t PMU_EVENT_SEL;
           uint8_t  RESERVED8[0x2100 - 0x2008];
    FWK_RW uint64_t DTM_CONTROL;
           uint8_t  RESERVED9[0x2118 - 0x2108];
    FWK_R  uint64_t DTM_FIFO_ENTRY_READY;
    FWK_R  uint64_t DTM_FIFO_ENTRY0[3];
    FWK_R  uint64_t DTM_FIFO_ENTRY1[3];
    FWK_R  uint64_t DTM_FIFO_ENTRY2[3];
    FWK_R  uint64_t DTM_FIFO_ENTRY3[3];
           uint8_t  RESERVED10[0x21A0 - 0x2180];
    FWK_RW uint64_t DTM_WP0_CONFIG;
    FWK_RW uint64_t DTM_WP0_VAL;
    FWK_RW uint64_t DTM_WP0_MASK;
    FWK_RW uint64_t DTM_WP1_CONFIG;
    FWK_RW uint64_t DTM_WP1_VAL;
    FWK_RW uint64_t DTM_WP1_MASK;
    FWK_RW uint64_t DTM_WP2_CONFIG;
    FWK_RW uint64_t DTM_WP2_VAL;
    FWK_RW uint64_t DTM_WP2_MASK;
    FWK_RW uint64_t DTM_WP3_CONFIG;
    FWK_RW uint64_t DTM_WP3_VAL;
    FWK_RW uint64_t DTM_WP3_MASK;
    FWK_RW uint64_t PMSICR;
    FWK_RW uint64_t PMSIRR;
    FWK_RW uint64_t DTM_PMU_CONFIG;
           uint8_t  RESERVED11[0x2220 - 0x2218];
    FWK_RW uint64_t PMEVCNT[4];
    FWK_RW uint64_t PMEVCNTSR[4];
           uint8_t  RESERVED12[0x3000 - 0x2260];
    FWK_R  uint64_t ERRFR;
    FWK_RW uint64_t ERRCTL;
    FWK_RW uint64_t ERRSTATUS;
           uint8_t  RESERVED13[0x3028 - 0x3018];
    FWK_RW uint64_t ERRMISC;
    FWK_W  uint64_t P_BYTE_PAR_ERR_INJ[2];
           uint8_t  RESERVED14[0x3100 - 0x3040];
    FWK_R  uint64_t ERRFR_NS;
    FWK_RW uint64_t ERRCTL_NS;
    FWK_RW uint64_t ERRSTATUS_NS;
           uint8_t  RESERVED15[0x3128 - 0x3118];
    FWK_RW uint64_t ERRMISC_NS;
};

/*
 * Request Node I/O (RN-I)registers
 */
struct cmn600_rni_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t UNIT_INFO;
           uint8_t  RESERVED2[0x980 - 0x908];
    FWK_RW uint64_t SECURE_REGISTER_GROUPS_OVERRIDE;
           uint8_t  RESERVED3[0xA00 - 0x988];
    FWK_RW uint64_t CFG_CTL;
    FWK_RW uint64_t AUX_CTL;
    FWK_RW uint64_t S0_PORT_CONTRL;
    FWK_RW uint64_t S1_PORT_CONTRL;
    FWK_RW uint64_t S2_PORT_CONTRL;
           uint8_t  RESERVED4[0xA80 - 0xA28];
    struct cmn600_qos_reg S_QOS[3];
           uint8_t  RESERVED5[0x2000 - 0xAE0];
    FWK_RW uint64_t PMU_EVENT_SEL;
};

/*
 * Home Node I/O (HN-I) registers
 */
struct cmn600_hni_reg {
    FWK_R  uint64_t NODE_INFO;
           uint8_t  RESERVED0[0x80 - 0x8];
    FWK_R  uint64_t CHILD_INFO;
           uint8_t  RESERVED1[0x900 - 0x88];
    FWK_R  uint64_t UNIT_INFO;
           uint8_t  RESERVED2[0x980 - 0x908];
    FWK_RW uint64_t SECURE_REGISTER_GROUPS_OVERRIDE;
           uint8_t  RESERVED3[0xA00 - 0x988];
    FWK_RW uint64_t CFG_CTL;
    FWK_RW uint64_t AUX_CTL;
           uint8_t  RESERVED4[0xC00 - 0xA10];
    FWK_RW uint64_t SAM_ADDRREGION_CFG[4];
           uint8_t  RESERVED5[0x2000 - 0xC20];
    FWK_RW uint64_t PMU_EVENT_SEL;
           uint8_t  RESERVED6[0x3000 - 0x2008];
    FWK_R  uint64_t ERRFR;
    FWK_RW uint64_t ERRCTL;
    FWK_RW uint64_t ERRSTATUS;
    FWK_RW uint64_t ERRADDR;
    FWK_RW uint64_t ERRMISC;
           uint8_t  RESERVED7[0x3100 - 0x3028];
    FWK_R  uint64_t ERRFR_NS;
    FWK_RW uint64_t ERRCTL_NS;
    FWK_RW uint64_t ERRSTATUS_NS;
    FWK_RW uint64_t ERRADDR_NS;
    FWK_RW uint64_t ERRMISC_NS;
};

#define CMN600_NODE_INFO_TYPE           UINT64_C(0x000000000000FFFF)
#define CMN600_NODE_INFO_ID             UINT64_C(0x00000000FFFF0000)
#define CMN600_NODE_INFO_ID_POS         16
#define CMN600_NODE_INFO_LOGICAL_ID     UINT64_C(0x0000FFFF00000000)
#define CMN600_NODE_INFO_LOGICAL_ID_POS 32

#define CMN600_CHILD_INFO_COUNT         UINT64_C(0x000000000000FFFF)

#define CMN600_CHILD_POINTER            UINT64_C(0x00000000FFFFFFFF)
#define CMN600_CHILD_POINTER_OFFSET     UINT64_C(0x000000000FFFFFFF)
#define CMN600_CHILD_POINTER_EXT        UINT64_C(0x0000000080000000)

/* External child node */
#define CMN600_CHILD_POINTER_EXT_REGISTER_OFFSET  UINT64_C(0x0000000000003FFF)
#define CMN600_CHILD_POINTER_EXT_NODE_POINTER     UINT64_C(0x000000000FFFC000)
#define CMN600_CHILD_POINTER_EXT_NODE_POINTER_POS 14

/* Used by NON_HASH_MEM_REGIONx and SYS_CACHE_GRP_REGIONx group registers */
#define CMN600_RNSAM_REGION_ENTRY_VALID_POS  0
#define CMN600_RNSAM_REGION_ENTRY_EN_POS 1 /* Not available for all regions */
#define CMN600_RNSAM_REGION_ENTRY_TYPE_POS 2
#define CMN600_RNSAM_REGION_ENTRY_SIZE_POS 4
#define CMN600_RNSAM_REGION_ENTRY_BASE_POS 9
#define CMN600_RNSAM_REGION_ENTRY_BITS_WIDTH 32
#define CMN600_RNSAM_REGION_ENTRY_VALID UINT64_C(0x0000000000000001)
#define CMN600_RNSAM_REGION_ENTRY_MASK UINT64_C(0xFFFFFFFF)
#define CMN600_RNSAM_REGION_ENTRIES_PER_GROUP 2
#define CMN600_RNSAM_MAX_HASH_MEM_REGION_ENTRIES 4
#define CMN600_RNSAM_MAX_NON_HASH_MEM_REGION_ENTRIES 20
#define CMN600_RNSAM_SCG_HNF_CAL_MODE_EN UINT64_C(0x01)
#define CMN600_RNSAM_SCG_HNF_CAL_MODE_SHIFT 16

#define CMN600_RNSAM_STATUS_UNSTALL UINT64_C(0x0000000000000002)

#define CMN600_HNF_SAM_MEMREGION_SIZE_POS 12
#define CMN600_HNF_SAM_MEMREGION_BASE_POS 26
#define CMN600_HNF_SAM_MEMREGION_VALID UINT64_C(0x8000000000000000)

#define CMN600_HNF_CACHE_GROUP_ENTRIES_MAX 32
#define CMN600_HNF_CACHE_GROUP_ENTRIES_PER_GROUP 4
#define CMN600_HNF_CACHE_GROUP_ENTRY_BITS_WIDTH 12

#define CMN600_HNI_SAM_REGION_CFG_SER_DEVNE_WR UINT64_C(0x1000000000000000)

#define CMN600_RND_CFG_CTL_PCIE_MSTR_PRESENT UINT64_C(0x0000000000000020)

#define CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRY_BITS_WIDTH 12
#define CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK UINT64_C(0xFFF)
#define CMN600_RNSAM_NON_HASH_TGT_NODEID_ENTRIES_PER_GROUP 4

#define CMN600_PPU_PWPR_POLICY_OFF UINT64_C(0x0000000000000000)
#define CMN600_PPU_PWPR_POLICY_MEM_RET UINT64_C(0x0000000000000002)
#define CMN600_PPU_PWPR_POLICY_FUNC_RET UINT64_C(0x000000000000007)
#define CMN600_PPU_PWPR_POLICY_ON UINT64_C(0x0000000000000008)
#define CMN600_PPU_PWPR_OPMODE_NOSFSLC UINT64_C(0x0000000000000000)
#define CMN600_PPU_PWPR_OPMODE_SFONLY UINT64_C(0x0000000000000010)
#define CMN600_PPU_PWPR_OPMODE_HAM UINT64_C(0x0000000000000020)
#define CMN600_PPU_PWPR_OPMODE_FAM UINT64_C(0x0000000000000030)
#define CMN600_PPU_PWPR_DYN_EN UINT64_C(0x0000000000000100)

/* Mesh and Node ID mapping */
#define CMN600_MESH_X_MAX 8
#define CMN600_MESH_Y_MAX 8

#define CMN600_NODE_ID_PORT_POS 2
#define CMN600_NODE_ID_PORT_MASK 0x1
#define CMN600_NODE_ID_Y_POS 3

#define CMN600_MXP_PORT_CONNECT_INFO_DEVICE_TYPE_MASK UINT64_C(0x1F)

#define CMN600_ROOT_NODE_OFFSET_PORT_POS 14
#define CMN600_ROOT_NODE_OFFSET_Y_POS 20

/* Peripheral ID Revision Numbers */
#define CMN600_PERIPH_ID_2_REV_R1_P0 ((0x00 << 4) + (0x0B))
#define CMN600_PERIPH_ID_2_REV_R1_P1 ((0x01 << 4) + (0x0B))
#define CMN600_PERIPH_ID_2_REV_R1_P2 ((0x02 << 4) + (0x0B))
#define CMN600_PERIPH_ID_2_REV_R1_P3 ((0x03 << 4) + (0x0B))
#define CMN600_PERIPH_ID_2_REV_R2_P0 ((0x04 << 4) + (0x0B))
#define CMN600_PERIPH_ID_2_REV_R3_P0 ((0x05 << 4) + (0x0B))
#define CMN600_PERIPH_ID_2_REV_R3_P1 ((0x06 << 4) + (0x0B))

/* Peripheral ID Revision Numbers */
#define CMN600_PERIPH_ID_2_MASK UINT64_C(0xFF)

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
 * Retrieve the revision number of CMN-600.
 *
 * \param root Pointer to the CMN-600 configuration master register.
 *
 * \return CMN-600 revision as integer value.
 */
unsigned int get_cmn600_revision(struct cmn600_cfgm_reg *root);

/*
 * Verify if a child node (given a parent node base and child index) is an
 * external node from the CMN600 instance point of view.
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
 *
 * \retval port number (either 0 or 1).
 */
bool get_port_number(unsigned int child_node_id);

/*
 * Returns the device type from the MXP's port connect info register.
 *
 * \param mxp_base Pointer to the cross point node descriptor
 *      \pre The cross point node pointer must be valid
 * \param port Port number
 *      \pre The port number should be either 0 or 1.
 *
 * \retval device type (por_mxp_por_mxp_device_port_connect_info_p[port] & 0x1F)
 */
unsigned int get_device_type(void *mxp_base, bool port);

/*
 * Convert a memory region size into a size format used by the CMN600 registers
 * The format is the binary logarithm of the memory region size represented as
 * blocks multiple of the CMN600's granularity:
 * n =  log2(size / SAM_GRANULARITY)
 *
 * \param size Memory region size to be converted
 *      \pre size must be a multiple of SAM_GRANULARITY
 *
 * \return log2(size / SAM_GRANULARITY)
 */
uint64_t sam_encode_region_size(uint64_t size);

/*
 * Configure a memory region
 *
 * \param[out] reg Region group descriptor to be configured
 *      \pre Must be a valid pointer
 * \param bit_offset Bit offset of the memory region in the group descriptor
 * \param base Region base address
 * \param size Region size
 * \param node_type Type of the target node
 *
 * \return None
 */
void configure_region(volatile uint64_t *reg, unsigned int bit_offset,
    uint64_t base, uint64_t size, enum sam_node_type node_type);

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
 * Get the root node descriptor based on the peripheral base, HN-D node
 * identifier and mesh size.
 *
 * \param base CMN600 peripheral base address
 * \param hnd_node_id HN-D node identifier containing the global configuration
 * \param mesh_size_x Size of the mesh along the x-axis
 * \param mesh_size_y Size of the mesh along the x-axis
 *
 * \return Pointer to the root node descriptor
 */
struct cmn600_cfgm_reg *get_root_node(uintptr_t base, unsigned int hnd_node_id,
    unsigned int mesh_size_x, unsigned int mesh_size_y);

/*
 * Check if CMN600 supports CAL mode. CAL mode is supported from CMN600 r2p0.
 *
 * \param root Pointer to the root node descriptor
 *
 * \retval true if the CMN600 revision is found to be r2p0 or above
 * \retval false if the CMN600 revision is found to be r1p3 or below
 */
bool is_cal_mode_supported(struct cmn600_cfgm_reg *root);

#endif /* CMN600_H */
