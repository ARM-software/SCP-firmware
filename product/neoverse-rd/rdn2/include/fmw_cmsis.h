/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_CMSIS_H
#define FMW_CMSIS_H

#include <stdint.h>

#define __CHECK_DEVICE_DEFINES
#define __CM7_REV              0x0000U
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
    PPU_CORES0_IRQ =
        50, /* Consolidated PPU Interrupt for cores 1-32, 129-160 */
    PPU_CORES1_IRQ =
        51, /* Consolidated PPU Interrupt for cores 33-64, 161-192 */
    PPU_CORES2_IRQ =
        52, /* Consolidated PPU Interrupt for cores 65-96, 193-224 */
    PPU_CORES3_IRQ =
        53, /* Consolidated PPU Interrupt for cores 97-128, 225-256 */
    PPU_CLUSTERS_IRQ = 54, /* Consolidated clusters PPU */
    MHU_AP_SEC_IRQ = 83, /* MHU secure IRQ between SCP and AP */

    IRQn_MAX = INT16_MAX,
} IRQn_Type;

#include <core_cm7.h>

#endif /* FMW_CMSIS_H */
