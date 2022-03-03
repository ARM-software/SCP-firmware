/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_CMSIS_H
#define FMW_CMSIS_H

#include <stdint.h>

#define __CHECK_DEVICE_DEFINES
#define __CM3_REV              0x0201U
#define __MPU_PRESENT          1U
#define __NVIC_PRIO_BITS       3U
#define __Vendor_SysTickConfig 0U
#define __VTOR_PRESENT         1U

extern uint32_t SystemCoreClock;

typedef enum IRQn {
    Reset_IRQn = -15,
    NonMaskableInt_IRQn = -14, /*!< SCP Watchdog (SP805) - NMI */
    HardFault_IRQn = -13,
    MemoryManagement_IRQn = -12,
    BusFault_IRQn = -11,
    UsageFault_IRQn = -10,
    SVCall_IRQn = -5,
    DebugMonitor_IRQn = -4,
    PendSV_IRQn = -2,
    SysTick_IRQn = -1,

    CDBG_PWR_UP_REQ_IRQn = 1, /*!< Coresight Debug Power Request */
    CSYS_PWR_UP_REQ_IRQn = 2, /*!< Coresight System Power Request */
    CDBG_RST_REQ_IRQn = 3, /*!< Coresight Debug Reset Request */
    GIC_EXT_WAKEUP = 4, /*!< External GIC Wakeup Request */

    TIMREFCLK_IRQn = 33, /*!< REFCLK Physical Timer */

    CTI_TRIGGER0_IRQn = 37, /*!< SCP CTI Trigger */
    CTI_TRIGGER1_IRQn = 38, /*!< SCP CTI Trigger */
    GIC_ERROR_ECC_IRQn = 39, /*!< GIC Error (ECC Fatal) */
    GIC_ERROR_AXIM_IRQn = 40, /*!< GIC Error (AXIM) */

    AON_UART_IRQn = 42, /*!< Always on UART */

    GEN_WD_WS0_IRQn = 44, /*!< Generic Watchdog timer WS0 */
    GEN_WD_WS1_IRQn = 45, /*!< Generic Watchdog timer WS1 */
    TRUSTED_WD_WS0_IRQn = 46, /*!< Trusted Watchdog timer WS0 */
    TRUSTED_WD_WS1_IRQn = 47, /*!< Trusted Watchdog timer WS1 */
    APPS_UART_IRQn = 48, /*!< Application UART */
    CCN_IRQn = 49, /*!< CCN */
    PPU_A72_1_32_IRQn = 50, /*!< A72 CPU1 - 32 Power Policy Unit */
    PPU_A72_33_64_IRQn = 51, /*!< A72 CPU33 - 64 Power Policy Unit */
    PPU_A72_65_96_IRQn = 52, /*!< A72 CPU65 - 96 Power Policy Unit */
    PPU_A72_97_128_IRQn = 53, /*!< A72 CPU97 - 128 Power Policy Unit */
    PPU_A72_SSTOP_CLUSTER_1_32_IRQn = 54, /*!< A72 SSTOP Power Policy Unit */
    PLL_CLUSTER_1_16_LOCK_IRQn = 55, /*!< CPU 1 ~ 16  PLL Lock */
    PLL_CLUSTER_17_32_LOCK_IRQn = 56, /*!< CPU 17~32  PLL Lock */
    PLL_CLUSTER_1_16_UNLOCK_IRQn = 57, /*!< CPU 1 ~ 16  PLL Unlock */
    PLL_CLUSTER_17_32_UNLOCK_IRQn = 58, /*!< CPU 17 ~ 32 PLL Unlock */

    MHU_NON_SEC_SCP_AP_IRQn = 82, /*!< MHU non-secure IRQ between SCP and AP */
    MHU_SEC_SCP_AP_IRQn = 83, /*!< MHU secure IRQ between SCP and AP */
    MHU_NON_SEC_SCP_MCP_IRQn =
        84, /*!< MHU non-secure IRQ between SCP and MCP */
    MHU_SEC_SCP_MCP_IRQn = 85, /*!< MHU secure IRQ between SCP and MCP */

    P1_32_REFCLK_GEN_TIM_IRQn = 90, /*!< P0 Reference Clock Timer */
    P33_64_REFCLK_GEN_TIM_IRQn = 91, /*!< P1 Reference Clock Timer */
    P65_96_REFCLK_GEN_TIM_IRQn = 92, /*!< P2 Reference Clock Timer */
    P97_128_REFCLK_GEN_TIM_IRQn = 93, /*!< P3 Reference Clock Timer */

    DEBUG_PIK_IRQn = 130, /*!< DEBUG PIK */
    PPU_LOGIC_IRQn = 131, /*!< PPU LOGIC */
    PPU_L3RAM0_IRQn = 132, /*!< PPU L3 RAM 0 */
    PPU_L3RAM1_IRQn = 133, /*!< PPU L3 RAM 1 */
    PPU_SFRAM_IRQn = 134, /*!< PPU SF RAM */
    PPU_SRAM_IRQn = 135, /*!< PPU S  RAM */

    MCP_WS1_IRQn = 139, /*!< MCP watchdog reset */
    PLL_SYS_LOCK_IRQn = 140, /*!< System PLL Lock */
    PLL_SYS_UNLOCK_IRQn = 141, /*!< System PLL Unlock */

    PLL_DMC_LOCK_IRQn = 174, /*!< DMC PLL LOCK */
    PLL_DMC_UNLOCK_IRQn = 175, /*!< DMC PLL LOCK */

    DMC1_COMBINED_OFLOW_IRQn = 187, /*!< DMC 1 Combined Overflow */
    DMC1_COMBINED_ERR_OVLOW_IRQn = 188, /*!< DMC 1 Combined err Ovlow */
    DMC1_COMBINED_ECC_ERR_IRQn = 189, /*!< DMC 1 ECC Error */
    DMC1_COMBINED_MISC_ACCESS_IRQn =
        190, /*!< DMC 1 Combined Miscellaneous access */
    DMC1_TEMPERATURE_EVENT_IRQn = 191, /*!< DMC 1 Temperature event */
    DMC1_SCRUB_IRQn = 192, /*!< DMC 1 Scrub */
    DMC1_CC_FREQ_IRQn = 193, /*!< DMC 1 clock frequency */

    DMC3_COMBINED_OFLOW_IRQn = 201, /*!< DMC 3 Combined Overflow */
    DMC3_COMBINED_ERR_OVLOW_IRQn = 202, /*!< DMC 3 Combined err Ovlow */
    DMC3_COMBINED_ECC_ERR_IRQn = 203, /*!< DMC 3 ECC Error */
    DMC3_COMBINED_MISC_ACCESS_IRQn =
        204, /*!< DMC 3 Combined Miscellaneous access */
    DMC3_TEMPERATURE_EVENT_IRQn = 205, /*!< DMC 3 Temperature event */
    DMC3_SCRUB_IRQn = 206, /*!< DMC 3 Scrub */
    DMC3_CC_FREQ_IRQn = 207, /*!< DMC 3 clock frequency */

    NETSEC0_IRQn = 212, /*!< NETSEC0 INTA */
    NETSEC1_IRQn = 214, /*!< NETSEC1 INTA */

    IRQn_MAX = INT16_MAX,
} IRQn_Type;

#include <core_cm3.h>

#endif /* FMW_CMSIS_H */
