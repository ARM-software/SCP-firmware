/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_CMSIS_MCP_H
#define FMW_CMSIS_MCP_H

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

    RESERVED0_IRQ = 0, /* Reserved */
    CDBG_PWR_UP_REQ_IRQ = 1, /* Coresight Debug Power Request */
    CSYS_PWR_UP_REQ_IRQ = 2, /* Coresight System Power Request */
    CDBG_RST_REQ_IRQ = 3, /* Coresight Debug Reset Request */
    GIC_EXT_WAKEUP_IRQ = 4, /* External GIC Wakeup Request */
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
    MCP_PIK_IRQ = 32, /* Power Intergration Kit Interrupt */
    TIMREFCLK_IRQ = 33, /* REFCLK Physical Timer */
    MHU_AP_NONSEC_IRQ = 34, /* MHU Non-Secure IRQ between MCP and AP */
    RESERVED35_IRQ = 35, /* Reserved */
    MHU_AP_SEC_IRQ = 36, /* MHU Secure IRQ between MCP and AP */
    CTI_TRIGGER0_IRQ = 37, /* MCP CTI Trigger */
    CTI_TRIGGER1_IRQ = 38, /* MCP CTI Trigger */
    RESERVED39_IRQ = 39, /* Reserved */
    RESERVED40_IRQ = 40, /* Reserved */
    RESERVED41_IRQ = 41, /* Reserved */
    MCP_UART0_INT_IRQ = 42, /* Always-on UART interrupt */
    MCP_UART1_INT_IRQ = 43, /* Always-on UART interrupt */
    RESERVED44_IRQ = 44, /* Reserved */
    RESERVED45_IRQ = 45, /* Reserved */
    RESERVED46_IRQ = 46, /* Reserved */
    RESERVED47_IRQ = 47, /* Reserved */
    RESERVED48_IRQ = 48, /* Reserved */
    RESERVED49_IRQ = 49, /* Reserved */
    RESERVED50_IRQ = 50, /* Reserved */
    RESERVED51_IRQ = 51, /* Reserved */
    RESERVED52_IRQ = 52, /* Reserved */
    RESERVED53_IRQ = 53, /* Reserved */
    RESERVED54_IRQ = 54, /* Reserved */
    RESERVED55_IRQ = 55, /* Reserved */
    RESERVED56_IRQ = 56, /* Reserved */
    RESERVED57_IRQ = 57, /* Reserved */
    RESERVED58_IRQ = 58, /* Reserved */
    RESERVED59_IRQ = 59, /* Reserved */
    RESERVED60_IRQ = 60, /* Reserved */
    RESERVED61_IRQ = 61, /* Reserved */
    RESERVED62_IRQ = 62, /* Reserved */
    RESERVED63_IRQ = 63, /* Reserved */
    RESERVED64_IRQ = 64, /* Reserved */
    RESERVED65_IRQ = 65, /* Reserved */
    RESERVED66_IRQ = 66, /* Reserved */
    RESERVED67_IRQ = 67, /* Reserved */
    RESERVED68_IRQ = 68, /* Reserved */
    RESERVED69_IRQ = 69, /* Reserved */
    RESERVED70_IRQ = 70, /* Reserved */
    RESERVED71_IRQ = 71, /* Reserved */
    RESERVED72_IRQ = 72, /* Reserved */
    RESERVED73_IRQ = 73, /* Reserved */
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
    MHU_SCP_NONSEC_IRQ = 84, /* MHU Non-Sec IRQ between SCP and MCP */
    MHU_SCP_SEC_IRQ = 85, /* MHU Secure IRQ between SCP and MCP */
    RESERVED86_IRQ = 86, /* Reserved */
    RESERVED87_IRQ = 87, /* Reserved */
    RESERVED88_IRQ = 88, /* Reserved */
    RESERVED89_IRQ = 89, /* Reserved */
    RESERVED90_IRQ = 90, /* Reserved */
    RESERVED91_IRQ = 91, /* Reserved */
    RESERVED92_IRQ = 92, /* Reserved */
    RESERVED93_IRQ = 93, /* Reserved */
    MMU_TCU_RASIRPT_IRQ = 94, /* Consolidated MMU RAS */
    MMU_TBU_RASIRPT_IRQ = 95, /* Consolidated TBU RAS */
    INT_PPU_IRQ = 96, /* PPU interrupt from Interconnect PPU */
    INT_ERRNS_IRQ = 97, /* Non-Sec error interrupt from Interconnect PPU */
    INT_ERRS_IRQ = 98, /* Secure error interrupt from Interconnect PPU */
    INT_FAULTS_IRQ = 99, /* Secure fault interrupt from Interconnect PPU */
    INT_FAULTNS_IRQ = 100, /* Non-Sec fault interrupt from Interconnect PPU */
    INT_PMU_IRQ = 101, /* PMU count overflow interrupt */
    RESERVED102_IRQ = 102, /* Reserved */
    RESERVED103_IRQ = 103, /* Reserved */
    RESERVED104_IRQ = 104, /* Reserved */
    RESERVED105_IRQ = 105, /* Reserved */
    RESERVED106_IRQ = 106, /* Reserved */
    RESERVED107_IRQ = 107, /* Reserved */
    RESERVED108_IRQ = 108, /* Reserved */
    RESERVED109_IRQ = 109, /* Reserved */
    RESERVED110_IRQ = 110, /* Reserved */
    RESERVED111_IRQ = 111, /* Reserved */
    RESERVED112_IRQ = 112, /* Reserved */
    RESERVED113_IRQ = 113, /* Reserved */
    RESERVED114_IRQ = 114, /* Reserved */
    RESERVED115_IRQ = 115, /* Reserved */
    RESERVED116_IRQ = 116, /* Reserved */
    RESERVED117_IRQ = 117, /* Reserved */
    RESERVED118_IRQ = 118, /* Reserved */
    RESERVED119_IRQ = 119, /* Reserved */
    RESERVED120_IRQ = 120, /* Reserved */
    RESERVED121_IRQ = 121, /* Reserved */
    RESERVED122_IRQ = 122, /* Reserved */
    RESERVED123_IRQ = 123, /* Reserved */
    RESERVED124_IRQ = 124, /* Reserved */
    RESERVED125_IRQ = 125, /* Reserved */
    RESERVED126_IRQ = 126, /* Reserved */
    RESERVED127_IRQ = 127, /* Reserved */
    RESERVED128_IRQ = 128, /* Reserved */
    RESERVED129_IRQ = 129, /* Reserved */
    RESERVED130_IRQ = 130, /* Reserved */
    RESERVED131_IRQ = 131, /* Reserved */
    RESERVED132_IRQ = 132, /* Reserved */
    RESERVED133_IRQ = 133, /* Reserved */
    RESERVED134_IRQ = 134, /* Reserved */
    RESERVED135_IRQ = 135, /* Reserved */
    RESERVED136_IRQ = 136, /* Reserved */
    RESERVED137_IRQ = 137, /* Reserved */
    RESERVED138_IRQ = 138, /* Reserved */
    MCP_WD_WS1_IRQ = 139, /* MCP watchdog reset */
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
    RESERVED160_IRQ = 160, /* Reserved */
    RESERVED161_IRQ = 161, /* Reserved */
    RESERVED162_IRQ = 162, /* Reserved */
    RESERVED163_IRQ = 163, /* Reserved */
    RESERVED164_IRQ = 164, /* Reserved */
    RESERVED165_IRQ = 165, /* Reserved */
    RESERVED166_IRQ = 166, /* Reserved */
    RESERVED167_IRQ = 167, /* Reserved */
    RESERVED168_IRQ = 168, /* Reserved */
    RESERVED169_IRQ = 169, /* Reserved */
    RESERVED170_IRQ = 170, /* Reserved */
    RESERVED171_IRQ = 171, /* Reserved */
    RESERVED172_IRQ = 172, /* Reserved */
    RESERVED173_IRQ = 173, /* Reserved */
    RESERVED174_IRQ = 174, /* Reserved */
    RESERVED175_IRQ = 175, /* Reserved */
    RESERVED176_IRQ = 176, /* Reserved */
    RESERVED177_IRQ = 177, /* Reserved */
    RESERVED178_IRQ = 178, /* Reserved */
    RESERVED179_IRQ = 179, /* Reserved */
    DMCS0_MISC_OFLOW_IRQ = 180, /* DMC 0/4 Combined Error Overflow */
    DMCS0_ERR_OFLOW_IRQ = 181, /* DMC 0/4 Error Overflow */
    DMCS0_ECC_ERR_INT_IRQ = 182, /* DMC 0/4 ECC Error Int */
    DMCS0_MISC_ACCESS_INT_IRQ =
        183, /* DMC 0/4 Combined Miscellaneous access int */
    DMCS0_TEMPERATURE_EVENT_INT_IRQ = 184, /* DMC 0/4 Temperature event int */
    DMCS0_FAILED_ACCESS_INT_IRQ = 185, /* DMC 0/4 Failed access int */
    DMCS0_MGR_INT_IRQ = 186, /* DMC 0/4 combined manager int */
    DMCS1_MISC_OFLOW_IRQ = 187, /* DMC 1/5 Combined Error Overflow */
    DMCS1_ERR_OFLOW_IRQ = 188, /* DMC 1/5 Error Overflow */
    DMCS1_ECC_ERR_INT_IRQ = 189, /* DMC 1/5 ECC Error Int */
    DMCS1_MISC_ACCESS_INT_IRQ =
        190, /* DMC 1/5 Combined Miscellaneous access int */
    DMCS1_TEMPERATURE_EVENT_INT_IRQ = 191, /* DMC 1/5 Temperature event int */
    DMCS1_FAILED_ACCESS_INT_IRQ = 192, /* DMC 1/5 Failed access int */
    DMCS1_MGR_INT_IRQ = 193, /* DMC 1/5 combined manager int */
    DMCS2_MISC_OFLOW_IRQ = 194, /* DMC 2/6 Combined Error Overflow */
    DMCS2_ERR_OFLOW_IRQ = 195, /* DMC 2/6 Error Overflow */
    DMCS2_ECC_ERR_INT_IRQ = 196, /* DMC 2/6 ECC Error Int */
    DMCS2_MISC_ACCESS_INT_IRQ =
        197, /* DMC 2/6 Combined Miscellaneous access int */
    DMCS2_TEMPERATURE_EVENT_INT_IRQ = 198, /* DMC 2/6 Temperature event int */
    DMCS2_FAILED_ACCESS_INT_IRQ = 199, /* DMC 2/6 Failed access int */
    DMCS2_MGR_INT_IRQ = 200, /* DMC 2/6 combined manager int */
    DMCS3_MISC_OFLOW_IRQ = 201, /* DMC 3/7 Combined Error Overflow */
    DMCS3_ERR_OFLOW_IRQ = 202, /* DMC 3/7 Error Overflow */
    DMCS3_ECC_ERR_INT_IRQ = 203, /* DMC 3/7 ECC Error Int */
    DMCS3_MISC_ACCESS_INT_IRQ =
        204, /* DMC 3/7 Combined Miscellaneous access int */
    DMCS3_TEMPERATURE_EVENT_INT_IRQ = 205, /* DMC 3/7 Temperature event int */
    DMCS3_FAILED_ACCESS_INT_IRQ = 206, /* DMC 3/7 Failed access int */
    DMCS3_MGR_INT_IRQ = 207, /* DMC 3/7 combined manager int */
    MCP_EXT_INTR0_IRQ = 208, /* MCP Customer Extension */
    MCP_EXT_INTR1_IRQ = 209, /* MCP Customer Extension */
    MCP_EXT_INTR2_IRQ = 210, /* MCP Customer Extension */
    MCP_EXT_INTR3_IRQ = 211, /* MCP Customer Extension */
    MCP_EXT_INTR4_IRQ = 212, /* MCP Customer Extension */
    MCP_EXT_INTR5_IRQ = 213, /* MCP Customer Extension */
    MCP_EXT_INTR6_IRQ = 214, /* MCP Customer Extension */
    MCP_EXT_INTR7_IRQ = 215, /* MCP Customer Extension */
    MCP_EXT_INTR8_IRQ = 216, /* MCP Customer Extension */
    MCP_EXT_INTR9_IRQ = 217, /* MCP Customer Extension */
    MCP_EXT_INTR10_IRQ = 218, /* MCP Customer Extension */
    MCP_EXT_INTR11_IRQ = 219, /* MCP Customer Extension */
    MCP_EXT_INTR12_IRQ = 220, /* MCP Customer Extension */
    MCP_EXT_INTR13_IRQ = 221, /* MCP Customer Extension */
    MCP_EXT_INTR14_IRQ = 222, /* MCP Customer Extension */
    MCP_EXT_INTR15_IRQ = 223, /* MCP Customer Extension */
    MCP_EXT_INTR16_IRQ = 224, /* MCP Customer Extension */
    MCP_EXT_INTR17_IRQ = 225, /* MCP Customer Extension */
    MCP_EXT_INTR18_IRQ = 226, /* MCP Customer Extension */
    MCP_EXT_INTR19_IRQ = 227, /* MCP Customer Extension */
    MCP_EXT_INTR20_IRQ = 228, /* MCP Customer Extension */
    MCP_EXT_INTR21_IRQ = 229, /* MCP Customer Extension */
    MCP_EXT_INTR22_IRQ = 230, /* MCP Customer Extension */
    MCP_EXT_INTR23_IRQ = 231, /* MCP Customer Extension */
    MCP_EXT_INTR24_IRQ = 232, /* MCP Customer Extension */
    MCP_EXT_INTR25_IRQ = 233, /* MCP Customer Extension */
    MCP_EXT_INTR26_IRQ = 234, /* MCP Customer Extension */
    MCP_EXT_INTR27_IRQ = 235, /* MCP Customer Extension */
    MCP_EXT_INTR28_IRQ = 236, /* MCP Customer Extension */
    MCP_EXT_INTR29_IRQ = 237, /* MCP Customer Extension */
    MCP_EXT_INTR30_IRQ = 238, /* MCP Customer Extension */
    MCP_EXT_INTR31_IRQ = 239, /* MCP Customer Extension */

    IRQn_MAX = INT16_MAX,
} IRQn_Type;

#include <core_cm7.h>

#endif /* FMW_CMSIS_MCP_H */
