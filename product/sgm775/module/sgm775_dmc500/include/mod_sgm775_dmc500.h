/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGM775 DMC-500 module.
 */

#ifndef MOD_SGM775_DMC500_H
#define MOD_SGM775_DMC500_H

#include <mod_timer.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdint.h>

/*!
 * \addtogroup GroupSGM775Module SGM775 Product Modules
 * @{
 */

/*!
 * \addtogroup GroupSGM775_DMC SGM775 DMC-500 Driver
 * @{
 */

/*!
 * \brief DMC-500 register definitions
 */
struct mod_sgm775_dmc500_reg {
    /*!
     * \cond
     * @{
     */
    FWK_R   uint32_t SI0_SI_STATUS;
    FWK_R   uint32_t SI0_SI_INTERRUPT_STATUS;
    FWK_R   uint32_t SI0_TZ_FAIL_ADDRESS_LOW;
    FWK_R   uint32_t SI0_TZ_FAIL_ADDRESS_HIGH;
    FWK_R   uint32_t SI0_TZ_FAIL_CONTROL;
    FWK_R   uint32_t SI0_TZ_FAIL_ID;
    FWK_R   uint32_t SI0_PMU_REQ_INT_INFO;
    FWK_RW  uint32_t SI0_PMU_REQ_COUNT0;
    FWK_RW  uint32_t SI0_PMU_REQ_COUNT1;
    FWK_RW  uint32_t SI0_PMU_REQ_COUNT2;
    FWK_RW  uint32_t SI0_PMU_REQ_COUNT3;
    FWK_RW  uint32_t SI0_PMU_SCLK_COUNT_COUNT;
    FWK_RW  uint32_t SI0_SI_STATE_CONTROL;
    FWK_W   uint32_t SI0_SI_FLUSH_CONTROL;
    FWK_RW  uint32_t ADDRESS_CONTROL;
    FWK_RW  uint32_t DECODE_CONTROL;
    FWK_RW  uint32_t ADDRESS_MAP;
    FWK_RW  uint32_t RANK_REMAP_CONTROL;
    FWK_RW  uint32_t SI0_SI_INTERRUPT_CONTROL;
    FWK_W   uint32_t SI0_SI_INTERRUPT_CLR;
    FWK_RW  uint32_t TZ_ACTION;
    FWK_R   uint32_t SI0_TZ_REGION_BASE_LOW_0;
    FWK_R   uint32_t SI0_TZ_REGION_BASE_HIGH_0;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_LOW_0;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_HIGH_0;
    FWK_RW  uint32_t SI0_TZ_REGION_ATTRIBUTES_0;
    FWK_RW  uint32_t SI0_TZ_REGION_ID_ACCESS_0;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_LOW_1;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_HIGH_1;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_LOW_1;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_HIGH_1;
    FWK_RW  uint32_t SI0_TZ_REGION_ATTRIBUTES_1;
    FWK_RW  uint32_t SI0_TZ_REGION_ID_ACCESS_1;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_LOW_2;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_HIGH_2;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_LOW_2;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_HIGH_2;
    FWK_RW  uint32_t SI0_TZ_REGION_ATTRIBUTES_2;
    FWK_RW  uint32_t SI0_TZ_REGION_ID_ACCESS_2;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_LOW_3;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_HIGH_3;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_LOW_3;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_HIGH_3;
    FWK_RW  uint32_t SI0_TZ_REGION_ATTRIBUTES_3;
    FWK_RW  uint32_t SI0_TZ_REGION_ID_ACCESS_3;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_LOW_4;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_HIGH_4;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_LOW_4;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_HIGH_4;
    FWK_RW  uint32_t SI0_TZ_REGION_ATTRIBUTES_4;
    FWK_RW  uint32_t SI0_TZ_REGION_ID_ACCESS_4;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_LOW_5;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_HIGH_5;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_LOW_5;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_HIGH_5;
    FWK_RW  uint32_t SI0_TZ_REGION_ATTRIBUTES_5;
    FWK_RW  uint32_t SI0_TZ_REGION_ID_ACCESS_5;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_LOW_6;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_HIGH_6;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_LOW_6;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_HIGH_6;
    FWK_RW  uint32_t SI0_TZ_REGION_ATTRIBUTES_6;
    FWK_RW  uint32_t SI0_TZ_REGION_ID_ACCESS_6;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_LOW_7;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_HIGH_7;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_LOW_7;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_HIGH_7;
    FWK_RW  uint32_t SI0_TZ_REGION_ATTRIBUTES_7;
    FWK_RW  uint32_t SI0_TZ_REGION_ID_ACCESS_7;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_LOW_8;
    FWK_RW  uint32_t SI0_TZ_REGION_BASE_HIGH_8;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_LOW_8;
    FWK_RW  uint32_t SI0_TZ_REGION_TOP_HIGH_8;
    FWK_RW  uint32_t SI0_TZ_REGION_ATTRIBUTES_8;
    FWK_RW  uint32_t SI0_TZ_REGION_ID_ACCESS_8;
    FWK_RW  uint32_t SI0_PMU_REQ_CONTROL;
    FWK_RW  uint32_t SI0_PMU_REQ_ATTRIBUTE_MASK_0;
    FWK_RW  uint32_t SI0_PMU_REQ_ATTRIBUTE_MATCH_0;
    FWK_RW  uint32_t SI0_PMU_REQ_ATTRIBUTE_MASK_1;
    FWK_RW  uint32_t SI0_PMU_REQ_ATTRIBUTE_MATCH_1;
    FWK_RW  uint32_t SI0_PMU_REQ_ATTRIBUTE_MASK_2;
    FWK_RW  uint32_t SI0_PMU_REQ_ATTRIBUTE_MATCH_2;
    FWK_RW  uint32_t SI0_PMU_REQ_ATTRIBUTE_MASK_3;
    FWK_RW  uint32_t SI0_PMU_REQ_ATTRIBUTE_MATCH_3;
    FWK_RW  uint32_t SI0_THRESHOLD_CONTROL;
            uint8_t  RESERVED0[0x200 - 0x154];
    FWK_R   uint32_t SI1_SI_STATUS;
    FWK_R   uint32_t SI1_SI_INTERRUPT_STATUS;
    FWK_R   uint32_t SI1_TZ_FAIL_ADDRESS_LOW;
    FWK_R   uint32_t SI1_TZ_FAIL_ADDRESS_HIGH;
    FWK_R   uint32_t SI1_TZ_FAIL_CONTROL;
    FWK_R   uint32_t SI1_TZ_FAIL_ID;
    FWK_R   uint32_t SI1_PMU_REQ_INT_INFO;
    FWK_RW  uint32_t SI1_PMU_REQ_COUNT0;
    FWK_RW  uint32_t SI1_PMU_REQ_COUNT1;
    FWK_RW  uint32_t SI1_PMU_REQ_COUNT2;
    FWK_RW  uint32_t SI1_PMU_REQ_COUNT3;
    FWK_RW  uint32_t SI1_PMU_SCLK_COUNT_COUNT;
    FWK_RW  uint32_t SI1_SI_STATE_CONTROL;
    FWK_W   uint32_t SI1_SI_FLUSH_CONTROL;
            uint8_t  RESERVED1[0x248 - 0x238];
    FWK_RW  uint32_t SI1_SI_INTERRUPT_CONTROL;
    FWK_W   uint32_t SI1_SI_INTERRUPT_CLR;
            uint8_t  RESERVED2[0x254 - 0x250];
    FWK_R   uint32_t SI1_TZ_REGION_BASE_LOW_0;
    FWK_R   uint32_t SI1_TZ_REGION_BASE_HIGH_0;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_LOW_0;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_HIGH_0;
    FWK_RW  uint32_t SI1_TZ_REGION_ATTRIBUTES_0;
    FWK_RW  uint32_t SI1_TZ_REGION_ID_ACCESS_0;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_LOW_1;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_HIGH_1;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_LOW_1;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_HIGH_1;
    FWK_RW  uint32_t SI1_TZ_REGION_ATTRIBUTES_1;
    FWK_RW  uint32_t SI1_TZ_REGION_ID_ACCESS_1;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_LOW_2;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_HIGH_2;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_LOW_2;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_HIGH_2;
    FWK_RW  uint32_t SI1_TZ_REGION_ATTRIBUTES_2;
    FWK_RW  uint32_t SI1_TZ_REGION_ID_ACCESS_2;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_LOW_3;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_HIGH_3;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_LOW_3;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_HIGH_3;
    FWK_RW  uint32_t SI1_TZ_REGION_ATTRIBUTES_3;
    FWK_RW  uint32_t SI1_TZ_REGION_ID_ACCESS_3;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_LOW_4;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_HIGH_4;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_LOW_4;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_HIGH_4;
    FWK_RW  uint32_t SI1_TZ_REGION_ATTRIBUTES_4;
    FWK_RW  uint32_t SI1_TZ_REGION_ID_ACCESS_4;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_LOW_5;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_HIGH_5;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_LOW_5;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_HIGH_5;
    FWK_RW  uint32_t SI1_TZ_REGION_ATTRIBUTES_5;
    FWK_RW  uint32_t SI1_TZ_REGION_ID_ACCESS_5;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_LOW_6;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_HIGH_6;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_LOW_6;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_HIGH_6;
    FWK_RW  uint32_t SI1_TZ_REGION_ATTRIBUTES_6;
    FWK_RW  uint32_t SI1_TZ_REGION_ID_ACCESS_6;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_LOW_7;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_HIGH_7;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_LOW_7;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_HIGH_7;
    FWK_RW  uint32_t SI1_TZ_REGION_ATTRIBUTES_7;
    FWK_RW  uint32_t SI1_TZ_REGION_ID_ACCESS_7;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_LOW_8;
    FWK_RW  uint32_t SI1_TZ_REGION_BASE_HIGH_8;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_LOW_8;
    FWK_RW  uint32_t SI1_TZ_REGION_TOP_HIGH_8;
    FWK_RW  uint32_t SI1_TZ_REGION_ATTRIBUTES_8;
    FWK_RW  uint32_t SI1_TZ_REGION_ID_ACCESS_8;
    FWK_RW  uint32_t SI1_PMU_REQ_CONTROL;
    FWK_RW  uint32_t SI1_PMU_REQ_ATTRIBUTE_MASK_0;
    FWK_RW  uint32_t SI1_PMU_REQ_ATTRIBUTE_MATCH_0;
    FWK_RW  uint32_t SI1_PMU_REQ_ATTRIBUTE_MASK_1;
    FWK_RW  uint32_t SI1_PMU_REQ_ATTRIBUTE_MATCH_1;
    FWK_RW  uint32_t SI1_PMU_REQ_ATTRIBUTE_MASK_2;
    FWK_RW  uint32_t SI1_PMU_REQ_ATTRIBUTE_MATCH_2;
    FWK_RW  uint32_t SI1_PMU_REQ_ATTRIBUTE_MASK_3;
    FWK_RW  uint32_t SI1_PMU_REQ_ATTRIBUTE_MATCH_3;
    FWK_RW  uint32_t SI1_THRESHOLD_CONTROL;
            uint8_t  RESERVED3[0x400 - 0x354];
    FWK_R   uint32_t DCB_STATUS;
    FWK_R   uint32_t M_INTERRUPT_STATUS;
    FWK_R   uint32_t PMU_DCB_INT_INFO;
    FWK_W   uint32_t DCB_STATE_CONTROL;
            uint8_t  RESERVED4[0x414 - 0x410];
    FWK_RW  uint32_t QUEUE_THRESHOLD_CONTROL_31_00;
    FWK_RW  uint32_t QUEUE_THRESHOLD_CONTROL_63_32;
            uint8_t  RESERVED5[0x42C - 0x41C];
    FWK_RW  uint32_t DCB_INTERRUPT_CONTROL;
    FWK_W   uint32_t DCB_INTERRUPT_CLR;
    FWK_RW  uint32_t PMU_DCB_CONTROL;
    FWK_RW  uint32_t PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_0;
    FWK_RW  uint32_t PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_0;
    FWK_RW  uint32_t PMU_DATA_CONTROL_BLOCK_COUNT_0;
    FWK_RW  uint32_t PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_1;
    FWK_RW  uint32_t PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_1;
    FWK_RW  uint32_t PMU_DATA_CONTROL_BLOCK_COUNT_1;
    FWK_RW  uint32_t PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MASK_2;
    FWK_RW  uint32_t PMU_DATA_CONTROL_BLOCK_ATTRIBUTE_MATCH_2;
    FWK_RW  uint32_t PMU_DATA_CONTROL_BLOCK_COUNT_2;
    FWK_RW  uint32_t PMU_TAG_ENTRIES_ATTRIBUTE_MASK;
    FWK_RW  uint32_t PMU_TAG_ENTRIES_ATTRIBUTE_MATCH;
    FWK_RW  uint32_t PMU_TAG_ENTRIES_COUNT;
    FWK_RW  uint32_t PMU_MCLK_COUNT_COUNT;
            uint8_t  RESERVED6[0x470 - 0x46C];
    FWK_R   uint32_t ERR_RAMECC_FR;
            uint8_t  RESERVED7[0x478 - 0x474];
    FWK_RW  uint32_t ERR_RAMECC_CTLR;
            uint8_t  RESERVED8[0x480 - 0x47C];
    FWK_RW  uint32_t ERR_RAMECC_STATUS;
            uint8_t  RESERVED9[0x488 - 0x484];
    FWK_RW  uint32_t ERR_RAMECC_ADDR;
    FWK_RW  uint32_t ERR_RAMECC_ADDR2;
    FWK_RW  uint32_t ERR_RAMECC_MISC0;
            uint8_t  RESERVED10[0x4A0 - 0x494];
    FWK_W   uint32_t ERR_RAMECC_INJECT;
            uint8_t  RESERVED11[0x500 - 0x4A4];
    FWK_R   uint32_t QUEUE_STATUS;
            uint8_t  RESERVED12[0x508 - 0x504];
    FWK_R   uint32_t PMU_QE_INT_INFO;
    FWK_RW  uint32_t QUEUE_STATE_CONTROL;
    FWK_RW  uint32_t QE_INTERRUPT_CONTROL;
    FWK_W   uint32_t QE_INTERRUPT_CLR;
    FWK_RW  uint32_t RANK_TURNAROUND_CONTROL;
    FWK_RW  uint32_t HIT_TURNAROUND_CONTROL;
    FWK_RW  uint32_t QOS_CLASS_CONTROL;
    FWK_RW  uint32_t ESCALATION_CONTROL;
    FWK_RW  uint32_t QV_CONTROL_31_00;
    FWK_RW  uint32_t QV_CONTROL_63_32;
    FWK_RW  uint32_t RT_CONTROL_31_00;
    FWK_RW  uint32_t RT_CONTROL_63_32;
    FWK_RW  uint32_t TIMEOUT_CONTROL;
    FWK_RW  uint32_t WRITE_PRIORITY_CONTROL_31_00;
    FWK_RW  uint32_t WRITE_PRIORITY_CONTROL_63_32;
            uint8_t  RESERVED13[0x548 - 0x544];
    FWK_RW  uint32_t DIR_TURNAROUND_CONTROL;
    FWK_RW  uint32_t HIT_PREDICTION_CONTROL;
    FWK_RW  uint32_t REFRESH_ENABLE;
    FWK_R   uint32_t REFRESH_STATUS;
    FWK_R   uint32_t REFRESH_STATUS_FG;
    FWK_RW  uint32_t REFRESH_PRIORITY;
    FWK_RW  uint32_t MC_UPDATE_CONTROL;
    FWK_RW  uint32_t PHY_UPDATE_CONTROL;
    FWK_RW  uint32_t PHY_MASTER_CONTROL;
    FWK_RW  uint32_t LOW_POWER_CONTROL;
    FWK_RW  uint32_t PMU_QE_CONTROL;
    FWK_RW  uint32_t PMU_QE_MUX;
    FWK_RW  uint32_t PMU_QOS_ENGINE_ATTRIBUTE_MASK_0;
    FWK_RW  uint32_t PMU_QOS_ENGINE_ATTRIBUTE_MATCH_0;
    FWK_RW  uint32_t PMU_QOS_ENGINE_COUNT_0;
    FWK_RW  uint32_t PMU_QOS_ENGINE_ATTRIBUTE_MASK_1;
    FWK_RW  uint32_t PMU_QOS_ENGINE_ATTRIBUTE_MATCH_1;
    FWK_RW  uint32_t PMU_QOS_ENGINE_COUNT_1;
    FWK_RW  uint32_t PMU_QOS_ENGINE_ATTRIBUTE_MASK_2;
    FWK_RW  uint32_t PMU_QOS_ENGINE_ATTRIBUTE_MATCH_2;
    FWK_RW  uint32_t PMU_QOS_ENGINE_COUNT_2;
    FWK_RW  uint32_t PMU_QUEUED_ENTRIES_ATTRIBUTE_MASK;
    FWK_RW  uint32_t PMU_QUEUED_ENTRIES_ATTRIBUTE_MATCH;
    FWK_RW  uint32_t PMU_QUEUED_ENTRIES_COUNT;
            uint8_t  RESERVED14[0x600 - 0x5A8];
    FWK_R   uint32_t MI_STATUS;
    FWK_R   uint32_t RANKS_READY;
    FWK_R   uint32_t RANKS_RESET;
    FWK_R   uint32_t RANKS_DEEP_POWER_DOWN;
    FWK_R   uint32_t RANKS_SELF_REFRESH;
    FWK_R   uint32_t RANKS_POWERED_DOWN;
    FWK_R   uint32_t RANKS_CLOCK_DISABLED;
    FWK_R   uint32_t PHY_STATUS0;
    FWK_R   uint32_t PHY_STATUS1;
            uint8_t  RESERVED15[0x628 - 0x624];
    FWK_R   uint32_t PMU_MI_INT_INFO;
            uint8_t  RESERVED16[0x630 - 0x62C];
    FWK_RW  uint32_t MI_STATE_CONTROL;
    FWK_RW  uint32_t PHY_CONFIG;
    FWK_RW  uint32_t DIRECT_CMD_SETTINGS;
    FWK_RW  uint32_t DIRECT_CMD;
    FWK_RW  uint32_t DIRECT_CLK_DISABLE;
    FWK_RW  uint32_t DIRECT_ODT;
    FWK_RW  uint32_t DCI_STRB;
    FWK_RW  uint32_t DCI_DATA;
    FWK_W   uint32_t DCI_DATA_CLR;
    FWK_W   uint32_t RANK_STATUS_OVERRIDE;
    FWK_W   uint32_t CLK_STATUS_OVERRIDE;
    FWK_W   uint32_t BANK_STATUS_OVERRIDE;
    FWK_RW  uint32_t MI_INTERRUPT_CONTROL;
    FWK_W   uint32_t MI_INTERRUPT_CLR;
    FWK_RW  uint32_t MEMORY_TYPE;
    FWK_RW  uint32_t FORMAT_CONTROL;
    FWK_RW  uint32_t FEATURE_CONTROL;
    FWK_RW  uint32_t POWER_DOWN_CONTROL;
    FWK_RW  uint32_t REFRESH_CONTROL;
    FWK_RW  uint32_t ODT_WR_CONTROL_31_00;
            uint8_t  RESERVED17[0x684 - 0x680];
    FWK_RW  uint32_t ODT_RD_CONTROL_31_00;
            uint8_t  RESERVED18[0x68C - 0x688];
    FWK_RW  uint32_t PHY_WRDATA_CS_CONTROL_31_00;
            uint8_t  RESERVED19[0x694 - 0x690];
    FWK_RW  uint32_t PHY_RDDATA_CS_CONTROL_31_00;
            uint8_t  RESERVED20[0x69C - 0x698];
    FWK_RW  uint32_t PHYUPD_INIT;
    FWK_RW  uint32_t PHY_POWER_CONTROL;
            uint8_t  RESERVED21[0x6A8 - 0x6A4];
    FWK_RW  uint32_t ODT_TIMING;
    FWK_RW  uint32_t T_REFI;
    FWK_RW  uint32_t T_RFC;
    FWK_RW  uint32_t T_RCD;
    FWK_RW  uint32_t T_RAS;
    FWK_RW  uint32_t T_RP;
    FWK_RW  uint32_t T_RRD;
    FWK_RW  uint32_t T_ACT_WINDOW;
    FWK_RW  uint32_t T_RTR;
    FWK_RW  uint32_t T_RTW;
    FWK_RW  uint32_t T_RTP;
    FWK_RW  uint32_t T_RDPDEN;
    FWK_RW  uint32_t T_WR;
    FWK_RW  uint32_t T_WTR;
    FWK_RW  uint32_t T_WTW;
    FWK_RW  uint32_t T_XTMW;
    FWK_RW  uint32_t T_WRPDEN;
    FWK_RW  uint32_t T_CLOCK_CONTROL;
    FWK_RW  uint32_t T_EP;
    FWK_RW  uint32_t T_XP;
    FWK_RW  uint32_t T_ESR;
    FWK_RW  uint32_t T_XSR;
            uint8_t  RESERVED22[0x704 - 0x700];
    FWK_RW  uint32_t T_COMPLETION_CHECKS;
    FWK_RW  uint32_t T_RDDATA_EN;
    FWK_RW  uint32_t T_PHYRDLAT;
    FWK_RW  uint32_t T_PHYWRLAT;
    FWK_RW  uint32_t T_PHY_TRAIN;
    FWK_R   uint32_t ERR_PHY_FR;
            uint8_t  RESERVED23[0x720 - 0x71C];
    FWK_RW  uint32_t ERR_PHY_CTLR;
            uint8_t  RESERVED24[0x728 - 0x724];
    FWK_RW  uint32_t ERR_PHY_STATUS;
            uint8_t  RESERVED25[0x730 - 0x72C];
    FWK_RW  uint32_t ERR_PHY_ADDR;
    FWK_RW  uint32_t ERR_PHY_ADDR2;
    FWK_RW  uint32_t ERR_PHY_MISC0;
            uint8_t  RESERVED26[0x74C - 0x73C];
    FWK_W   uint32_t ERR_PHY_INJECT;
    FWK_RW  uint32_t PMU_MI_CONTROL;
    FWK_RW  uint32_t PMU_MEMORY_IF_ATTRIBUTE_MASK_0;
    FWK_RW  uint32_t PMU_MEMORY_IF_ATTRIBUTE_MATCH_0;
    FWK_RW  uint32_t PMU_MEMORY_IF_COUNT_0;
    FWK_RW  uint32_t PMU_MEMORY_IF_ATTRIBUTE_MASK_1;
    FWK_RW  uint32_t PMU_MEMORY_IF_ATTRIBUTE_MATCH_1;
    FWK_RW  uint32_t PMU_MEMORY_IF_COUNT_1;
    FWK_RW  uint32_t PMU_BANK_STATES_ATTRIBUTE_MASK;
    FWK_RW  uint32_t PMU_BANK_STATES_ATTRIBUTE_MATCH;
    FWK_RW  uint32_t PMU_BANK_STATES_COUNT;
    FWK_RW  uint32_t PMU_RANK_STATES_ATTRIBUTE_MASK;
    FWK_RW  uint32_t PMU_RANK_STATES_ATTRIBUTE_MATCH;
    FWK_RW  uint32_t PMU_RANK_STATES_COUNT;
            uint8_t  RESERVED27[0xF00 - 0x784];
    FWK_R   uint32_t MEMC_CONFIG;
            uint8_t  RESERVED28[0xF10 - 0xF04];
    FWK_R   uint32_t CFG_INTERRUPT_STATUS;
    FWK_R   uint32_t CFG_FAILED_ACCESS_INT_INFO;
            uint8_t  RESERVED29[0xF30 - 0xF18];
    FWK_RW  uint32_t CFG_INTERRUPT_CONTROL;
            uint8_t  RESERVED30[0xF38 - 0xF34];
    FWK_W   uint32_t CFG_INTERRUPT_CLR;
            uint8_t  RESERVED31[0xFC0 - 0xF3C];
    FWK_RW  uint32_t INTEGRATION_TEST_CONTROL;
    FWK_RW  uint32_t INTEGRATION_TEST_OUTPUT;
            uint8_t  RESERVED32[0xFD0 - 0xFC8];
    FWK_R   uint32_t PERIPH_ID_4;
            uint8_t  RESERVED33[0xFE0 - 0xFD4];
    FWK_R   uint32_t PERIPH_ID_0;
    FWK_R   uint32_t PERIPH_ID_1;
    FWK_R   uint32_t PERIPH_ID_2;
    FWK_R   uint32_t PERIPH_ID_3;
    FWK_R   uint32_t COMPONENT_ID_0;
    FWK_R   uint32_t COMPONENT_ID_1;
    FWK_R   uint32_t COMPONENT_ID_2;
    FWK_R   uint32_t COMPONENT_ID_3;
    /*!
    * \endcond
    * @}
    */
};

/*!
 * \brief SI_STATUS mask used to confirm that request stalling is active.
 */
#define MOD_DMC500_SI_STATUS_STALL_ACK (1 << 0)

/*!
 * \brief QUEUE_STATUS mask used to confirm that request stalling is active.
 */
#define MOD_DMC500_QUEUE_STATUS_STALL_ACK (1 << 0)

/*!
 * \brief MI_STATUS mask used to read the idle bit.
 */
#define MOD_DMC500_MI_STATUS_IDLE  (1 << 0)

/*!
 * \brief Element configuration.
 */
struct mod_sgm775_dmc500_element_config {
    /*! Base address of the DMC-500 device's registers */
    uintptr_t dmc;

    /*! Element identifier of the associated DDR PHY-500 device */
    fwk_id_t ddr_phy_id;
};

/*!
 * \brief API of the DDR PHY associated to the DMC.
 */
struct mod_sgm775_dmc_ddr_phy_api {
    /*!
     * \brief Configure a DDR physical device.
     *
     * \param element_id Element identifier corresponding to the device to
     *      configure.
     *
     * \retval FWK_SUCCESS if the operation succeed.
     * \return One of the standard framework error codes.
     */
    int (*configure)(fwk_id_t element_id);
};

/*!
 * \brief SGM775 DMC-500 module configuration.
 */
struct mod_sgm775_dmc500_module_config {
    /*!
     * Element identifier of the timer used for delays when programming the
     * DMC-500.
     */
    fwk_id_t timer_id;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SGM775_DMC500_H */
