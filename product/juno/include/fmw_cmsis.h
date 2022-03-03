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

    WDOG_IRQ = 0, /* SCP System WatchDog (SP805) */
    TIM32KHZ_IRQ = 1, /* 32KHz Physical Timer */
    TIMREFCLK_IRQ = 2, /* REFCLK Physical Timer */
    MHU_HIGH_PRIO_IRQ = 3, /* MHU High Priority Non-Secure int. */
    MHU_LOW_PRIO_IRQ = 4, /* MHU Low Priority interrupt */
    MHU_SECURE_IRQ = 5, /* MHU Secure interrupt */
    CTI_TRIG_OUT_2_IRQ = 6, /* SCP CTI Trigger Interrupt */
    CTI_TRIG_OUT_3_IRQ = 7, /* SCP CTI Trigger Interrupt */
    CDBG_PWR_UP_REQ_IRQ = 8, /* Coresight Debug Power Request */
    CSYS_PWR_UP_REQ_IRQ = 9, /* Coresight System Power Request */
    CDBG_RST_REQ_IRQ = 10, /* Coresight Debug Reset Request */
    BIG_0_DBG_PWR_UP_IRQ = 11, /* big CPU0 Debug Power Up Request */
    BIG_1_DBG_PWR_UP_IRQ = 12, /* big CPU1 Debug Power Up Request */
    BIG_2_DBG_PWR_UP_IRQ = 13, /* big CPU2 Debug Power Up Request */
    BIG_3_DBG_PWR_UP_IRQ = 14, /* big CPU3 Debug Power Up Request */
    LITTLE_0_DBG_PWR_UP_IRQ = 15, /* LITTLE CPU0 Debug Power Up Request */
    LITTLE_1_DBG_PWR_UP_IRQ = 16, /* LITTLE CPU1 Debug Power Up Request */
    LITTLE_2_DBG_PWR_UP_IRQ = 17, /* LITTLE CPU2 Debug Power Up Request */
    LITTLE_3_DBG_PWR_UP_IRQ = 18, /* LITTLE CPU3 Debug Power Up Request */
    BIG_SAC_WAKEUP_REQ_IRQ = 19, /* big CPU Snoop Access Wakeup Request */
    LITTLE_SAC_WAKEUP_REQ_IRQ = 20, /* LITTLE CPU Snoop Access Wakeup Request */
    EXT_IF_WAKEUP_IRQ = 21, /* Extension Interface Access Wake-Up Request */
    EXT_IF_SERVICE_IRQ = 22, /* Extension Interface Access Service Request */
    EXT_IF_COHERENCY_IRQ = 23, /* Expansion Interface Coherency Req. */
    BIG_0_IRQ_WAKEUP_IRQ = 24, /* big CPU0 IRQ Wakeup Request */
    BIG_0_FIQ_WAKEUP_IRQ = 25, /* big CPU0 FIQ Wakeup Request */
    BIG_1_IRQ_WAKEUP_IRQ = 26, /* big CPU1 IRQ Wakeup Request */
    BIG_1_FIQ_WAKEUP_IRQ = 27, /* big CPU1 FIQ Wakeup Request */
    BIG_2_IRQ_WAKEUP_IRQ = 28, /* big CPU2 IRQ Wakeup Request */
    BIG_2_FIQ_WAKEUP_IRQ = 29, /* big CPU2 FIQ Wakeup Request */
    BIG_3_IRQ_WAKEUP_IRQ = 30, /* big CPU3 IRQ Wakeup Request */
    BIG_3_FIQ_WAKEUP_IRQ = 31, /* big CPU3 FIQ Wakeup Request */
    LITTLE_0_IRQ_WAKEUP_IRQ = 32, /* LITTLE CPU0 IRQ Wakeup Request */
    LITTLE_0_FIQ_WAKEUP_IRQ = 33, /* LITTLE CPU0 FIQ Wakeup Request */
    LITTLE_1_IRQ_WAKEUP_IRQ = 34, /* LITTLE CPU1 IRQ Wakeup Request */
    LITTLE_1_FIQ_WAKEUP_IRQ = 35, /* LITTLE CPU1 FIQ Wakeup Request */
    LITTLE_2_IRQ_WAKEUP_IRQ = 36, /* LITTLE CPU2 IRQ Wakeup Request */
    LITTLE_2_FIQ_WAKEUP_IRQ = 37, /* LITTLE CPU2 FIQ Wakeup Request */
    LITTLE_3_IRQ_WAKEUP_IRQ = 38, /* LITTLE CPU3 IRQ Wakeup Request */
    LITTLE_3_FIQ_WAKEUP_IRQ = 39, /* LITTLE CPU3 FIQ Wakeup Request */
    PPU_BIG_0_IRQ = 40, /* big CPU0 Power Policy Unit */
    PPU_BIG_1_IRQ = 41, /* big CPU1 Power Policy Unit */
    PPU_BIG_2_IRQ = 42, /* big CPU2 Power Policy Unit */
    PPU_BIG_3_IRQ = 43, /* big CPU3 Power Policy Unit */
    PPU_BIG_SSTOP_IRQ = 44, /* big SSTOP Power Policy Unit */
    PPU_LITTLE_0_IRQ = 45, /* LITTLE CPU0 Power Policy Unit */
    PPU_LITTLE_1_IRQ = 46, /* LITTLE CPU1 Power Policy Unit */
    PPU_LITTLE_2_IRQ = 47, /* LITTLE CPU2 Power Policy Unit */
    PPU_LITTLE_3_IRQ = 48, /* LITTLE CPU3 Power Policy Unit */
    PPU_LITTLE_SSTOP_IRQ = 49, /* LITTLE SSTOP Power Policy Unit */
    PPU_GPU_IRQ = 50, /* GPUTOP Sub-System Power Policy Unit */
    PPU_DBGSYS_IRQ = 51, /* Debug Sub-System Power Policy Unit */
    PPU_SYSTOP_IRQ = 52, /* SYSTOP Power Policy Unit */
    PLL_BIG_LOCK_IRQ = 53, /* big CPU PLL Lock */
    PLL_LITTLE_LOCK_IRQ = 54, /* LITTLE CPU PLL Lock */
    PLL_GPU_LOCK_IRQ = 55, /* GPU PLL Lock */
    PLL_SYS_LOCK_IRQ = 56, /* System PLL Lock */
    EXT_WAKEUP_IRQ = 57, /* External GIC Interrupt Wakeup */
    BIG_0_WARM_RST_REQ_IRQ = 58, /* big CPU0 Warm Reset Request */
    BIG_1_WARM_RST_REQ_IRQ = 59, /* big CPU1 Warm Reset Request */
    BIG_2_WARM_RST_REQ_IRQ = 60, /* big CPU2 Warm Reset Request */
    BIG_3_WARM_RST_REQ_IRQ = 61, /* big CPU3 Warm Reset Request */
    LITTLE_0_WARM_RST_REQ_IRQ = 62, /* LITTLE CPU0 Warm Reset Request */
    LITTLE_1_WARM_RST_REQ_IRQ = 63, /* LITTLE CPU1 Warm Reset Request */
    LITTLE_2_WARM_RST_REQ_IRQ = 64, /* LITTLE CPU2 Warm Reset Request */
    LITTLE_3_WARM_RST_REQ_IRQ = 65, /* LITTLE CPU3 Warm Reset Request */
    BIG_0_DBG_RST_REQ_IRQ = 66, /* big CPU0 Debug Reset Request */
    BIG_1_DBG_RST_REQ_IRQ = 67, /* big CPU1 Debug Reset Request */
    BIG_2_DBG_RST_REQ_IRQ = 68, /* big CPU2 Debug Reset Request */
    BIG_3_DBG_RST_REQ_IRQ = 69, /* big CPU3 Debug Reset Request */
    LITTLE_0_DBG_RST_REQ_IRQ = 70, /* LITTLE CPU0 Debug Reset Request */
    LITTLE_1_DBG_RST_REQ_IRQ = 71, /* LITTLE CPU1 Debug Reset Request */
    LITTLE_2_DBG_RST_REQ_IRQ = 72, /* LITTLE CPU2 Debug Reset Request */
    LITTLE_3_DBG_RST_REQ_IRQ = 73, /* LITTLE CPU3 Debug Reset Request */
    RESERVED74_IRQ = 74, /* Reserved */
    RESERVED75_IRQ = 75, /* Reserved */
    RESERVED76_IRQ = 76, /* Reserved */
    RESERVED77_IRQ = 77, /* Reserved */
    RESERVED78_IRQ = 78, /* Reserved */
    RESERVED79_IRQ = 79, /* Reserved */
    RESERVED80_IRQ = 80, /* Reserved */
    RESERVED81_IRQ = 81, /* Reserved */
    RESERVED82_IRQ = 82, /* Reserved */
    RESERVED83_IRQ = 83, /* Reserved */
    RESERVED84_IRQ = 84, /* Reserved */
    RESERVED85_IRQ = 85, /* Reserved */
    RESERVED86_IRQ = 86, /* Reserved */
    RESERVED87_IRQ = 87, /* Reserved */
    RESERVED88_IRQ = 88, /* Reserved */
    RESERVED89_IRQ = 89, /* Reserved */
    RESERVED90_IRQ = 90, /* Reserved */
    RESERVED91_IRQ = 91, /* Reserved */
    RESERVED92_IRQ = 92, /* Reserved */
    RESERVED93_IRQ = 93, /* Reserved */
    RESERVED94_IRQ = 94, /* Reserved */
    RESERVED95_IRQ = 95, /* Reserved */
    I2C_IRQ = 96, /* I2C Interrupt */
    LITTLE_PVT_IRQ = 97, /* LITTLE CPU PVT monitor Interrupt */
    BIG_PVT_IRQ = 98, /* big CPU PVT monitor Interrupt */
    GPU_PVT_IRQ = 99, /* GPU PVT monitor Interrupt */
    SOC_PVT_IRQ = 100, /* SoC PVT monitor Interrupt */
    SCP_EXT_INTR5_IRQ = 101, /* SCP Customer Extension */
    STD_CELL_PVT_IRQ = 102, /* Std Cell PVT monitor Interrupt */
    SCP_EXT_INTR7_IRQ = 103, /* SCP Customer Extension */
    PHY_TRAINING_IRQ = 104, /* PHY Training Interrupt */
    SCP_EXT_INTR9_IRQ = 105, /* SCP Customer Extension */
    SCP_EXT_INTR10_IRQ = 106, /* SCP Customer Extension */
    SCP_EXT_INTR11_IRQ = 107, /* SCP Customer Extension */
    SCP_EXT_INTR12_IRQ = 108, /* SCP Customer Extension */
    SCP_EXT_INTR13_IRQ = 109, /* SCP Customer Extension */
    SCP_EXT_INTR14_IRQ = 110, /* SCP Customer Extension */
    SCP_EXT_INTR15_IRQ = 111, /* SCP Customer Extension */
    SCP_EXT_INTR16_IRQ = 112, /* SCP Customer Extension */
    SCP_EXT_INTR17_IRQ = 113, /* SCP Customer Extension */
    SCP_EXT_INTR18_IRQ = 114, /* SCP Customer Extension */
    SCP_EXT_INTR19_IRQ = 115, /* SCP Customer Extension */
    SCP_EXT_INTR20_IRQ = 116, /* SCP Customer Extension */
    SCP_EXT_INTR21_IRQ = 117, /* SCP Customer Extension */
    SCP_EXT_INTR22_IRQ = 118, /* SCP Customer Extension */
    SCP_EXT_INTR23_IRQ = 119, /* SCP Customer Extension */
    SCP_EXT_INTR24_IRQ = 120, /* SCP Customer Extension */
    SCP_EXT_INTR25_IRQ = 121, /* SCP Customer Extension */
    SCP_EXT_INTR26_IRQ = 122, /* SCP Customer Extension */
    SCP_EXT_INTR27_IRQ = 123, /* SCP Customer Extension */
    SCP_EXT_INTR28_IRQ = 124, /* SCP Customer Extension */
    SCP_EXT_INTR29_IRQ = 125, /* SCP Customer Extension */
    SCP_EXT_INTR30_IRQ = 126, /* SCP Customer Extension */
    SCP_EXT_INTR31_IRQ = 127, /* SCP Customer Extension */

    IRQn_MAX = INT16_MAX,
} IRQn_Type;

#include <core_cm3.h>

#endif /* FMW_CMSIS_H */
