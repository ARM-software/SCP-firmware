/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_CMSIS_H
#define FMW_CMSIS_H

#define __CHECK_DEVICE_DEFINES
#define __CM3_REV 0x0201
#define __FPU_PRESENT 0U
#define __MPU_PRESENT 1U
#define __ICACHE_PRESENT 0U
#define __DCACHE_PRESENT 0U
#define __DTCM_PRESENT 0U
#define __NVIC_PRIO_BITS 3U
#define __Vendor_SysTickConfig 0U

typedef enum IRQn {
    Reset_IRQn = -15,
    NonMaskableInt_IRQn = -14,
    HardFault_IRQn = -13,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn = -11,
    UsageFault_IRQn = -10,
    SVCall_IRQn = -5,
    DebugMonitor_IRQn = -4,
    PendSV_IRQn = -2,
    SysTick_IRQn = -1,

    SOC_WAKEUP0_IRQ = 16, /* SoC Expansion Wakeup */
    TIMREFCLK_IRQ = 33, /* REFCLK Physical Timer */
    MHU_AP_NONSEC_HP_IRQ = 34, /* MHU High Priority Non-Secure */
    MHU_AP_NONSEC_LP_IRQ = 35, /* MHU Low Priority Non-Secure */
    MHU_AP_SEC_IRQ = 36, /* MHU Secure */
    PPU_CORES0_IRQ = 64, /* PPU core 0 IRQ */
    PPU_CORES1_IRQ = 65, /* PPU core 1 IRQ */
    PPU_CORES2_IRQ = 66, /* PPU core 2 IRQ */
    PPU_CORES3_IRQ = 67, /* PPU core 3 IRQ */
    PPU_CLUSTERS_IRQ = 54, /* Consolidated clusters PPU */
} IRQn_Type;

#include <core_cm3.h>

#endif /* FMW_CMSIS_H */
