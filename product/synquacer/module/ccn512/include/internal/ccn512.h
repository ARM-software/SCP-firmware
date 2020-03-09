/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_CCN512_H
#define INTERNAL_CCN512_H

#include <fwk_macros.h>

#include <stdint.h>

/*
 * CCN5xx miscellaneous node (MN) registers
 */
typedef struct {
    FWK_RW uint64_t SECURE_ACCESS;
    FWK_RW uint64_t ERRINT_STATUS;
    uint8_t RESERVED0[0x170];
    FWK_R uint64_t OLY_RNF_NODEID_LIST;
    uint8_t RESERVED1[0x8];
    FWK_R uint64_t OLY_RNI_NODEID_LIST;
    uint8_t RESERVED2[0x8];
    FWK_R uint64_t OLY_RNIDVM_NODEID_LIST;
    uint8_t RESERVED3[0x8];
    FWK_R uint64_t OLY_HNF_NODEID_LIST;
    uint8_t RESERVED4[0x8];
    FWK_R uint64_t OLY_HNI_NODEID_LIST;
    uint8_t RESERVED5[0x8];
    FWK_R uint64_t OLY_SN_NODEID_LIST;
    uint8_t RESERVED6[0x8];
    FWK_R uint64_t OLY_COMP_LIST_63_0;
    FWK_R uint64_t OLY_COMP_LIST_127_64;
    FWK_R uint64_t OLY_COMP_LIST_191_128;
    FWK_R uint64_t OLY_COMP_LIST_255_192;
    FWK_R uint64_t DVM_DOMAIN_CTL;
    uint8_t RESERVED7[0x8];
    FWK_W uint64_t DVM_DOMAIN_CTL_SET;
    uint8_t RESERVED8[0x8];
    FWK_W uint64_t DVM_DOMAIN_CTL_CLR;
    uint8_t RESERVED9[0xD8];
    FWK_R uint64_t ERR_SIG_VAL_63_0;
    FWK_R uint64_t ERR_SIG_VAL_127_64;
    FWK_R uint64_t ERR_SIG_VAL_191_128;
    uint8_t RESERVED10[0x8];
    FWK_R uint64_t ERR_TYPE_31_0;
    FWK_R uint64_t ERR_TYPE_63_32;
    FWK_R uint64_t ERR_TYPE_95_64;
    uint8_t RESERVED11[0x8];
    FWK_R uint64_t ERR_TYPE_159_128;
    uint8_t RESERVED12[0xC88];
    FWK_R uint64_t PERIPH_ID_4_PERIPH_ID_5;
    FWK_R uint64_t PERIPH_ID_6_PERIPH_ID_7;
    FWK_R uint64_t PERIPH_ID_0_PERIPH_ID_1;
    FWK_R uint64_t PERIPH_ID_2_PERIPH_ID_3;
    FWK_R uint64_t COMPONENT_ID_0_COMPONENT_ID_1;
    FWK_R uint64_t COMPONENT_ID_2_COMPONENT_ID_3;
    uint8_t RESERVED13[0xEF00];
    FWK_R uint64_t OLY_MN_OLY_ID;
    uint8_t RESERVED14[0xF8];
} ccn5xx_mn_reg_t;

/*
 * CCN5xx crosspoint (XP) registers
 */
typedef struct {
    FWK_RW uint64_t XP_ROUTING_CONTROL;
    FWK_RW uint64_t DEV0_NSM_ROUTING_VECTOR;
    FWK_RW uint64_t DEV1_NSM_ROUTING_VECTOR;
    uint8_t RESERVED0[0xF8];
    FWK_RW uint64_t DEV0_QOS_CONTROL;
    FWK_RW uint64_t DEV0_QOS_LAT_TGT;
    FWK_RW uint64_t DEV0_QOS_LAT_SCALE;
    FWK_RW uint64_t DEV0_QOS_LAT_RANGE;
    uint8_t RESERVED1[0xE0];
    FWK_RW uint64_t DEV1_QOS_CONTROL;
    FWK_RW uint64_t DEV1_QOS_LAT_TGT;
    FWK_RW uint64_t DEV1_QOS_LAT_SCALE;
    FWK_RW uint64_t DEV1_QOS_LAT_RANGE;
    uint8_t RESERVED2[0xD0];
    FWK_RW uint64_t DT_CONFIG;
    FWK_RW uint64_t DT_INTERFACE_SEL;
    FWK_RW uint64_t DT_CMP_VAL0_L;
    FWK_RW uint64_t DT_CMP_VAL0_H;
    FWK_RW uint64_t DT_CMP_MASK0_L;
    FWK_RW uint64_t DT_CMP_MASK0_H;
    uint8_t RESERVED3[0x20];
    FWK_RW uint64_t DT_CMP_VAL1_L;
    FWK_RW uint64_t DT_CMP_VAL1_H;
    FWK_RW uint64_t DT_CMP_MASK1_L;
    FWK_RW uint64_t DT_CMP_MASK1_H;
    FWK_RW uint64_t DT_CONTROL;
    FWK_RW uint64_t DT_STATUS;
    FWK_W uint64_t DT_STATUS_CLR;
    uint8_t RESERVED4[0x78];
    FWK_R uint64_t ERR_SYNDROME_REG0;
    uint8_t RESERVED5[0x78];
    FWK_W uint64_t ERR_SYNDROME_CLR;
    uint8_t RESERVED6[0x78];
    FWK_RW uint64_t AUX_CTL;
    uint8_t RESERVED7[0xF8];
    FWK_RW uint64_t PMU_EVENT_SEL;
    uint8_t RESERVED8[0xF8F8];
    FWK_R uint64_t OLY_XP_OLY_ID;
    uint8_t RESERVED9[0xF8];
} ccn5xx_xp_reg_t;

/*
 * CCN5xx fully coherent home node (HN-F) registers
 */
typedef struct {
    FWK_RW uint64_t HNF_CFG_CTRL;
    FWK_RW uint64_t HNF_SAM_CONTROL;
    FWK_W uint64_t HN_CFG_PSTATE_REQ;
    FWK_R uint64_t HN_CFG_PSTATE_STATUS;
    FWK_R uint64_t QOS_BAND;
    FWK_RW uint64_t QOS_RESERVATION;
    FWK_RW uint64_t RN_STARVATION;
    FWK_RW uint64_t HNF_ERR_INJ;
    FWK_RW uint64_t HNF_L3_LOCK_WAYS;
    FWK_RW uint64_t HNF_L3_LOCK_BASE0;
    FWK_RW uint64_t HNF_L3_LOCK_BASE1;
    FWK_RW uint64_t HNF_L3_LOCK_BASE2;
    FWK_RW uint64_t HNF_L3_LOCK_BASE3;
    uint8_t RESERVED0[0xA0];
    FWK_RW uint64_t HN_CFG_RNI_VEC;
    uint8_t RESERVED1[0xF0];
    FWK_R uint64_t SNOOP_DOMAIN_CTL;
    uint8_t RESERVED2[0x8];
    FWK_W uint64_t SNOOP_DOMAIN_CTL_SET;
    uint8_t RESERVED3[0x8];
    FWK_W uint64_t SNOOP_DOMAIN_CTL_CLR;
    uint8_t RESERVED4[0xD8];
    FWK_W uint64_t HN_CFG_L3SF_DBGRD;
    FWK_R uint64_t L3_CACHE_ACCESS_L3_TAG;
    FWK_R uint64_t L3_CACHE_ACCESS_L3_DATA;
    FWK_R uint64_t L3_CACHE_ACCESS_SF_TAG;
    uint8_t RESERVED5[0xE0];
    FWK_R uint64_t ERR_SYNDROME_REG0;
    FWK_R uint64_t ERR_SYNDROME_REG1;
    uint8_t RESERVED6[0x70];
    FWK_W uint64_t ERR_SYNDROME_CLR;
    uint8_t RESERVED7[0x78];
    FWK_RW uint64_t HNF_AUX_CTL;
    uint8_t RESERVED8[0xF8];
    FWK_RW uint64_t PMU_EVENT_SEL;
    uint8_t RESERVED9[0xF8F8];
    FWK_R uint64_t OLY_HNF_MISC_OLY_ID;
    uint8_t RESERVED10[0xF8];
} ccn5xx_hnf_reg_t;

/*
 * CCN5xx I/O home node (HN-I) registers
 */
typedef struct {
    FWK_RW uint64_t POS_CONTROL;
    FWK_RW uint64_t PCIERC_RNI_NODEID_LIST;
    uint8_t RESERVED0[0x3F0];
    FWK_R uint64_t ERR_SYNDROME_REG0;
    FWK_R uint64_t ERR_SYNDROME_REG1;
    uint8_t RESERVED1[0x70];
    FWK_W uint64_t ERR_SYNDROME_CLR;
    uint8_t RESERVED2[0x78];
    FWK_RW uint64_t SA_AUX_CTL;
    uint8_t RESERVED3[0xF9F8];
    FWK_R uint64_t OLY_HNI_OLY_ID;
    uint8_t RESERVED4[0xF8];
} ccn5xx_hni_reg_t;

/*
 * CCN5xx debug event module registers
 */
typedef struct {
    FWK_RW uint64_t ACTIVE_DSM;
    FWK_RW uint64_t TRIGGER_CTL;
    FWK_RW uint64_t TRIGGER_STATUS;
    FWK_W uint64_t TRIGGER_STATUS_CLR;
    FWK_RW uint64_t TIMER_VAL;
    FWK_RW int64_t DT_CTL;
    uint8_t RESERVED0[0x50];
    FWK_RW uint64_t DBG_ID;
    uint8_t RESERVED1[0x78];
    FWK_RW uint64_t PMEVCNT0;
    FWK_RW uint64_t PMEVCNT1;
    FWK_RW uint64_t PMEVCNT2;
    FWK_RW uint64_t PMEVCNT3;
    FWK_RW uint64_t PMEVCNT4;
    FWK_RW uint64_t PMEVCNT5;
    FWK_RW uint64_t PMEVCNT6;
    FWK_RW uint64_t PMEVCNT7;
    FWK_RW uint64_t PMCCNTR;
    uint8_t RESERVED2[0x8];
    FWK_RW uint64_t PMEVCNTSR0;
    FWK_RW uint64_t PMEVCNTSR1;
    FWK_RW uint64_t PMEVCNTSR2;
    FWK_RW uint64_t PMEVCNTSR3;
    FWK_RW uint64_t PMEVCNTSR4;
    FWK_RW uint64_t PMEVCNTSR5;
    FWK_RW uint64_t PMEVCNTSR6;
    FWK_RW uint64_t PMEVCNTSR7;
    FWK_RW uint64_t PMCCNTRSR;
    FWK_R uint64_t PMOVSR;
    FWK_RW uint64_t PMOVSR_CLR;
    FWK_RW uint64_t PMCR;
    FWK_R uint64_t PMSR;
    FWK_W uint64_t PMSR_REQ;
    FWK_W uint64_t PMSR_CLR;
    uint8_t RESERVED3[0xFD38];
    FWK_R uint64_t OLY_MN_DT_OLY_ID;
    uint8_t RESERVED4[0xF8];
} ccn5xx_dbg_reg_t;

/*
 * CCN5xx I/O coherent requesting node bridge (RN-I) registers
 */
typedef struct {
    uint8_t RESERVED0[0x8];
    FWK_RW uint64_t S0_PORT_CONTROL;
    FWK_RW uint64_t S0_QOS_CONTROL;
    FWK_RW uint64_t S0_QOS_LAT_TGT;
    FWK_RW uint64_t S0_QOS_LAT_SCALE;
    FWK_RW uint64_t S0_QOS_LAT_RANGE;
    uint8_t RESERVED1[0xD8];
    FWK_RW uint64_t S1_PORT_CONTROL;
    FWK_RW uint64_t S1_QOS_CONTROL;
    FWK_RW uint64_t S1_QOS_LAT_TGT;
    FWK_RW uint64_t S1_QOS_LAT_SCALE;
    FWK_RW uint64_t S1_QOS_LAT_RANGE;
    uint8_t RESERVED2[0xD8];
    FWK_RW uint64_t S2_PORT_CONTROL;
    FWK_RW uint64_t S2_QOS_CONTROL;
    FWK_RW uint64_t S2_QOS_LAT_TGT;
    FWK_RW uint64_t S2_QOS_LAT_SCALE;
    FWK_RW uint64_t S2_QOS_LAT_RANGE;
    uint8_t RESERVED3[0x2D0];
    FWK_RW uint64_t AUX_CTL;
    uint8_t RESERVED4[0xF8];
    FWK_RW uint64_t PMU_EVENT_SEL;
    uint8_t RESERVED5[0xF8F8];
    FWK_R uint64_t OLY_RNI_OLY_ID;
    uint8_t RESERVED6[0xF8];
} ccn5xx_rni_reg_t;

/*
 * CCN5xx CHI to AXI bridge (SBSX) registers
 */
typedef struct {
    uint8_t RESERVED0[0x500];
    FWK_RW uint64_t SA_AUX_CTL;
    uint8_t RESERVED1[0xF9F8];
    FWK_R uint64_t OLY_SBSX_OLY_ID;
    uint8_t RESERVED2[0xF8];
} ccn5xx_sbsx_reg_t;

/*
 * CCN5xx unused region structure, used as a placeholder
 */
typedef struct {
    uint8_t RESERVED0[0xFF00];
    FWK_R uint64_t OLY_REGION_OLY_ID;
    uint8_t RESERVED1[0xF8];
} ccn5xx_region_t;

#endif /* INTERNAL_CCN512_H */
