/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_CMSIS_MCP_H
#define FMW_CMSIS_MCP_H

#define __CHECK_DEVICE_DEFINES
#define __CM7_REV 0x0000U
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
    MCP_EXT_IRQ = 16, /* MCP External IRQ */
    GPIO_COMBINED_IRQ = 17, /* GPIO Combined IRQ */
    GPIO_0_IRQ = 18, /* GPIO 0 IRQ */
    GPIO_1_IRQ = 19, /* GPIO 1 IRQ */
    GPIO_2_IRQ = 20, /* GPIO 2 IRQ */
    GPIO_3_IRQ = 21, /* GPIO 3 IRQ */
    GPIO_4_IRQ = 22, /* GPIO 4 IRQ */
    GPIO_5_IRQ = 23, /* GPIO 5 IRQ */
    GPIO_6_IRQ = 24, /* GPIO 6 IRQ */
    GPIO_7_IRQ = 25, /* GPIO 7 IRQ */
    RESERVED26_IRQ = 26, /* Reserved */
    RESERVED27_IRQ = 27, /* Reserved */
    RESERVED28_IRQ = 28, /* Reserved */
    RESERVED29_IRQ = 29, /* Reserved */
    RESERVED30_IRQ = 30, /* Reserved */
    RESERVED31_IRQ = 31, /* Reserved */
    RESERVED32_IRQ = 32, /* Reserved */
    TIMREFCLK_IRQ = 33, /* REFCLK Physical Timer */
    MHU_AP_NONSEC_IRQ = 34, /* MHU non-secure irq from AP to MCP */
    RESERVED35_IRQ = 35, /* Reserved */
    MHU_AP_SEC_IRQ = 36, /* MHU secure irq from AP to MCP */
    CTI_TRIGGER0_IRQ = 37, /* MCP CTI0 Trigger */
    CTI_TRIGGER1_IRQ = 38, /* MCP CTI1 Trigger */
    RESERVED39_IRQ = 39, /* Reserved */
    RESERVED40_IRQ = 40, /* Reserved */
    RESERVED41_IRQ = 41, /* Reserved */
    MCP_UART0_IRQ = 42, /* MCP UART0 */
    MCP_UART1_IRQ = 43, /* MCP UART1 */
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
    MHU_SCP_NONSEC_IRQ = 84, /* MHU non-secure irq from SCP to MCP */
    MHU_SCP_SEC_IRQ = 85, /* MHU secure irq from SCP to MCP */
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
    PMU_CNT_OVF_IRQ = 101, /* PMU count overflow irq */
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
    DMCS0_MISC_OFLOW_IRQ = 180, /* DMC 0/4 Combined Misc Overflow */
    DMCS0_ERR_OFLOW_IRQ = 181, /* DMC 0/4 Error Overflow */
    DMCS0_ECC_ERR_INT_IRQ = 182, /* DMC 0/4 ECC Error Int */
    DMCS0_MISC_ACCESS_INT_IRQ =
        183, /* DMC 0/4 Combined Miscellaneous access int */
    DMCS0_TEMPERATURE_EVENT_INT_IRQ = 184, /* DMC 0/4 Temperature event int */
    DMCS0_FAILED_ACCESS_INT_IRQ = 185, /* DMC 0/4 Failed access int */
    DMCS0_MGR_INT_IRQ = 186, /* DMC 0/4 combined manager int */
    DMCS1_MISC_OFLOW_IRQ = 187, /* DMC 1/5 Combined Misc Overflow */
    DMCS1_ERR_OFLOW_IRQ = 188, /* DMC 1/5 Error Overflow */
    DMCS1_ECC_ERR_INT_IRQ = 189, /* DMC 1/5 ECC Error Int */
    DMCS1_MISC_ACCESS_INT_IRQ =
        190, /* DMC 1/5 Combined Miscellaneous access int */
    DMCS1_TEMPERATURE_EVENT_INT_IRQ = 191, /* DMC 1/5 Temperature event int */
    DMCS1_FAILED_ACCESS_INT_IRQ = 192, /* DMC 1/5 Failed access int */
    DMCS1_MGR_INT_IRQ = 193, /* DMC 1/5 combined manager int */
    DMCS2_MISC_OFLOW_IRQ = 194, /* DMC 2/6 Combined Misc Overflow */
    DMCS2_ERR_OFLOW_IRQ = 195, /* DMC 2/6 Error Overflow */
    DMCS2_ECC_ERR_INT_IRQ = 196, /* DMC 2/6 ECC Error Int */
    DMCS2_MISC_ACCESS_INT_IRQ =
        197, /* DMC 2/6 Combined Miscellaneous access int */
    DMCS2_TEMPERATURE_EVENT_INT_IRQ = 198, /* DMC 2/6 Temperature event int */
    DMCS2_FAILED_ACCESS_INT_IRQ = 199, /* DMC 2/6 Failed access int */
    DMCS2_MGR_INT_IRQ = 200, /* DMC 2/6 combined manager int */
    DMCS3_MISC_OFLOW_IRQ = 201, /* DMC 3/7 Combined Misc Overflow */
    DMCS3_ERR_OFLOW_IRQ = 202, /* DMC 3/7 Error Overflow */
    DMCS3_ECC_ERR_INT_IRQ = 203, /* DMC 3/7 ECC Error Int */
    DMCS3_MISC_ACCESS_INT_IRQ =
        204, /* DMC 3/7 Combined Miscellaneous access int */
    DMCS3_TEMPERATURE_EVENT_INT_IRQ = 205, /* DMC 3/7 Temperature event int */
    DMCS3_FAILED_ACCESS_INT_IRQ = 206, /* DMC 3/7 Failed access int */
    DMCS3_MGR_INT_IRQ = 207, /* DMC 3/7 combined manager int */
    MCP_I2C0_IRQ = 208, /* MCP C2C I2C interrupt */
    MCP_I2C1_IRQ = 209, /* MCP PCC I2C interrupt */
    MCP_QSPI_IRQ = 210, /* MCP QSPI interrupt */
    RESERVED211_IRQ = 211, /* Reserved */
    RESERVED212_IRQ = 212, /* Reserved */
    RESERVED213_IRQ = 213, /* Reserved */
    RESERVED214_IRQ = 214, /* Reserved */
    RESERVED215_IRQ = 215, /* Reserved */
    RESERVED216_IRQ = 216, /* Reserved */
    RESERVED217_IRQ = 217, /* Reserved */
    RESERVED218_IRQ = 218, /* Reserved */
    RESERVED219_IRQ = 219, /* Reserved */
    PCIE_AER_IRQ = 220, /* PCIe address enable interrupt */
    PCIE_LOCAL_INT_REST_IRQ = 221, /* PCIe local error & status interrupt */
    PCIE_LOCAL_INT_RAS_IRQ = 222, /* PCIe local RAS interrupt */
    CCIX_AER_IRQ = 223, /* PCIe address enable interrupt */
    CCIX_LOCAL_INT_REST_IRQ = 224, /* PCIe local error & status interrupt */
    CCIX_LOCAL_INT_RAS_IRQ = 225, /* PCIe local RAS interrupt */
    RESERVED226_IRQ = 226, /* Reserved */
    RESERVED227_IRQ = 227, /* Reserved */
    RESERVED228_IRQ = 228, /* Reserved */
    RESERVED229_IRQ = 229, /* Reserved */
    RESERVED230_IRQ = 230, /* Reserved */
    RESERVED231_IRQ = 231, /* Reserved */
    RESERVED232_IRQ = 232, /* Reserved */
    RESERVED233_IRQ = 233, /* Reserved */
    RESERVED234_IRQ = 234, /* Reserved */
    RESERVED235_IRQ = 235, /* Reserved */
    RESERVED236_IRQ = 236, /* Reserved */
    RESERVED237_IRQ = 237, /* Reserved */
    RESERVED238_IRQ = 238, /* Reserved */
    RESERVED239_IRQ = 239, /* Reserved */
} IRQn_Type;

#include <core_cm7.h>

#endif /* FMW_CMSIS_MCP_H */
