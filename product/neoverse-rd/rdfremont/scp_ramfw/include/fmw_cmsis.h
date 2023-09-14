/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
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

/* System Clock Frequency (Core Clock) */
extern uint32_t SystemCoreClock;

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

    /* SCP REFCLK Physical Timer */
    REFCLK_GTIMER_IRQ = 32,
    /* Consolidated CPU PPU Interrupt for 1-32 cores, 129-160 cores */
    PPU_CORES0_IRQ = 50,
    /* Consolidated CPU PPU Interrupt for 33-64 cores, 161-192 cores */
    PPU_CORES1_IRQ = 51,
    /* Consolidated CPU PPU Interrupt for 65-96 cores, 193-224 cores */
    PPU_CORES2_IRQ = 52,
    /* Consolidated CPU PPU Interrupt for 97-128 cores, 225-256 cores */
    PPU_CORES3_IRQ = 53,
    /* Consolidated Cluster PPU Interrupt for 1-32 cores, 129-160 cores */
    PPU_CLUSTERS0_IRQ = 59,
    /* Consolidated Cluster PPU Interrupt for 33-64 cores, 161-192 cores */
    PPU_CLUSTERS1_IRQ = 60,
    /* Consolidated Cluster PPU Interrupt for 65-96 cores, 193-224 cores */
    PPU_CLUSTERS2_IRQ = 61,
    /* Consolidated Cluster PPU Interrupt for 97-128 cores, 225-256 cores */
    PPU_CLUSTERS3_IRQ = 62,
    /* MHUv3 secure IRQ between SCP and AP */
    MHU3_AP2SCP_IRQ_S = 83,
    /* MHUv3 secure IRQ between SCP and RSS */
    MHU3_RSS2SCP_IRQ_S = 86,

    IRQn_MAX = INT16_MAX,
} IRQn_Type;

#include <core_cm7.h>

#endif /* FMW_CMSIS_H */
