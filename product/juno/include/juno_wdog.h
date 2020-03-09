/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno Watchdog registers definition.
 */

#ifndef JUNO_WDOG_H
#define JUNO_WDOG_H

#include "system_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

struct sp805_reg {
    FWK_RW  uint32_t    LOAD;
    FWK_R   uint32_t    VALUE;
    FWK_RW  uint32_t    CONTROL;
    FWK_W   uint32_t    INTCLR;
    FWK_R   uint32_t    RIS;
    FWK_R   uint32_t    MIS;
            uint8_t     RESERVED1[0xC00 - 0x18];
    FWK_RW  uint32_t    LOCK;
            uint8_t     RESERVED2[0xF00 - 0xC04];
    FWK_RW  uint32_t    ITCR;
    FWK_W   uint32_t    ITOP;
            uint8_t     RESERVED3[0xFE0 - 0xF08];
    FWK_R   uint32_t    PERIPHID0;
    FWK_R   uint32_t    PERIPHID1;
    FWK_R   uint32_t    PERIPHID2;
    FWK_R   uint32_t    PERIPHID3;
    FWK_R   uint32_t    PCELLID0;
    FWK_R   uint32_t    PCELLID1;
    FWK_R   uint32_t    PCELLID2;
    FWK_R   uint32_t    PCELLID3;
};

#define WDOG    ((struct sp805_reg *) WDOG_BASE)

#define SP805_LOCK_EN_ACCESS            UINT32_C(0x1ACCE551)
#define SP805_CONTROL_INTR_EN           UINT32_C(0x00000001)
#define SP805_CONTROL_RESET_EN          UINT32_C(0x00000002)

/* Watchdog frequency */
#define SCP_WDOG_FREQUENCY_KHZ          32

/* Convert a period from milliseconds to watchdog timer ticks */
#define SCP_WDOG_MS_TO_TICKS(VALUE_MS)  ((VALUE_MS) * SCP_WDOG_FREQUENCY_KHZ)

#endif /* JUNO_WDOG_H */
