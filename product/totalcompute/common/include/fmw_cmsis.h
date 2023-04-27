/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_CMSIS_H
#define FMW_CMSIS_H

#include <stdint.h>

#define __CHECK_DEVICE_DEFINES
#define __CM3_REV              0x0201
#define __FPU_PRESENT          0U
#define __MPU_PRESENT          1U
#define __ICACHE_PRESENT       0U
#define __DCACHE_PRESENT       0U
#define __DTCM_PRESENT         0U
#define __NVIC_PRIO_BITS       3U
#define __Vendor_SysTickConfig 0U
#define __VTOR_PRESENT         1U

extern uint32_t SystemCoreClock; /*!< System Clock Frequency (Core Clock)*/

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

    IRQn_MAX = INT16_MAX,
} IRQn_Type;

#include <core_cm3.h>

#endif /* FMW_CMSIS_H */
