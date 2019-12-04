/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     DMC-500 module.
 */

#ifndef MOD_DMC500_H
#define MOD_DMC500_H

#include <mod_timer.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \addtogroup GroupDMC DMC-500 Driver
 *
 * \details Please consult the Arm CoreLink DMC-500 Dynamic Memory Controller
 *      Technical Reference Manual for details on the specific registers that
 *      are programmed here.
 * @{
 */

/*!
 * \brief DMC-500 register definitions
 */
struct mod_dmc500_reg {
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
            uint32_t RESERVED2;
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
            uint32_t RESERVED4;
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
            uint32_t RESERVED6;
    FWK_R   uint32_t ERR_RAMECC_FR;
            uint32_t RESERVED7;
    FWK_RW  uint32_t ERR_RAMECC_CTLR;
            uint32_t RESERVED8;
    FWK_RW  uint32_t ERR_RAMECC_STATUS;
            uint32_t RESERVED9;
    FWK_RW  uint32_t ERR_RAMECC_ADDR;
    FWK_RW  uint32_t ERR_RAMECC_ADDR2;
    FWK_RW  uint32_t ERR_RAMECC_MISC0;
            uint32_t RESERVED10[3];
    FWK_W   uint32_t ERR_RAMECC_INJECT;
            uint8_t  RESERVED11[0x500 - 0x4A4];
    FWK_R   uint32_t QUEUE_STATUS;
            uint32_t RESERVED12;
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
            uint32_t RESERVED13;
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
            uint32_t RESERVED15;
    FWK_R   uint32_t PMU_MI_INT_INFO;
            uint32_t RESERVED16;
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
            uint32_t RESERVED17;
    FWK_RW  uint32_t ODT_RD_CONTROL_31_00;
            uint32_t RESERVED18;
    FWK_RW  uint32_t PHY_WRDATA_CS_CONTROL_31_00;
            uint32_t RESERVED19;
    FWK_RW  uint32_t PHY_RDDATA_CS_CONTROL_31_00;
            uint32_t RESERVED20;
    FWK_RW  uint32_t PHYUPD_INIT;
    FWK_RW  uint32_t PHY_POWER_CONTROL;
            uint32_t RESERVED21;
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
            uint32_t RESERVED22;
    FWK_RW  uint32_t T_COMPLETION_CHECKS;
    FWK_RW  uint32_t T_RDDATA_EN;
    FWK_RW  uint32_t T_PHYRDLAT;
    FWK_RW  uint32_t T_PHYWRLAT;
    FWK_RW  uint32_t T_PHY_TRAIN;
    FWK_R   uint32_t ERR_PHY_FR;
            uint32_t RESERVED23;
    FWK_RW  uint32_t ERR_PHY_CTLR;
            uint32_t RESERVED24;
    FWK_RW  uint32_t ERR_PHY_STATUS;
            uint32_t RESERVED25;
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
            uint32_t RESERVED28[3];
    FWK_R   uint32_t CFG_INTERRUPT_STATUS;
    FWK_R   uint32_t CFG_FAILED_ACCESS_INT_INFO;
            uint8_t  RESERVED29[0xF30 - 0xF18];
    FWK_RW  uint32_t CFG_INTERRUPT_CONTROL;
            uint32_t RESERVED30;
    FWK_W   uint32_t CFG_INTERRUPT_CLR;
            uint8_t  RESERVED31[0xFC0 - 0xF3C];
    FWK_RW  uint32_t INTEGRATION_TEST_CONTROL;
    FWK_RW  uint32_t INTEGRATION_TEST_OUTPUT;
            uint32_t RESERVED32[2];
    FWK_R   uint32_t PERIPH_ID_4;
            uint32_t RESERVED33[3];
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
 * \brief SI_STATE_CONTROL mask used to prevent request stalling.
 */
#define MOD_DMC500_SI_STATE_CONTROL_GO 0

/*!
 * \brief SI_STATE_CONTROL mask used to enable request stalling.
 */
#define MOD_DMC500_SI_STATE_CONTROL_STALL_REQ (1 << 0)

/*!
 * \brief SI_STATUS mask used to confirm that request stalling is active.
 */
#define MOD_DMC500_SI_STATUS_STALL_ACK (1 << 0)

/*!
 * \brief SI_STATUS mask used to read the empty bit.
 */
#define MOD_DMC500_SI_STATUS_EMPTY (1 << 1)

/*!
 * \brief QUEUE_STATE_CONTROL mask used to prevent request stalling.
 */
#define MOD_DMC500_QUEUE_STATE_CONTROL_GO 0

/*!
 * \brief QUEUE_STATE_CONTROL mask used to enable request stalling.
 */
#define MOD_DMC500_QUEUE_STATE_CONTROL_STALL_REQ (1 << 0)

/*!
 * \brief QUEUE_STATUS mask used to confirm that request stalling is active.
 */
#define MOD_DMC500_QUEUE_STATUS_STALL_ACK (1 << 0)

/*!
 * \brief QUEUE_STATUS mask used to read the empty bit.
 */
#define MOD_DMC500_QUEUE_STATUS_EMPTY (1 << 1)

/*!
 * \brief MI_STATUS mask used to read the idle bit.
 */
#define MOD_DMC500_MI_STATUS_IDLE  (1 << 0)

/*!
 * \brief MI_STATUS mask used to read the empty bit.
 */
#define MOD_DMC500_MI_STATUS_EMPTY (1 << 1)

/*!
 * \brief Create the ADDRESS_MAP value.
 *
 * \param SHUTTER The address shutter.
 *
 * \return The ADDRESS_MAP value.
 */
#define ADDRESS_MAP_VAL(SHUTTER) ((1 << 8) | (SHUTTER))

/*!
 * \brief Create the ADDRESS_CONTROL value.
 *
 * \param RANK Number of bits for the rank.
 * \param BANK Number of bits for the bank.
 * \param ROW Number of bits for the row.
 * \param COL Number of bits for the column.
 *
 * \return The ADDRESS_CONTROL value.
 */
#define ADDRESS_CONTROL_VAL(RANK, BANK, ROW, COL) (((RANK) << 24) | \
                                                  ((BANK) << 16) | \
                                                  ((ROW) << 8) | \
                                                  (COL))

/*!
 * \brief Create the MEMORY_TYPE value
 *
 * \param BANK_GROUP Bank group.
 * \param WIDTH Memory device width.
 * \param TYPE Memory type.
 *
 * \return The MEMORY_TYPE value.
 */
#define MEMORY_TYPE_VAL(BANK_GROUP, WIDTH, TYPE) (((BANK_GROUP) << 16) | \
                                                 ((WIDTH) << 8) | \
                                                 (TYPE))

/*!
 * \brief Create the FORMAT_CONTROL value.
 *
 * \param BURST Memory burst.
 *
 * \return The FORMAT_CONTROL value.
 */
#define FORMAT_CONTROL_VAL(BURST) ((BURST) << 8)

/*!
 * \brief Element configuration.
 */
struct mod_dmc500_element_config {
    /*! Base address of the DMC-500 device's registers */
    uintptr_t dmc;
    /*! Element identifier of the associated DDR PHY-500 device */
    fwk_id_t ddr_phy_id;
};

/*!
 * \brief API of the DDR PHY associate to the DMC
 */
struct mod_dmc_ddr_phy_api {
    /*!
     * \brief Configure a DDR PHY500 device
     *
     * \param element_id Element identifier corresponding to the device to
     *      configure.
     *
     * \retval FWK_SUCCESS if the operation succeed.
     * \return one of the error code otherwise.
     */
    int (*configure)(fwk_id_t element_id);
};

/*!
 * \brief DMC-500 module configuration.
 */
struct mod_dmc500_module_config {
    /*!
     * Element identifier of the timer used for delays when programming the
     * DMC-500
     */
    fwk_id_t timer_id;
    /*! DDR PHY module ID */
    fwk_id_t ddr_phy_module_id;
    /*! DDR PHY API ID */
    fwk_id_t ddr_phy_api_id;
    /*! Initial value for the dmc registers */
    const struct mod_dmc500_reg *reg_val;
    /*! Pointer to a product-specific function that issues direct commands */
    void (*direct_ddr_cmd)(struct mod_dmc500_reg *dmc);
};

/*!
 * \brief DMC-500 module description.
 */
extern const struct fwk_module module_dmc500;

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_DMC500_H */
