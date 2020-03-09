/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno V2M System registers definitions.
 */

#ifndef V2M_SYS_REGS_H
#define V2M_SYS_REGS_H

#include "juno_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

enum v2m_sys_regs_adc_dev {
    SYS_REGS_ADC_DEV_SYS,
    SYS_REGS_ADC_DEV_BIG,
    SYS_REGS_ADC_DEV_LITTLE,
    SYS_REGS_ADC_DEV_GPU,
    SYS_REGS_ADC_DEV_COUNT,
};

struct v2m_sys_regs {
    FWK_RW  uint32_t  ID;
    FWK_RW  uint32_t  USERSW;
    FWK_RW  uint32_t  LED;
            uint32_t  RESERVED1[6];
    FWK_RW  uint32_t  T100HZ;
            uint32_t  RESERVED2[8];
    FWK_RW  uint32_t  MCI;
    FWK_RW  uint32_t  FLASH;
            uint32_t  RESERVED3[2];
    FWK_RW  uint32_t  CONFSW;
            uint32_t  RESERVED4;
    FWK_RW  uint32_t  MISC;
            uint32_t  RESERVED5[8];
    FWK_RW  uint32_t  PROCID0;
    FWK_RW  uint32_t  PROCID1;
            uint32_t  RESERVED6[5];
    FWK_RW  uint32_t  CFG_DATA;
    FWK_RW  uint32_t  CFG_CTRL;
    FWK_RW  uint32_t  CFG_STAT;
            uint32_t  RESERVED7[9];
    FWK_R   uint32_t  ADC_CURRENT[SYS_REGS_ADC_DEV_COUNT];
    FWK_R   uint32_t  ADC_VOLT[SYS_REGS_ADC_DEV_COUNT];
    FWK_R   uint32_t  ADC_POWER[SYS_REGS_ADC_DEV_COUNT];
    FWK_RW  uint64_t  ADC_ENERGY[SYS_REGS_ADC_DEV_COUNT];
};

#define V2M_SYS_REGS ((struct v2m_sys_regs *) V2M_SYS_REG_BASE)

#define V2M_SYS_REGS_ID_REV_POS   28
#define V2M_SYS_REGS_ID_VAR_MASK  0x000F000
#define V2M_SYS_REGS_ID_VAR_POS   12

#endif /* V2M_SYS_REGS_H */
