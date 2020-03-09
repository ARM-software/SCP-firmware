/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     CoreSight SoC-400 component register definitions.
 *     Derived from ARM DDI 0480B (CoreSight SoC Technical Reference Manual)
 *     IP Revision: r1p0
 */

#ifndef CORESIGHT_SOC400_H
#define CORESIGHT_SOC400_H

#include <fwk_macros.h>

#include <stdint.h>

struct cti_reg {
    FWK_RW uint32_t  CTI_CONTROL;
           uint8_t   RESERVED1[0x10 - 0x04];
    FWK_W  uint32_t  CTI_INT_ACK;
    FWK_RW uint32_t  CTI_APP_SET;
    FWK_W  uint32_t  CTI_APP_CLEAR;
    FWK_W  uint32_t  CTI_APP_PULSE;
    FWK_RW uint32_t  CTI_IN_EN[8];
           uint8_t   RESERVED2[0xA0 - 0x40];
    FWK_RW uint32_t  CTI_OUT_EN[8];
           uint8_t   RESERVED3[0x130 - 0xC0];
    FWK_R  uint32_t  CTI_TRIG_IN_STATUS;
    FWK_R  uint32_t  CTI_TRIG_OUT_STATUS;
    FWK_R  uint32_t  CTI_CH_IN_STATUS;
    FWK_R  uint32_t  CTI_CH_OUT_STATUS;
    FWK_RW uint32_t  CTI_GATE;
    FWK_RW uint32_t  ASIC_CTL;
           uint8_t   RESERVED4[0xEDC - 0x148];
    FWK_W  uint32_t  IT_CH_IN_ACK;
    FWK_W  uint32_t  IT_TRIG_IN_ACK;
    FWK_W  uint32_t  IT_CH_OUT;
    FWK_W  uint32_t  IT_TRIG_OUT;
    FWK_R  uint32_t  IT_CH_OUT_ACK;
    FWK_R  uint32_t  IT_TRIG_OUT_ACK;
    FWK_R  uint32_t  IT_CH_IN;
    FWK_R  uint32_t  IT_TRIG_IN;
           uint8_t   RESERVED5[0xF00 - 0xEFC];
    FWK_RW uint32_t  IT_CTRL;
           uint8_t   RESERVED6[0xFA0 - 0xF04];
    FWK_RW uint32_t  CLAIM_SET;
    FWK_RW uint32_t  CLAIM_CLEAR;
           uint8_t   RESERVED7[0xFB0 - 0xFA8];
    FWK_W  uint32_t  LAR;
    FWK_R  uint32_t  LSR;
    FWK_R  uint32_t  AUTH_STATUS;
           uint8_t   RESERVED8[0xFC8 - 0xFBC];
    FWK_R  uint32_t  DEV_ID;
    FWK_R  uint32_t  DEV_TYPE;
    FWK_R  uint32_t  PERIPH_ID_4;
    FWK_R  uint32_t  PERIPH_ID_5;
    FWK_R  uint32_t  PERIPH_ID_6;
    FWK_R  uint32_t  PERIPH_ID_7;
    FWK_R  uint32_t  PERIPH_ID_0;
    FWK_R  uint32_t  PERIPH_ID_1;
    FWK_R  uint32_t  PERIPH_ID_2;
    FWK_R  uint32_t  PERIPH_ID_3;
    FWK_R  uint32_t  COMPONENT_ID_0;
    FWK_R  uint32_t  COMPONENT_ID_1;
    FWK_R  uint32_t  COMPONENT_ID_2;
    FWK_R  uint32_t  COMPONENT_ID_3;
};

#define CTI_CONTROL_ENABLE      UINT32_C(0x00000001)

#define CTI_APP_SET_CH0         UINT32_C(0x00000001)
#define CTI_APP_SET_CH1         UINT32_C(0x00000002)
#define CTI_APP_SET_CH2         UINT32_C(0x00000004)
#define CTI_APP_SET_CH3         UINT32_C(0x00000008)

#define CTI_OUT_EN_CH0          UINT32_C(0x00000001)
#define CTI_OUT_EN_CH1          UINT32_C(0x00000002)
#define CTI_OUT_EN_CH2          UINT32_C(0x00000004)
#define CTI_OUT_EN_CH3          UINT32_C(0x00000008)
#define CTI_OUT_EN_ALL          UINT32_C(0x0000000F)
#define CTI_OUT_EN_NONE         UINT32_C(0x00000000)

#define CTI_GATE_EN_CH0         UINT32_C(0x00000001)
#define CTI_GATE_EN_CH1         UINT32_C(0x00000002)
#define CTI_GATE_EN_CH2         UINT32_C(0x00000004)
#define CTI_GATE_EN_CH3         UINT32_C(0x00000008)
#define CTI_GATE_EN_ALL         UINT32_C(0x0000000F)
#define CTI_GATE_EN_NONE        UINT32_C(0x00000000)

#define CTI_LAR_KEY             UINT32_C(0xC5ACCE55)

#endif /* CORESIGHT_SOC400_H */
