/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     NIC-400 Network Interconnect Global Programmers View (GPV) register
 *     definitions.
 *
 */

#ifndef NIC400_GPV_H
#define NIC400_GPV_H

#include "juno_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

struct nic400_gpv_reg {
            uint8_t   RESERVED1[0x100 - 0x0];
    FWK_RW  uint32_t  QOS_READ_PRIORITY;
    FWK_RW  uint32_t  QOS_WRITE_PRIORITY;
    FWK_RW  uint32_t  FN_MOD;
    FWK_RW  uint32_t  QOS_CONTROL;
    FWK_RW  uint32_t  MAX_OT;
    FWK_RW  uint32_t  MAX_COMBINED_OT;
    FWK_RW  uint32_t  AW_PEAK_RATE;
    FWK_RW  uint32_t  AW_BURSTINESS;
    FWK_RW  uint32_t  AW_AVG_RATE;
    FWK_RW  uint32_t  AR_PEAK_RATE;
    FWK_RW  uint32_t  AR_BURSTINESS;
    FWK_RW  uint32_t  AR_AVG_RATE;
    FWK_RW  uint32_t  FEEDBACK_CTRL_TARGET;
    FWK_RW  uint32_t  FEEDBACK_CTRL_SCALE;
    FWK_RW  uint32_t  QOS_RANGE;
            uint8_t   RESERVED2[0x1000 - 0x13C];
};

#define NIC400_GPV_HDLCD0 ((struct nic400_gpv_reg *) NIC400_GPV_HDLCD0_BASE)
#define NIC400_GPV_HDLCD1 ((struct nic400_gpv_reg *) NIC400_GPV_HDLCD1_BASE)
#define NIC400_GPV_CCI_PCIE ((struct nic400_gpv_reg *) NIC400_GPV_CCI_PCIE_BASE)
#define NIC400_GPV_CCI_GPU ((struct nic400_gpv_reg *) NIC400_GPV_CCI_GPU_BASE)
#define NIC400_GPV_CCI_LITTLE \
    ((struct nic400_gpv_reg *)NIC400_GPV_CCI_LITTLE_BASE)
#define NIC400_GPV_CCI_BIG ((struct nic400_gpv_reg *) NIC400_GPV_CCI_BIG_BASE)

/* QOS_CONTROL definitions */
#define NIC400_GPV_QOS_CTRL_EN_AW_RATE                  UINT32_C(0x00000001)
#define NIC400_GPV_QOS_CTRL_EN_AR_RATE                  UINT32_C(0x00000002)
#define NIC400_GPV_QOS_CTRL_EN_AWAR_RATE                UINT32_C(0x00000004)
#define NIC400_GPV_QOS_CTRL_EN_AW_FC                    UINT32_C(0x00000008)
#define NIC400_GPV_QOS_CTRL_EN_AR_FC                    UINT32_C(0x00000010)
#define NIC400_GPV_QOS_CTRL_EN_AW_OT                    UINT32_C(0x00000020)
#define NIC400_GPV_QOS_CTRL_EN_AR_OT                    UINT32_C(0x00000040)
#define NIC400_GPV_QOS_CTRL_EN_AWAR_OT                  UINT32_C(0x00000080)
#define NIC400_GPV_QOS_CTRL_MODE_AW_FC                  UINT32_C(0x00010000)
#define NIC400_GPV_QOS_CTRL_MODE_AR_FC                  UINT32_C(0x00100000)

#define NIC400_GPV_MAX_OT_AW_MAX_OTF                    UINT32_C(0x000000FF)
#define NIC400_GPV_MAX_OT_AW_MAX_OTI                    UINT32_C(0x00003F00)
#define NIC400_GPV_MAX_OT_AR_MAX_OTF                    UINT32_C(0x00FF0000)
#define NIC400_GPV_MAX_OT_AR_MAX_OTI                    UINT32_C(0x3F000000)

#define NIC400_GPV_MAX_COMBINED_OT_AWAR_MAX_OTF         UINT32_C(0x000000FF)
#define NIC400_GPV_MAX_COMBINED_OT_AWAR_MAX_OTI         UINT32_C(0x00007F00)

/* AW_PEAK_RATE definitions */
#define NIC400_GPV_WRITE_PEAK_RATE_AW_P                 UINT32_C(0xFF000000)

/* AW_BURSTINESS definitions */
#define NIC400_GPV_WRITE_BURSTINESS_AW_B                UINT32_C(0x0000FFFF)

/* AW_AVG_RATE definitions */
#define NIC400_GPV_WRITE_AVG_RATE_AW_R                  UINT32_C(0xFFF00000)

/* AR_PEAK_RATE definitions */
#define NIC400_GPV_READ_PEAK_RATE_AR_P                  UINT32_C(0xFF000000)

/* AR_BURSTINESS definitions */
#define NIC400_GPV_READ_BURSTINESS_AR_B                 UINT32_C(0x0000FFFF)

/* AR_AVG_RATE definitions */
#define NIC400_GPV_READ_AVG_RATE_AR_R                   UINT32_C(0xFFF00000)

#define NIC400_GPV_FEEDBACK_CTRL_TARGET_AW_TGT_LATENCY  UINT32_C(0x00000FFF)
#define NIC400_GPV_FEEDBACK_CTRL_TARGET_AR_TGT_LATENCY  UINT32_C(0x0FFF0000)

#define NIC400_GPV_FEEDBACK_CTRL_SCALE_AW_KI            UINT32_C(0x00000007)
#define NIC400_GPV_FEEDBACK_CTRL_SCALE_AR_KI            UINT32_C(0x00000700)

#define NIC400_GPV_QOS_RANGE_AW_MIN_QOS                 UINT32_C(0x0000000F)
#define NIC400_GPV_QOS_RANGE_AW_MAX_QOS                 UINT32_C(0x00000F00)
#define NIC400_GPV_QOS_RANGE_AR_MIN_QOS                 UINT32_C(0x000F0000)
#define NIC400_GPV_QOS_RANGE_AR_MAX_QOS                 UINT32_C(0x0F000000)

#endif /* NIC400_GPV_H */
