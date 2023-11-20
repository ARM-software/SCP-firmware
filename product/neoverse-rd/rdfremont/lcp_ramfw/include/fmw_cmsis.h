/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_CMSIS_H
#define FMW_CMSIS_H

#include <stdint.h>

#define __CHECK_DEVICE_DEFINES
#define __CM55_REV             0x0000U
#define __FPU_PRESENT          0U
#define __MPU_PRESENT          1U
#define __VTOR_PRESENT         1U
#define __PMU_PRESENT          0U
#define __DSP_PRESENT          0U
#define __ICACHE_PRESENT       0U
#define __DCACHE_PRESENT       0U
#define __DTCM_PRESENT         0U
#define __NVIC_PRIO_BITS       3U
#define __SAUREGION_PRESENT    0U
#define __Vendor_SysTickConfig 0U

typedef enum IRQn {
    Reset_IRQn = -15,
    NonMaskableInt_IRQn = -14,
    HardFault_IRQn = -13,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn = -11,
    UsageFault_IRQn = -10,
    SecureFault_IRQn = -9,
    SVCall_IRQn = -5,
    DebugMonitor_IRQn = -4,
    PendSV_IRQn = -2,
    SysTick_IRQn = -1,

    WDG_RST_RQST_IRQ = 0, /* Watchdog reset request */
    WDG_INT_IRQ = 1, /* Watchdog interrupt */
    RESERVED2_IRQ = 2, /* Reserved */
    TIMER_IRQ = 3, /* Timer */
    RESERVED4_IRQ = 4, /* Reserved */
    RESERVED5_IRQ = 5, /* Reserved */
    RESERVED6_IRQ = 6, /* Reserved */
    RESERVED7_IRQ = 7, /* Reserved */
    RESERVED8_IRQ = 8, /* Reserved */
    RESERVED9_IRQ = 9, /* Reserved */

    IRQn_MAX = INT16_MAX,
} IRQn_Type;

#include <core_cm55.h>

extern uint32_t SystemCoreClock;

#endif /* FMW_CMSIS_H */
