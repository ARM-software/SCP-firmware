/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     DMC-BING module.
 */

#ifndef MOD_DMC_BING_H
#define MOD_DMC_BING_H

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \addtogroup GroupDMC_BING DMC-BING Driver
 *
 * \details Please consult the Arm CoreLink DMC-BING Dynamic Memory Controller
 *      Technical Reference Manual for details on the specific registers that
 *      are programmed here.
 *
 * \sa https://developer.arm.com/docs/100568/latest/programmers-model/register-summary
 * \{
 */

/*!
 * \brief Number of access addresses
 */
#define MOD_DMC_BING_ACCESS_ADDRESS_COUNT 8

/*!
 * \brief Access address next registers
 */
struct mod_dmc_bing_access_address_next {
    /*!
     * \cond
     */
    FWK_RW uint32_t MIN_31_00;
    FWK_RW uint32_t MIN_43_32;
    FWK_RW uint32_t MAX_31_00;
    FWK_RW uint32_t MAX_43_32;
    /*!
     * \endcond
     */
};

/*!
 * \brief Access address next registers
 */
struct mod_dmc_bing_access_address_now {
    /*!
     * \cond
     */
    FWK_R uint32_t MIN_31_00;
    FWK_R uint32_t MIN_43_32;
    FWK_R uint32_t MAX_31_00;
    FWK_R uint32_t MAX_43_32;
    /*!
     * \endcond
     */
};

/*!
 * \brief PMU payload information operation register
 */
struct mod_dmc_bing_pmu_counter {
    /*!
     * \cond
     */
    FWK_RW uint32_t MASK_31_00;
    FWK_RW uint32_t MASK_63_32;
    FWK_RW uint32_t MATCH_31_00;
    FWK_RW uint32_t MATCH_63_32;
    FWK_RW uint32_t CONTROL;
    uint32_t RESERVED0;
    FWK_R uint32_t SNAPSHOT_VALUE_31_00;
    uint32_t RESERVED1;
    FWK_RW uint32_t VALUE_31_00;
    uint32_t RESERVED2;
    /*!
     * \endcond
     */
};

/*!
 * \brief DMC-BING register definitions
 */
struct mod_dmc_bing_reg {
    /*!
     * \cond
     */
    FWK_R uint32_t MEMC_STATUS;
    FWK_R uint32_t MEMC_CONFIG;
    FWK_W uint32_t MEMC_CMD;
    uint32_t RESERVED1;
    FWK_RW uint32_t ADDRESS_CONTROL_NEXT;
    FWK_RW uint32_t DECODE_CONTROL_NEXT;
    FWK_RW uint32_t FORMAT_CONTROL;
    FWK_RW uint32_t ADDRESS_MAP_NEXT;
    FWK_RW uint32_t LOW_POWER_CONTROL_NEXT;
    uint32_t RESERVED2;
    FWK_RW uint32_t TURNAROUND_CONTROL_NEXT;
    FWK_RW uint32_t HIT_TURNAROUND_CONTROL_NEXT;
    FWK_RW uint32_t QOS_CLASS_CONTROL_NEXT;
    FWK_RW uint32_t ESCALATION_CONTROL_NEXT;
    FWK_RW uint32_t QV_CONTROL_31_00_NEXT;
    FWK_RW uint32_t QV_CONTROL_63_32_NEXT;
    FWK_RW uint32_t RT_CONTROL_31_00_NEXT;
    FWK_RW uint32_t RT_CONTROL_63_32_NEXT;
    FWK_RW uint32_t TIMEOUT_CONTROL_NEXT;
    FWK_RW uint32_t CREDIT_CONTROL_NEXT;
    FWK_RW uint32_t WRITE_PRIORITY_CONTROL_31_00_NEXT;
    FWK_RW uint32_t WRITE_PRIORITY_CONTROL_63_32_NEXT;
    FWK_RW uint32_t QUEUE_THRESHOLD_CONTROL_31_00_NEXT;
    FWK_RW uint32_t QUEUE_THRESHOLD_CONTROL_63_32_NEXT;
    FWK_RW uint32_t ADDRESS_SHUTTER_31_00_NEXT;
    FWK_RW uint32_t ADDRESS_SHUTTER_63_32_NEXT;
    FWK_RW uint32_t ADDRESS_SHUTTER_95_64_NEXT;
    FWK_RW uint32_t ADDRESS_SHUTTER_127_96_NEXT;
    FWK_RW uint32_t ADDRESS_SHUTTER_159_128_NEXT;
    FWK_RW uint32_t ADDRESS_SHUTTER_191_160_NEXT;
    FWK_RW uint32_t MEMORY_ADDRESS_MAX_31_00_NEXT;
    FWK_RW uint32_t MEMORY_ADDRESS_MAX_43_32_NEXT;
    struct mod_dmc_bing_access_address_next
        ACCESS_ADDRESS_NEXT[MOD_DMC_BING_ACCESS_ADDRESS_COUNT];
    FWK_R uint32_t CHANNEL_STATUS;
    FWK_R uint32_t CHANNEL_STATUS_63_32;
    FWK_RW uint32_t DIRECT_ADDR;
    FWK_W uint32_t DIRECT_CMD;
    FWK_RW uint32_t DCI_REPLAY_TYPE_NEXT;
    FWK_RW uint32_t DIRECT_CONTROL_NEXT;
    FWK_RW uint32_t DCI_STRB;
    FWK_RW uint32_t DCI_DATA;
    FWK_RW uint32_t REFRESH_CONTROL_NEXT;
    uint32_t RESERVED3;
    FWK_RW uint32_t MEMORY_TYPE_NEXT;
    uint32_t RESERVED4;
    FWK_RW uint32_t FEATURE_CONFIG;
    uint32_t RESERVED5;
    FWK_RW uint32_t NIBBLE_FAILED_031_000;
    FWK_RW uint32_t NIBBLE_FAILED_063_032;
    FWK_RW uint32_t NIBBLE_FAILED_095_064;
    FWK_RW uint32_t NIBBLE_FAILED_127_096;
    FWK_RW uint32_t QUEUE_ALLOCATE_CONTROL_031_000;
    FWK_RW uint32_t QUEUE_ALLOCATE_CONTROL_063_032;
    FWK_RW uint32_t QUEUE_ALLOCATE_CONTROL_095_064;
    FWK_RW uint32_t QUEUE_ALLOCATE_CONTROL_127_096;
    uint8_t RESERVED6[0x16C - 0x158];
    FWK_RW uint32_t LINK_ERR_COUNT;
    FWK_RW uint32_t SCRUB_CONTROL0_NEXT;
    FWK_RW uint32_t SCRUB_ADDRESS_MIN0_NEXT;
    FWK_RW uint32_t SCRUB_ADDRESS_MAX0_NEXT;
    FWK_R uint32_t SCRUB_ADDRESS_CURRENT0;
    FWK_RW uint32_t SCRUB_CONTROL1_NEXT;
    FWK_RW uint32_t SCRUB_ADDRESS_MIN1_NEXT;
    FWK_RW uint32_t SCRUB_ADDRESS_MAX1_NEXT;
    FWK_R uint32_t SCRUB_ADDRESS_CURRENT1;
    uint8_t RESERVED7[0x1A0 - 0x190];
    FWK_RW uint32_t CS_REMAP_CONTROL_31_00_NEXT;
    FWK_RW uint32_t CS_REMAP_CONTROL_63_32_NEXT;
    FWK_RW uint32_t CS_REMAP_CONTROL_95_64_NEXT;
    FWK_RW uint32_t CS_REMAP_CONTROL_127_96_NEXT;
    FWK_RW uint32_t CID_REMAP_CONTROL_31_00_NEXT;
    FWK_RW uint32_t CID_REMAP_CONTROL_63_32_NEXT;
    uint8_t RESERVED8[0x1C0 - 0x1B8];
    FWK_RW uint32_t CKE_REMAP_CONTROL_NEXT;
    FWK_RW uint32_t RST_REMAP_CONTROL_NEXT;
    FWK_RW uint32_t CK_REMAP_CONTROL_NEXT;
    uint32_t RESERVED9;
    FWK_RW uint32_t POWER_GROUP_CONTROL_31_00_NEXT;
    FWK_RW uint32_t POWER_GROUP_CONTROL_63_32_NEXT;
    FWK_RW uint32_t POWER_GROUP_CONTROL_95_64_NEXT;
    FWK_RW uint32_t POWER_GROUP_CONTROL_127_96_NEXT;
    FWK_RW uint32_t PHY_RDWRDATA_CS_MASK_31_00;
    FWK_RW uint32_t PHY_RDWRDATA_CS_MASK_63_32;
    FWK_RW uint32_t PHY_REQUEST_CS_REMAP;
    uint32_t RESERVED10;
    FWK_RW uint32_t FEATURE_CONTROL_NEXT;
    FWK_RW uint32_t MUX_CONTROL_NEXT;
    FWK_RW uint32_t RANK_REMAP_CONTROL_NEXT;
    uint32_t RESERVED11;
    FWK_RW uint32_t T_REFI_NEXT;
    FWK_RW uint32_t T_RFC_NEXT;
    FWK_RW uint32_t T_MRR_NEXT;
    FWK_RW uint32_t T_MRW_NEXT;
    FWK_RW uint32_t REFRESH_ENABLE_NEXT;
    uint32_t RESERVED12;
    FWK_RW uint32_t T_RCD_NEXT;
    FWK_RW uint32_t T_RAS_NEXT;
    FWK_RW uint32_t T_RP_NEXT;
    FWK_RW uint32_t T_RPALL_NEXT;
    FWK_RW uint32_t T_RRD_NEXT;
    FWK_RW uint32_t T_ACT_WINDOW_NEXT;
    uint32_t RESERVED13;
    FWK_RW uint32_t T_RTR_NEXT;
    FWK_RW uint32_t T_RTW_NEXT;
    FWK_RW uint32_t T_RTP_NEXT;
    uint32_t RESERVED14;
    FWK_RW uint32_t T_WR_NEXT;
    FWK_RW uint32_t T_WTR_NEXT;
    FWK_RW uint32_t T_WTW_NEXT;
    FWK_RW uint32_t T_CLOCK_CONTROL_NEXT;
    FWK_RW uint32_t T_XMPD_NEXT;
    FWK_RW uint32_t T_EP_NEXT;
    FWK_RW uint32_t T_XP_NEXT;
    FWK_RW uint32_t T_ESR_NEXT;
    FWK_RW uint32_t T_XSR_NEXT;
    FWK_RW uint32_t T_ESRCK_NEXT;
    FWK_RW uint32_t T_CKXSR_NEXT;
    FWK_RW uint32_t T_CMD_NEXT;
    FWK_RW uint32_t T_PARITY_NEXT;
    FWK_RW uint32_t T_ZQCS_NEXT;
    FWK_RW uint32_t T_RW_ODT_CLR_NEXT;
    uint8_t RESERVED16[0x300 - 0x280];
    FWK_RW uint32_t T_RDDATA_EN_NEXT;
    FWK_RW uint32_t T_PHYRDLAT_NEXT;
    FWK_RW uint32_t T_PHYWRLAT_NEXT;
    uint32_t RESERVED17;
    FWK_RW uint32_t RDLVL_CONTROL_NEXT;
    FWK_RW uint32_t RDLVL_MRS_NEXT;
    FWK_RW uint32_t T_RDLVL_EN_NEXT;
    FWK_RW uint32_t T_RDLVL_RR_NEXT;
    FWK_RW uint32_t WRLVL_CONTROL_NEXT;
    FWK_RW uint32_t WRLVL_MRS_NEXT;
    FWK_RW uint32_t T_WRLVL_EN_NEXT;
    FWK_RW uint32_t T_WRLVL_WW_NEXT;
    uint32_t RESERVED18;
    FWK_R uint32_t TRAINING_WRLVL_SLICE_STATUS;
    FWK_R uint32_t TRAINING_RDLVL_SLICE_STATUS;
    FWK_R uint32_t TRAINING_RDLVL_GATE_SLICE_STATUS;
    FWK_R uint32_t TRAINING_WDQLVL_SLICE_STATUS;
    FWK_R uint32_t TRAINING_WDQLVL_SLICE_RESULT;
    FWK_RW uint32_t PHY_POWER_CONTROL_NEXT;
    FWK_RW uint32_t T_LPRESP_NEXT;
    FWK_RW uint32_t PHY_UPDATE_CONTROL_NEXT;
    FWK_RW uint32_t T_ODTH_NEXT;
    FWK_RW uint32_t ODT_TIMING_NEXT;
    uint32_t RESERVED19;
    FWK_RW uint32_t ODT_WR_CONTROL_31_00_NEXT;
    FWK_RW uint32_t ODT_WR_CONTROL_63_32_NEXT;
    FWK_RW uint32_t ODT_RD_CONTROL_31_00_NEXT;
    FWK_RW uint32_t ODT_RD_CONTROL_63_32_NEXT;
    FWK_R uint32_t TEMPERATURE_READOUT;
    uint32_t RESERVED20;
    FWK_R uint32_t TRAINING_STATUS;
    FWK_R uint32_t TRAINING_STATUS_63_32;
    FWK_RW uint32_t DQ_MAP_CONTROL_15_00_NEXT;
    FWK_RW uint32_t DQ_MAP_CONTROL_31_16_NEXT;
    FWK_RW uint32_t DQ_MAP_CONTROL_47_32_NEXT;
    FWK_RW uint32_t DQ_MAP_CONTROL_63_48_NEXT;
    FWK_RW uint32_t DQ_MAP_CONTROL_71_64_NEXT;
    uint32_t RESERVED21;
    FWK_R uint32_t RANK_STATUS;
    FWK_R uint32_t MODE_CHANGE_STATUS;
    uint8_t RESERVED22[0x3B0 - 0x3A0];
    FWK_RW uint32_t ODT_CP_CONTROL_31_00_NEXT;
    FWK_RW uint32_t ODT_CP_CONTROL_63_32_NEXT;
    uint8_t RESERVED23[0x400 - 0x3B8];
    FWK_R uint32_t USER_STATUS;
    uint32_t RESERVED24;
    FWK_RW uint32_t USER_CONFIG0_NEXT;
    FWK_RW uint32_t USER_CONFIG1_NEXT;
    FWK_RW uint32_t USER_CONFIG2;
    FWK_RW uint32_t USER_CONFIG3;
    uint8_t RESERVED25[0x500 - 0x418];
    FWK_RW uint32_t INTERRUPT_CONTROL;
    uint32_t RESERVED26;
    FWK_W uint32_t INTERRUPT_CLR;
    uint32_t RESERVED27;
    FWK_R uint32_t INTERRUPT_STATUS;
    uint8_t RESERVED28[0x538 - 0x514];
    FWK_R uint32_t FAILED_ACCESS_INT_INFO_31_00;
    FWK_R uint32_t FAILED_ACCESS_INT_INFO_63_32;
    FWK_R uint32_t FAILED_PROG_INT_INFO_31_00;
    FWK_R uint32_t FAILED_PROG_INT_INFO_63_32;
    FWK_R uint32_t LINK_ERR_INT_INFO_31_00;
    FWK_R uint32_t LINK_ERR_INT_INFO_63_32;
    FWK_R uint32_t ARCH_FSM_INT_INFO_31_00;
    FWK_R uint32_t ARCH_FSM_INT_INFO_63_32;
    uint8_t RESERVED29[0x610 - 0x558];
    FWK_RW uint32_t T_DB_TRAIN_RESP_NEXT;
    FWK_RW uint32_t T_LVL_DISCONNECT_NEXT;
    uint8_t RESERVED30[0x620 - 0x618];
    FWK_RW uint32_t WDQLVL_CONTROL_NEXT;
    FWK_RW uint32_t WDQLVL_VREFDQ_TRAIN_MRS_NEXT;
    FWK_RW uint32_t WDQLVL_ADDRESS_31_00_NEXT;
    FWK_RW uint32_t WDQLVL_ADDRESS_63_32_NEXT;
    FWK_RW uint32_t T_WDQLVL_EN_NEXT;
    FWK_RW uint32_t T_WDQLVL_WW_NEXT;
    FWK_RW uint32_t T_WDQLVL_RW_NEXT;
    FWK_R uint32_t TRAINING_WDQLVL_SLICE_RESP;
    FWK_R uint32_t TRAINING_RDLVL_SLICE_RESP;
    uint8_t RESERVED31[0x654 - 0x644];
    FWK_RW uint32_t PHYMSTR_CONTROL_NEXT;
    uint8_t RESERVED32[0x700 - 0x658];
    FWK_R uint32_t ERR0FR;
    uint32_t RESERVED33;
    FWK_RW uint32_t ERR0CTLR0;
    FWK_RW uint32_t ERR0CTLR1;
    FWK_RW uint32_t ERR0STATUS;
    uint8_t RESERVED34[0x740 - 0x714];
    FWK_R uint32_t ERR1FR;
    uint32_t RESERVED35;
    FWK_R uint32_t ERR1CTLR;
    uint32_t RESERVED36;
    FWK_RW uint32_t ERR1STATUS;
    uint32_t RESERVED37;
    FWK_RW uint32_t ERR1ADDR0;
    FWK_RW uint32_t ERR1ADDR1;
    FWK_RW uint32_t ERR1MISC0;
    FWK_RW uint32_t ERR1MISC1;
    FWK_RW uint32_t ERR1MISC2;
    FWK_RW uint32_t ERR1MISC3;
    FWK_RW uint32_t ERR1MISC4;
    FWK_RW uint32_t ERR1MISC5;
    uint8_t RESERVED38[0x780 - 0x778];
    FWK_R uint32_t ERR2FR;
    uint32_t RESERVED39;
    FWK_R uint32_t ERR2CTLR;
    uint32_t RESERVED40;
    FWK_RW uint32_t ERR2STATUS;
    uint32_t RESERVED41;
    FWK_RW uint32_t ERR2ADDR0;
    FWK_RW uint32_t ERR2ADDR1;
    FWK_RW uint32_t ERR2MISC0;
    FWK_RW uint32_t ERR2MISC1;
    FWK_RW uint32_t ERR2MISC2;
    FWK_RW uint32_t ERR2MISC3;
    FWK_RW uint32_t ERR2MISC4;
    FWK_RW uint32_t ERR2MISC5;
    uint8_t RESERVED42[0x7C0 - 0x7B8];
    FWK_R uint32_t ERR3FR;
    uint32_t RESERVED43;
    FWK_R uint32_t ERR3CTLR;
    uint32_t RESERVED44;
    FWK_RW uint32_t ERR3STATUS;
    uint32_t RESERVED45;
    FWK_RW uint32_t ERR3ADDR0;
    FWK_RW uint32_t ERR3ADDR1;
    FWK_R uint32_t ERR3MISC0;
    FWK_R uint32_t ERR3MISC1;
    uint8_t RESERVED46[0x800 - 0x7E8];
    FWK_R uint32_t ERR4FR;
    uint32_t RESERVED47;
    FWK_R uint32_t ERR4CTLR;
    uint32_t RESERVED48;
    FWK_RW uint32_t ERR4STATUS;
    uint32_t RESERVED49;
    FWK_RW uint32_t ERR4ADDR0;
    FWK_RW uint32_t ERR4ADDR1;
    FWK_RW uint32_t ERR4MISC0;
    FWK_RW uint32_t ERR4MISC1;
    FWK_RW uint32_t ERR4MISC2;
    uint8_t RESERVED50[0x840 - 0x82C];
    FWK_R uint32_t ERR5FR;
    uint32_t RESERVED51;
    FWK_R uint32_t ERR5CTLR;
    uint32_t RESERVED52;
    FWK_RW uint32_t ERR5STATUS;
    uint32_t RESERVED53;
    FWK_RW uint32_t ERR5ADDR0;
    FWK_RW uint32_t ERR5ADDR1;
    FWK_RW uint32_t ERR5MISC0;
    FWK_RW uint32_t ERR5MISC1;
    FWK_RW uint32_t ERR5MISC2;
    uint8_t RESERVED54[0x880 - 0x86C];
    FWK_R uint32_t ERR6FR;
    uint32_t RESERVED55;
    FWK_R uint32_t ERR6CTLR;
    uint32_t RESERVED56;
    FWK_RW uint32_t ERR6STATUS;
    uint32_t RESERVED57;
    FWK_RW uint32_t ERR6ADDR0;
    FWK_RW uint32_t ERR6ADDR1;
    FWK_RW uint32_t ERR6MISC0;
    FWK_RW uint32_t ERR6MISC1;
    uint8_t RESERVED58[0x920 - 0x8A8];
    FWK_RW uint32_t ERRGSR;
    uint8_t RESERVED59[0xA00 - 0x924];
    FWK_W uint32_t PMU_SNAPSHOT_REQ;
    FWK_R uint32_t PMU_SNAPSHOT_ACK;
    FWK_RW uint32_t PMU_OVERFLOW_STATUS_CLKDIV2;
    FWK_RW uint32_t PMU_OVERFLOW_STATUS_CLK;
    struct mod_dmc_bing_pmu_counter PMC_CLKDIV2_COUNT[8];
    struct mod_dmc_bing_pmu_counter PMC_CLK_COUNT[2];
    uint8_t RESERVED60[0xD00 - 0xBA0];
    FWK_RW uint32_t CAPABILITY_CTRL;
    FWK_W uint32_t TAG_CACHE_CTRL;
    FWK_RW uint32_t TAG_CACHE_CFG;
    FWK_RW uint32_t MEMORY_ACCESS_CTRL;
    FWK_RW uint32_t MEMORY_ADDRESS_CTRL;
    FWK_RW uint32_t MEMORY_ADDRESS_CTRL2;
    FWK_RW uint32_t BING_SPL_CTRL_REG;
    uint8_t RESERVED90[0xE00 - 0xD1C];
    FWK_RW uint32_t INTEG_CFG;
    uint32_t RESERVED61;
    FWK_W uint32_t INTEG_OUTPUTS;
    uint8_t RESERVED62[0x1010 - 0xE0C];
    FWK_R uint32_t ADDRESS_CONTROL_NOW;
    FWK_R uint32_t DECODE_CONTROL_NOW;
    uint32_t RESERVED63;
    FWK_R uint32_t ADDRESS_MAP_NOW;
    FWK_R uint32_t LOW_POWER_CONTROL_NOW;
    uint32_t RESERVED64;
    FWK_R uint32_t TURNAROUND_CONTROL_NOW;
    FWK_R uint32_t HIT_TURNAROUND_CONTROL_NOW;
    FWK_R uint32_t QOS_CLASS_CONTROL_NOW;
    FWK_R uint32_t ESCALATION_CONTROL_NOW;
    FWK_R uint32_t QV_CONTROL_31_00_NOW;
    FWK_R uint32_t QV_CONTROL_63_32_NOW;
    FWK_R uint32_t RT_CONTROL_31_00_NOW;
    FWK_R uint32_t RT_CONTROL_63_32_NOW;
    FWK_R uint32_t TIMEOUT_CONTROL_NOW;
    FWK_R uint32_t CREDIT_CONTROL_NOW;
    FWK_R uint32_t WRITE_PRIORITY_CONTROL_31_00_NOW;
    FWK_R uint32_t WRITE_PRIORITY_CONTROL_63_32_NOW;
    FWK_R uint32_t QUEUE_THRESHOLD_CONTROL_31_00_NOW;
    FWK_R uint32_t QUEUE_THRESHOLD_CONTROL_63_32_NOW;
    FWK_R uint32_t ADDRESS_SHUTTER_31_00_NOW;
    FWK_R uint32_t ADDRESS_SHUTTER_63_32_NOW;
    FWK_R uint32_t ADDRESS_SHUTTER_95_64_NOW;
    FWK_R uint32_t ADDRESS_SHUTTER_127_96_NOW;
    FWK_R uint32_t ADDRESS_SHUTTER_159_128_NOW;
    FWK_R uint32_t ADDRESS_SHUTTER_191_160_NOW;
    FWK_R uint32_t MEMORY_ADDRESS_MAX_31_00_NOW;
    FWK_R uint32_t MEMORY_ADDRESS_MAX_43_32_NOW;
    struct mod_dmc_bing_access_address_now
        ACCESS_ADDRESS_NOW[MOD_DMC_BING_ACCESS_ADDRESS_COUNT];
    uint8_t RESERVED65[0x1110 - 0x1100];
    FWK_R uint32_t DCI_REPLAY_TYPE_NOW;
    FWK_R uint32_t DIRECT_CONTROL_NOW;
    uint8_t RESERVED66[0x1120 - 0x1118];
    FWK_R uint32_t REFRESH_CONTROL_NOW;
    uint32_t RESERVED67;
    FWK_R uint32_t MEMORY_TYPE_NOW;
    uint8_t RESERVED68[0x1170 - 0x112C];
    FWK_R uint32_t SCRUB_CONTROL0_NOW;
    FWK_R uint32_t SCRUB_ADDRESS_MIN0_NOW;
    FWK_R uint32_t SCRUB_ADDRESS_MAX0_NOW;
    uint32_t RESERVED69;
    FWK_R uint32_t SCRUB_CONTROL1_NOW;
    FWK_R uint32_t SCRUB_ADDRESS_MIN1_NOW;
    FWK_R uint32_t SCRUB_ADDRESS_MAX1_NOW;
    uint8_t RESERVED70[0x11A0 - 0x118C];
    FWK_R uint32_t CS_REMAP_CONTROL_31_00_NOW;
    FWK_R uint32_t CS_REMAP_CONTROL_63_32_NOW;
    FWK_R uint32_t CS_REMAP_CONTROL_95_64_NOW;
    FWK_R uint32_t CS_REMAP_CONTROL_127_96_NOW;
    FWK_R uint32_t CID_REMAP_CONTROL_31_00_NOW;
    FWK_R uint32_t CID_REMAP_CONTROL_63_32_NOW;
    uint8_t RESERVED71[0x11C0 - 0x11B8];
    FWK_R uint32_t CKE_REMAP_CONTROL_31_00_NOW;
    FWK_R uint32_t RST_REMAP_CONTROL_31_00_NOW;
    FWK_R uint32_t CK_REMAP_CONTROL_31_00_NOW;
    FWK_R uint32_t POWER_GROUP_CONTROL_31_00_NOW;
    FWK_R uint32_t POWER_GROUP_CONTROL_63_32_NOW;
    FWK_R uint32_t POWER_GROUP_CONTROL_95_64_NOW;
    FWK_R uint32_t POWER_GROUP_CONTROL_127_96_NOW;
    uint8_t RESERVED72[0x11F0 - 0x11E0];
    FWK_R uint32_t FEATURE_CONTROL_NOW;
    FWK_R uint32_t MUX_CONTROL_NOW;
    FWK_R uint32_t RANK_REMAP_CONTROL_NOW;
    uint32_t RESERVED73;
    FWK_R uint32_t T_REFI_NOW;
    FWK_R uint32_t T_RFC_NOW;
    FWK_R uint32_t T_MRR_NOW;
    FWK_R uint32_t T_MRW_NOW;
    uint8_t RESERVED74[0x1218 - 0x1210];
    FWK_R uint32_t T_RCD_NOW;
    FWK_R uint32_t T_RAS_NOW;
    FWK_R uint32_t T_RP_NOW;
    FWK_R uint32_t T_RPALL_NOW;
    FWK_R uint32_t T_RRD_NOW;
    FWK_R uint32_t T_ACT_WINDOW_NOW;
    uint32_t RESERVED75;
    FWK_R uint32_t T_RTR_NOW;
    FWK_R uint32_t T_RTW_NOW;
    FWK_R uint32_t T_RTP_NOW;
    uint32_t RESERVED76;
    FWK_R uint32_t T_WR_NOW;
    FWK_R uint32_t T_WTR_NOW;
    FWK_R uint32_t T_WTW_NOW;
    uint32_t RESERVED77;
    FWK_R uint32_t T_XMPD_NOW;
    FWK_R uint32_t T_EP_NOW;
    FWK_R uint32_t T_XP_NOW;
    FWK_R uint32_t T_ESR_NOW;
    FWK_R uint32_t T_XSR_NOW;
    FWK_R uint32_t T_ESRCK_NOW;
    FWK_R uint32_t T_CKXSR_NOW;
    FWK_R uint32_t T_CMD_NOW;
    FWK_R uint32_t T_PARITY_NOW;
    FWK_R uint32_t T_ZQCS_NOW;
    FWK_R uint32_t T_RW_ODT_CLR_NOW;
    uint8_t RESERVED78[0x1300 - 0x1280];
    FWK_R uint32_t T_RDDATA_EN_NOW;
    FWK_R uint32_t T_PHYRDLAT_NOW;
    FWK_R uint32_t T_PHYWRLAT_NOW;
    uint32_t RESERVED79;
    FWK_R uint32_t RDLVL_CONTROL_NOW;
    FWK_R uint32_t RDLVL_MRS_NOW;
    FWK_R uint32_t T_RDLVL_EN_NOW;
    FWK_R uint32_t T_RDLVL_RR_NOW;
    FWK_R uint32_t WRLVL_CONTROL_NOW;
    FWK_R uint32_t WRLVL_MRS_NOW;
    FWK_R uint32_t T_WRLVL_EN_NOW;
    FWK_R uint32_t T_WRLVL_WW_NOW;
    uint8_t RESERVED80[0x1348 - 0x1330];
    FWK_R uint32_t PHY_POWER_CONTROL_NOW;
    FWK_R uint32_t T_LPRESP_NOW;
    FWK_R uint32_t PHY_UPDATE_CONTROL_NOW;
    FWK_R uint32_t T_ODTH_NOW;
    FWK_R uint32_t ODT_TIMING_NOW;
    uint32_t RESERVED81;
    FWK_R uint32_t ODT_WR_CONTROL_31_00_NOW;
    FWK_R uint32_t ODT_WR_CONTROL_63_32_NOW;
    FWK_R uint32_t ODT_RD_CONTROL_31_00_NOW;
    FWK_R uint32_t ODT_RD_CONTROL_63_32_NOW;
    uint8_t RESERVED82[0x1380 - 0x1370];
    FWK_R uint32_t DQ_MAP_CONTROL_15_00_NOW;
    FWK_R uint32_t DQ_MAP_CONTROL_31_16_NOW;
    FWK_R uint32_t DQ_MAP_CONTROL_47_32_NOW;
    FWK_R uint32_t DQ_MAP_CONTROL_63_48_NOW;
    FWK_R uint32_t DQ_MAP_CONTROL_71_64_NOW;
    uint8_t RESERVED83[0x13B0 - 0x1394];
    FWK_R uint32_t ODT_CP_CONTROL_31_00_NOW;
    FWK_R uint32_t ODT_CP_CONTROL_63_32_NOW;
    uint8_t RESERVED84[0x1408 - 0x13B8];
    FWK_R uint32_t USER_CONFIG0_NOW;
    FWK_R uint32_t USER_CONFIG1_NOW;
    uint8_t RESERVED85[0x1610 - 0x1410];
    FWK_R uint32_t T_DB_TRAIN_RESP_NOW;
    FWK_R uint32_t T_LVL_DISCONNECT_NOW;
    uint8_t RESERVED86[0x1620 - 0x1618];
    FWK_R uint32_t WDQLVL_CONTROL_NOW;
    FWK_R uint32_t WDQLVL_VREFDQ_TRAIN_MRS_NOW;
    FWK_R uint32_t WDQLVL_ADDRESS_31_00_NOW;
    FWK_R uint32_t WDQLVL_ADDRESS_63_32_NOW;
    FWK_R uint32_t T_WDQLVL_EN_NOW;
    FWK_R uint32_t T_WDQLVL_WW_NOW;
    FWK_R uint32_t T_WDQLVL_RW_NOW;
    uint8_t RESERVED87[0x1654 - 0x163C];
    FWK_R uint32_t PHYMSTR_CONTROL_NOW;
    uint8_t RESERVED88[0x1FD0 - 0x1658];
    FWK_R uint32_t PERIPH_ID_4;
    uint8_t RESERVED89[0x1FE0 - 0x1FD4];
    FWK_R uint32_t PERIPH_ID_0;
    FWK_R uint32_t PERIPH_ID_1;
    FWK_R uint32_t PERIPH_ID_2;
    FWK_R uint32_t PERIPH_ID_3;
    FWK_R uint32_t COMPONENT_ID_0;
    FWK_R uint32_t COMPONENT_ID_1;
    FWK_R uint32_t COMPONENT_ID_2;
    FWK_R uint32_t COMPONENT_ID_3;
    /*!
     * \endcond
     */
};

/*!
 * \brief Mask to get the current state of DMC
 */
#define MOD_DMC_BING_MEMC_STATUS UINT32_C(0x00000007)

/*!
 * \brief Mask to get the memc_cmd bitfield
 */
#define MOD_DMC_BING_MEMC_CMD UINT32_C(0x00000007)

/*!
 * \brief MEMC command MGR active status bit
 */
#define MOD_DMC_BING_MEMC_STATUS_MGR_ACTIVE UINT32_C(0x00000100)

/*!
 * \brief DMC channel M0 idle status bit
 */
#define MOD_DMC_BING_CHANNEL_STATUS_M0_IDLE UINT32_C(0x00000001)

/*!
 * \brief Enable ECC detection on reads
 */
#define DMC_ERR0CTRL0_ED_ENABLE UINT32_C(0x00000001)

/*!
 * \brief Enable defer on reads
 */
#define DMC_ERR0CTRL0_DE_ENABLE UINT32_C(0x00000002)

/*!
 * \brief Enable uncorrectable error recovery interrupt
 */
#define DMC_ERR0CTRL0_UI_ENABLE UINT32_C(0x00000004)

/*!
 * \brief Enable ECC FHI interrupt
 */
#define DMC_ERR0CTRL0_FI_ENABLE UINT32_C(0x00000008)

/*!
 * \brief Enable CFI interrupt
 */
#define DMC_ERR0CTRL0_CFI_ENABLE UINT32_C(0x00000100)

/*!
 * \brief DMC Bank Hash enable bit in ADDRESS_CONTROL register
 */
#define DMC_ADDR_CTLR_BANK_HASH_ENABLE UINT32_C(0x10000000)

/*!
 * \brief DDR training timeout in microseconds
 */
#define DMC_TRAINING_TIMEOUT UINT32_C(5000)

/*!
 * \brief DDR training command for rank 1
 */
#define DDR_CMD_TRAIN_RANK_1 UINT32_C(0x0001000A)
/*!
 * \brief DDR training command for rank 2
 */
#define DDR_CMD_TRAIN_RANK_2 UINT32_C(0x0002000A)

/*!
 * \brief DDR training data slices position
 */
#define DDR_ADDR_DATA_SLICES_POS 12
/*!
 * \brief Offset for Abort Register
 */
#define DMC_BING_ABORT_REG_OFFSET UINT32_C(0x10000)

/*!
 * \brief Deassert Abort Request
 */
#define DEASSERT_ABORT_REQUEST 0
/*!
 * \brief Assert Abort Request
 */
#define ASSERT_ABORT_REQUEST 1

/*!
 * \brief Element configuration.
 */
struct mod_dmc_bing_element_config {
    /*! Base address of the DMC-BING device's registers */
    uintptr_t dmc_bing_base;
    /*! Base address of the DDR PHY registers */
    uintptr_t ddr_phy_base;
    /*! Identifier of the clock that this element depends on */
    fwk_id_t clock_id;
};

/*!
 * \brief Structure defining the connected DIMM's parameters.
 */
struct dimm_info {
    /*! Current speed at which the DIMMs are configured & trained */
    uint16_t speed;

    /*! Number of ranks in DIMM */
    uint8_t number_of_ranks;

    /*! Number of ranks to train */
    uint8_t ranks_to_train;

    /*! DIMM memory width */
    uint8_t dimm_mem_width;

    /*! CAS Write Latency value */
    uint32_t cwl_value;
};

/*!
 * \brief API to expose DDR memory size.
 */
struct mod_dmc_bing_mem_info_api {
    /*!
     * \brief Pointer to function that gets DDR memory size in bytes.
     *
     * \param size Pointer where memory size will be stored.
     *
     * \retval FWK_SUCCESS if the operation succeed.
     * \return one of the error code otherwise.
     */
    int (*get_mem_size)(uint64_t *size);
};

/*!
 * \brief API indices.
 */
enum mod_dmc_bing_api_idx {
    /*! API index for getting memory information */
    MOD_DDR_API_IDX_MEM_INFO,

    /*! Number of exposed interfaces */
    MOD_DDR_API_COUNT,
};

/*!
 * \brief DMC-BING module configuration.
 */
struct mod_dmc_bing_module_config {
    /*! DDR operating frequency */
    uint16_t ddr_speed;
};

/*!
 * \brief Identifiers of DMC-Bing configuration stages.
 */
enum mod_dmc_bing_config_stage {
    /*! DMC-Bing DIMM training MGR active stage */
    DMC_BING_CONFIG_STAGE_TRAINING_MGR_ACTIVE,

    /*! DMC-Bing DIMM training channel M0 idle stage */
    DMC_BING_CONFIG_STAGE_TRAINING_M0_IDLE,

    /*! DMC-Bing configuration stages */
    DMC_BING_CONFIG_STAGE_COUNT,
};

/*!
 * \brief Structure defining data to be passed to timer API.
 */
struct mod_dmc_bing_wait_condition_data {
    /*! Pointer to DMC-Bing module registers */
    void *dmc;

    /*! DMC-Bing configuration stage identifier */
    enum mod_dmc_bing_config_stage stage;
};

/*!
 * \brief Identifiers of the DMC-Bing Commands.
 */
enum mod_dmc_bing_memc_cmd {
    /*! Command to enter into the CONFIG state */
    MOD_DMC_BING_MEMC_CMD_CONFIG,

    /*! Command to enter into the SLEEP state */
    MOD_DMC_BING_MEMC_CMD_SLEEP,

    /*! Command to enter into the PAUSED state */
    MOD_DMC_BING_MEMC_CMD_PAUSED,

    /*! Command to enter into the READY state */
    MOD_DMC_BING_MEMC_CMD_GO,

    /*! Command to perform direct_cmd operations */
    MOD_DMC_BING_MEMC_CMD_EXECUTE,

    /*! Command to perform direct_cmd operations and updates the registers */
    MOD_DMC_BING_MEMC_CMD_EXECUTE_DRAIN,

    /*! Command to enter into RECOVER from ABORTED state */
    MOD_DMC_BING_MEMC_CMD_ABORT_CLR,

    /*! DMC-Bing Commands */
    MOD_DMC_BING_MEMC_CMD_COUNT,
};

/*!
 * \brief Identifiers of the current state of DMC-Bing.
 */
enum mod_dmc_bing_state {
    DMC_BING_CONFIG_STATE,
    DMC_BING_LOW_POWER_STATE,
    DMC_BING_PAUSED_STATE,
    DMC_BING_READY_STATE,
    DMC_BING_ABORTED_STATE,
    DMC_BING_RECOVER_STATE,
    DMC_BING_STATE_COUNT,
};

/*!
 * \brief Identifiers of the current state of DMC-Bing.
 */
enum mod_dmc_ddr_training_type {
    /*! Read data eye training */
    DDR_ADDR_TRAIN_TYPE_RD_EYE = 1,

    /*! Read gate training */
    DDR_ADDR_TRAIN_TYPE_RD_GATE,

    /*! Write levelling training */
    DDR_ADDR_TRAIN_TYPE_WR_LVL,

    /*! VREF training */
    DDR_ADDR_TRAIN_TYPE_VREF,

    /*! Training type count */
    DDR_ADDR_TRAIN_TYPE_COUNT,
};

/*!
 * \brief Delay Function
 *
 * \param ms Number of milliseconds delay required.
 *
 * \retval NONE
 */
void delay_ms(uint32_t ms);
/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_DMC_BING_H */
