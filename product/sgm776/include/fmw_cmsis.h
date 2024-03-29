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

    TIM32KHZ_IRQ = 0, /* 32KHz Physical Timer */
    CDBG_PWR_UP_REQ_IRQ = 1, /* Coresight Debug Power Request */
    CSYS_PWR_UP_REQ_IRQ = 2, /* Coresight System Power Request */
    CDBG_RST_REQ_IRQ = 3, /* Coresight Debug Reset Request */
    RESERVED4_IRQ = 4, /* Reserved */
    RESERVED5_IRQ = 5, /* Reserved */
    RESERVED6_IRQ = 6, /* Reserved */
    RESERVED7_IRQ = 7, /* Reserved */
    RESERVED8_IRQ = 8, /* Reserved */
    RESERVED9_IRQ = 9, /* Reserved */
    RESERVED10_IRQ = 10, /* Reserved */
    RESERVED11_IRQ = 11, /* Reserved */
    RESERVED12_IRQ = 12, /* Reserved */
    RESERVED13_IRQ = 13, /* Reserved */
    RESERVED14_IRQ = 14, /* Reserved */
    RESERVED15_IRQ = 15, /* Reserved */
    SOC_WAKEUP0_IRQ = 16, /* SoC Expansion Wakeup */
    SOC_WAKEUP1_IRQ = 17, /* SoC Expansion Wakeup */
    SOC_WAKEUP2_IRQ = 18, /* SoC Expansion Wakeup */
    SOC_WAKEUP3_IRQ = 19, /* SoC Expansion Wakeup */
    SOC_WAKEUP4_IRQ = 20, /* SoC Expansion Wakeup */
    SOC_WAKEUP5_IRQ = 21, /* SoC Expansion Wakeup */
    SOC_WAKEUP6_IRQ = 22, /* SoC Expansion Wakeup */
    SOC_WAKEUP7_IRQ = 23, /* SoC Expansion Wakeup */
    SOC_WAKEUP8_IRQ = 24, /* SoC Expansion Wakeup */
    SOC_WAKEUP9_IRQ = 25, /* SoC Expansion Wakeup */
    SOC_WAKEUP10_IRQ = 26, /* SoC Expansion Wakeup */
    SOC_WAKEUP11_IRQ = 27, /* SoC Expansion Wakeup */
    SOC_WAKEUP12_IRQ = 28, /* SoC Expansion Wakeup */
    SOC_WAKEUP13_IRQ = 29, /* SoC Expansion Wakeup */
    SOC_WAKEUP14_IRQ = 30, /* SoC Expansion Wakeup */
    SOC_WAKEUP15_IRQ = 31, /* SoC Expansion Wakeup */
    PPU_SCP_IRQ = 32, /* SCP Power Policy Unit */
    TIMREFCLK_IRQ = 33, /* REFCLK Physical Timer */
    MHU_HIGH_PRIO_IRQ = 34, /* MHU High Priority */
    MHU_LOW_PRIO_IRQ = 35, /* MHU Low Priority */
    MHU_SECURE_IRQ = 36, /* MHU Secure */
    CTI_TRIGGER0_IRQ = 37, /* SCP CTI Trigger */
    CTI_TRIGGER1_IRQ = 38, /* SCP CTI Trigger */
    GIC_ERROR_ECC_IRQ = 39, /* GIC Error (ECC Fatal) */
    GIC_ERROR_AXIM_IRQ = 40, /* GIC Error (AXIM) */
    DMC_RESERVED0_IRQ = 41, /* DMC, Reserved */
    DMC_0_ERROR_ECC_IRQ = 42, /* DMC0 Combined ECC Error */
    DMC_0_ERROR_ACCESS_IRQ = 43, /* DMC0 Combined Misc Access Error */
    DMC_RESERVED1_IRQ = 44, /* DMC, Reserved */
    DMC_RESERVED2_IRQ = 45, /* DMC, Reserved */
    DMC_1_ERROR_ECC_IRQ = 46, /* DMC1 Combined ECC Error */
    DMC_1_ERROR_ACCESS_IRQ = 47, /* DMC1 Combined Misc Access Error */
    DMC_RESERVED3_IRQ = 48, /* DMC, Reserved */
    DMC_RESERVED4_IRQ = 49, /* DMC, Reserved */
    DMC_2_ERROR_ECC_IRQ = 50, /* DMC2 Combined ECC Error */
    DMC_2_ERROR_ACCESS_IRQ = 51, /* DMC2 Combined Misc Access Error */
    DMC_RESERVED5_IRQ = 52, /* DMC, Reserved */
    DMC_RESERVED6_IRQ = 53, /* DMC, Reserved */
    DMC_3_ERROR_ECC_IRQ = 54, /* DMC3 Combined ECC Error */
    DMC_3_ERROR_ACCESS_IRQ = 55, /* DMC3 Combined Misc Access Error */
    DMC_RESERVED7_IRQ = 56, /* DMC, Reserved */
    RESERVED57_IRQ = 57, /* Reserved */
    RESERVED58_IRQ = 58, /* Reserved */
    RESERVED59_IRQ = 59, /* Reserved */
    RESERVED60_IRQ = 60, /* Reserved */
    RESERVED61_IRQ = 61, /* Reserved */
    RESERVED62_IRQ = 62, /* Reserved */
    RESERVED63_IRQ = 63, /* Reserved */
    PPU_CLUS0CORE0_IRQ = 64, /* Cluster 0 Core 0 Power Policy Unit */
    PPU_CLUS0CORE1_IRQ = 65, /* Cluster 0 Core 1 Power Policy Unit */
    PPU_CLUS0CORE2_IRQ = 66, /* Cluster 0 Core 2 Power Policy Unit */
    PPU_CLUS0CORE3_IRQ = 67, /* Cluster 0 Core 3 Power Policy Unit */
    PPU_CLUS0_IRQ = 68, /* Cluster 0 Power Policy Unit */
    PPU_CLUS1CORE0_IRQ = 69, /* Cluster 1 Core 0 Power Policy Unit */
    PPU_CLUS1CORE1_IRQ = 70, /* Cluster 1 Core 1 Power Policy Unit */
    PPU_CLUS1CORE2_IRQ = 71, /* Cluster 1 Core 2 Power Policy Unit */
    PPU_CLUS1CORE3_IRQ = 72, /* Cluster 1 Core 3 Power Policy Unit */
    PPU_CLUS1_IRQ = 73, /* Cluster 1 Power Policy Unit */
    PPU_SYS0_IRQ = 74, /* System Power Policy Unit 0 */
    PPU_SYS1_IRQ = 75, /* System Power Policy Unit 1 */
    PPU_GPU_IRQ = 76, /* GPU Power Policy Unit */
    PPU_VPU_IRQ = 77, /* Video Power Policy Unit */
    PPU_DPU_IRQ = 78, /* Display Power Policy Unit 0 */
    RESERVED79_IRQ = 79, /* Reserved */
    RESERVED80_IRQ = 80, /* Reserved */
    PPU_DEBUG_IRQ = 81, /* DBGSYS Power Policy Unit */
    PPU_DEBUG_CHAIN_IRQ = 82, /* Debug chain Power Policy Unit */
    RESERVED83_IRQ = 83, /* Reserved */
    RESERVED84_IRQ = 84, /* Reserved */
    RESERVED85_IRQ = 85, /* Reserved */
    RESERVED86_IRQ = 86, /* Reserved */
    RESERVED87_IRQ = 87, /* Reserved */
    RESERVED88_IRQ = 88, /* Reserved */
    RESERVED89_IRQ = 89, /* Reserved */
    PPU_CLUS0CORE4_IRQ = 90, /* Cluster 0 Core 4 Power Policy Unit */
    PPU_CLUS0CORE5_IRQ = 91, /* Cluster 0 Core 5 Power Policy Unit */
    PPU_CLUS0CORE6_IRQ = 92, /* Cluster 0 Core 6 Power Policy Unit */
    PPU_CLUS0CORE7_IRQ = 93, /* Cluster 0 Core 7 Power Policy Unit */
    PPU_CLUS1CORE4_IRQ = 94, /* Cluster 1 Core 4 Power Policy Unit */
    PPU_CLUS1CORE5_IRQ = 95, /* Cluster 1 Core 5 Power Policy Unit */
    PPU_CLUS1CORE6_IRQ = 96, /* Cluster 1 Core 6 Power Policy Unit */
    PPU_CLUS1CORE7_IRQ = 97, /* Cluster 1 Core 7 Power Policy Unit */
    PLL_CLUS0_LOCK_IRQ = 98, /* Cluster 0 CPU PLL Lock */
    PLL_CLUS1_LOCK_IRQ = 99, /* Cluster 1 CPU PLL Lock */
    PLL_GPU_LOCK_IRQ = 100, /* GPU PLL Lock */
    PLL_VPU_LOCK_IRQ = 101, /* Video PLL Lock */
    PLL_SYS_LOCK_IRQ = 102, /* System PLL Lock */
    PLL_DPU_LOCK_IRQ = 103, /* Display PLL Lock */
    PLL_CLUS0CORE0_IRQ = 104, /* Cluster 0 PLL0 Lock */
    PLL_CLUS0CORE1_IRQ = 105, /* Cluster 0 PLL1 Lock */
    PLL_CLUS0CORE2_IRQ = 106, /* Cluster 0 PLL2 Lock */
    PLL_CLUS0CORE3_IRQ = 107, /* Cluster 0 PLL3 Lock */
    PLL_CLUS0CORE4_IRQ = 108, /* Cluster 0 PLL4 Lock */
    PLL_CLUS0CORE5_IRQ = 109, /* Cluster 0 PLL5 Lock */
    PLL_CLUS0CORE6_IRQ = 110, /* Cluster 0 PLL6 Lock */
    PLL_CLUS0CORE7_IRQ = 111, /* Cluster 0 PLL7 Lock */
    PLL_CLUS1CORE0_IRQ = 112, /* Cluster 1 PLL0 Lock */
    PLL_CLUS1CORE1_IRQ = 113, /* Cluster 1 PLL1 Lock */
    PLL_CLUS1CORE2_IRQ = 114, /* Cluster 1 PLL2 Lock */
    PLL_CLUS1CORE3_IRQ = 115, /* Cluster 1 PLL3 Lock */
    PLL_CLUS1CORE4_IRQ = 116, /* Cluster 1 PLL4 Lock */
    PLL_CLUS1CORE5_IRQ = 117, /* Cluster 1 PLL5 Lock */
    PLL_CLUS1CORE6_IRQ = 118, /* Cluster 1 PLL6 Lock */
    PLL_CLUS1CORE7_IRQ = 119, /* Cluster 1 PLL7 Lock */
    DBG_PWR_REQ0_IRQ = 120, /* Debug power request 0 */
    DBG_PWR_REQ1_IRQ = 121, /* Debug power request 1 */
    DBG_PWR_REQ2_IRQ = 122, /* Debug power request 2 */
    DBG_PWR_REQ3_IRQ = 123, /* Debug power request 3 */
    DBG_RST_REQ_IRQ = 124, /* Debug reset request */
    SYS_PWR_REQ0_IRQ = 125, /* System power request 0 */
    SYS_PWR_REQ1_IRQ = 126, /* System power request 1 */
    SYS_PWR_REQ2_IRQ = 127, /* System power request 2 */
    SYS_PWR_REQ3_IRQ = 128, /* System power request 3 */
    SYS_RST_REQ_IRQ = 129, /* System reset request */
    DBG_CHN_PWRUP_IRQ = 130, /* Debug chain power up */
    RESERVED131_IRQ = 131, /* Reserved */
    RESERVED132_IRQ = 132, /* Reserved */
    RESERVED133_IRQ = 133, /* Reserved */
    RESERVED134_IRQ = 134, /* Reserved */
    RESERVED135_IRQ = 135, /* Reserved */
    RESERVED136_IRQ = 136, /* Reserved */
    RESERVED137_IRQ = 137, /* Reserved */
    RESERVED138_IRQ = 138, /* Reserved */
    RESERVED139_IRQ = 139, /* Reserved */
    RESERVED140_IRQ = 140, /* Reserved */
    RESERVED141_IRQ = 141, /* Reserved */
    RESERVED142_IRQ = 142, /* Reserved */
    RESERVED143_IRQ = 143, /* Reserved */
    RESERVED144_IRQ = 144, /* Reserved */
    RESERVED145_IRQ = 145, /* Reserved */
    RESERVED146_IRQ = 146, /* Reserved */
    RESERVED147_IRQ = 147, /* Reserved */
    RESERVED148_IRQ = 148, /* Reserved */
    RESERVED149_IRQ = 149, /* Reserved */
    RESERVED150_IRQ = 150, /* Reserved */
    RESERVED151_IRQ = 151, /* Reserved */
    RESERVED152_IRQ = 152, /* Reserved */
    RESERVED153_IRQ = 153, /* Reserved */
    RESERVED154_IRQ = 154, /* Reserved */
    RESERVED155_IRQ = 155, /* Reserved */
    RESERVED156_IRQ = 156, /* Reserved */
    RESERVED157_IRQ = 157, /* Reserved */
    RESERVED158_IRQ = 158, /* Reserved */
    RESERVED159_IRQ = 159, /* Reserved */
    SCP_EXT_INTR0_IRQ = 160, /* SCP Customer Extension */
    SCP_EXT_INTR1_IRQ = 161, /* SCP Customer Extension */
    SCP_EXT_INTR2_IRQ = 162, /* SCP Customer Extension */
    SCP_EXT_INTR3_IRQ = 163, /* SCP Customer Extension */
    SCP_EXT_INTR4_IRQ = 164, /* SCP Customer Extension */
    SCP_EXT_INTR5_IRQ = 165, /* SCP Customer Extension */
    SCP_EXT_INTR6_IRQ = 166, /* SCP Customer Extension */
    SCP_EXT_INTR7_IRQ = 167, /* SCP Customer Extension */
    SCP_EXT_INTR8_IRQ = 168, /* SCP Customer Extension */
    SCP_EXT_INTR9_IRQ = 169, /* SCP Customer Extension */
    SCP_EXT_INTR10_IRQ = 170, /* SCP Customer Extension */
    SCP_EXT_INTR11_IRQ = 171, /* SCP Customer Extension */
    SCP_EXT_INTR12_IRQ = 172, /* SCP Customer Extension */
    SCP_EXT_INTR13_IRQ = 173, /* SCP Customer Extension */
    SCP_EXT_INTR14_IRQ = 174, /* SCP Customer Extension */
    SCP_EXT_INTR15_IRQ = 175, /* SCP Customer Extension */
    SCP_EXT_INTR16_IRQ = 176, /* SCP Customer Extension */
    SCP_EXT_INTR17_IRQ = 177, /* SCP Customer Extension */
    SCP_EXT_INTR18_IRQ = 178, /* SCP Customer Extension */
    SCP_EXT_INTR19_IRQ = 179, /* SCP Customer Extension */
    SCP_EXT_INTR20_IRQ = 180, /* SCP Customer Extension */
    SCP_EXT_INTR21_IRQ = 181, /* SCP Customer Extension */
    SCP_EXT_INTR22_IRQ = 182, /* SCP Customer Extension */
    SCP_EXT_INTR23_IRQ = 183, /* SCP Customer Extension */
    SCP_EXT_INTR24_IRQ = 184, /* SCP Customer Extension */
    SCP_EXT_INTR25_IRQ = 185, /* SCP Customer Extension */
    SCP_EXT_INTR26_IRQ = 186, /* SCP Customer Extension */
    SCP_EXT_INTR27_IRQ = 187, /* SCP Customer Extension */
    SCP_EXT_INTR28_IRQ = 188, /* SCP Customer Extension */
    SCP_EXT_INTR29_IRQ = 189, /* SCP Customer Extension */
    SCP_EXT_INTR30_IRQ = 190, /* SCP Customer Extension */
    SCP_EXT_INTR31_IRQ = 191, /* SCP Customer Extension */

    IRQn_MAX = INT16_MAX,
} IRQn_Type;

#include <core_cm3.h>

#endif /* FMW_CMSIS_H */
