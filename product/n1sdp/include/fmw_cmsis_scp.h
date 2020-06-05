/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FMW_CMSIS_SCP_H
#define FMW_CMSIS_SCP_H


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
    SCP_EXT_IRQ = 16, /* SCP External IRQ */
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
    TIMER_SYNC_IRQ = 34, /* Timer Synchronization Module */
    RESERVED35_IRQ = 35, /* Reserved */
    RESERVED36_IRQ = 36, /* Reserved */
    CTI_TRIGGER0_IRQ = 37, /* SCP CTI0 Trigger */
    CTI_TRIGGER1_IRQ = 38, /* SCP CTI1 Trigger */
    GIC_ERROR_ECC_IRQ = 39, /* GIC Error (ECC Fatal) */
    GIC_ERROR_AXIM_IRQ = 40, /* GIC Error (AXIM) */
    RESERVED41_IRQ = 41, /* Reserved */
    AON_UART_IRQ = 42, /* Always on UART */
    RESERVED43_IRQ = 43, /* Reserved */
    GEN_WD_WS0_IRQ = 44, /* Generic Watchdog timer WS0 */
    GEN_WD_WS1_IRQ = 45, /* Generic Watchdog timer WS1 */
    TRUSTED_WD_WS0_IRQ = 46, /* Trusted Watchdog timer WS0 */
    TRUSTED_WD_WS1_IRQ = 47, /* Trusted Watchdog timer WS1 */
    APPS_UART_IRQ = 48, /* Application UART */
    RESERVED49_IRQ = 49, /* Reserved */
    PPU_CORES0_IRQ =
        50, /* Consolidated PPU Interrupt for cores 1-32, 129-160 */
    PPU_CORES1_IRQ =
        51, /* Consolidated PPU Interrupt for cores 33-64, 161-192 */
    PPU_CORES2_IRQ =
        52, /* Consolidated PPU Interrupt for cores 65-96, 193-224 */
    PPU_CORES3_IRQ =
        53, /* Consolidated PPU Interrupt for cores 97-128, 225-256 */
    PPU_CLUSTERS_IRQ = 54, /* Consolidated clusters PPU */
    PLL_CORES0_LOCK_IRQ =
        55, /* Consolidated PLL lock for PLLs 1-32, 65-96, 129-160, 193-224 */
    PLL_CORES1_LOCK_IRQ =
        56, /* Consolidated PLL lock for PLLs 33-64, 97-128, 161-192, 225-256 */
    PLL_CORES0_UNLOCK_IRQ =
        57, /* Consolidated PLL unlock for PLLs 1-32, 65-96, 129-160, 193-224 */
    PLL_CORES1_UNLOCK_IRQ =
        58, /* Consolidated PLL lock for PLLs 33-64, 97-128, 161-192, 225-256 */
    FAULT_CORES_IRQ = 59, /* Consolidated fault IRQ for cores */
    RESERVED60_IRQ = 60, /* Reserved */
    RESERVED61_IRQ = 61, /* Reserved */
    RESERVED62_IRQ = 62, /* Reserved */
    RESERVED63_IRQ = 63, /* Reserved */
    ECC_CORES_ERROR_IRQ = 64, /* Consolidated ECC ERROR for cores */
    RESERVED65_IRQ = 65, /* Reserved */
    RESERVED66_IRQ = 66, /* Reserved */
    RESERVED67_IRQ = 67, /* Reserved */
    RESERVED68_IRQ = 68, /* Reserved */
    PLL_CLUSTERS_LOCK_IRQ = 69, /* Consolidated clusters PLL Lock */
    PLL_CLUSTERS_UNLOCK_IRQ = 70, /* Consolidated clusters PLL Unlock*/
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
    MHU_AP_NONSEC_IRQ = 82, /* MHU non-secure irq between SCP and  AP */
    MHU_AP_SEC_IRQ = 83, /* MHU secure irq bewteen SCP and AP */
    MHU_MCP_NONSEC_IRQ = 84, /* MHU non-secure irq between SCP and  MCP */
    MHU_MCP_SEC_IRQ = 85, /* MHU secure irq bewteen SCP and MCP */
    RESERVED86_IRQ = 86, /* Reserved */
    RESERVED87_IRQ = 87, /* Reserved */
    RESERVED88_IRQ = 88, /* Reserved */
    RESERVED89_IRQ = 89, /* Reserved */
    TIMER_CLUSTERS_IRQ = 90, /* Consolidated clusters timer interrupt */
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
    INT_PMU_DTC_GT_0_IRQ = 101, /* PMU count overflow irq if #DTC > 0 */
    INT_PMU_DTC_GT_1_IRQ = 102, /* PMU count overflow irq if #DTC > 1 */
    INT_PMU_DTC_GT_2_IRQ = 103, /* PMU count overflow irq if #DTC > 2 */
    INT_PMU_DTC_GT_3_IRQ = 104, /* PMU count overflow irq if #DTC > 3 */
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
    DEBUG_PIK_IRQ = 130, /* DEBUG PIK */
    PPU_LOGIC_IRQ = 131, /* PPU LOGIC */
    RESERVED132_IRQ = 132, /* Reserved */
    RESERVED133_IRQ = 133, /* Reserved */
    RESERVED134_IRQ = 134, /* Reserved */
    PPU_SRAM_IRQ = 135, /* PPU SRAM */
    RESERVED136_IRQ = 136, /* Reserved */
    RESERVED137_IRQ = 137, /* Reserved */
    RESERVED138_IRQ = 138, /* Reserved */
    MCP_WD_WS1_IRQ = 139, /* MCP watchdog reset */
    PLL_SYS_LOCK_IRQ = 140, /* System PLL Lock */
    PLL_SYS_UNLOCK_IRQ = 141, /* System PLL Unlock */
    PLL_INT_LOCK_IRQ = 142, /* Interconnect PLL Lock */
    PLL_INT_UNLOCK_IRQ = 143, /* Interconnect PLL Unlock */
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
    PLL_DMC_LOCK_IRQ = 174, /* DMC PLL LOCK */
    PLL_DMC_UNLOCK_IRQ = 175, /* DMC PLL LOCK */
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
        197, /* DMC 2/6 Combined Miscellaneous  access int */
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
    SCP_I2C0_IRQ = 208, /* SCP C2C I2C interrupt */
    SCP_I2C1_IRQ = 209, /* SCP PMIC I2C interrupt */
    SCP_I2C2_IRQ = 210, /* SCP SPD-PCC I2C interrupt */
    SCP_QSPI_IRQ = 211, /* SCP QSPI interrupt */
    SCP_PVT_IRQ = 212, /* SCP PVT Controller interrupt */
    RESERVED213_IRQ = 213, /* Reserved */
    RESERVED214_IRQ = 214, /* Reserved */
    RESERVED215_IRQ = 215, /* Reserved */
    RESERVED216_IRQ = 216, /* Reserved */
    RESERVED217_IRQ = 217, /* Reserved */
    RESERVED218_IRQ = 218, /* Reserved */
    CCIX_BUS_DEV_CHG_IRQ = 219, /* CCIX Bus Device Change interrupt */
    CCIX_INTA_OUT_IRQ = 220, /* CCIX INTA Out interrupt */
    CCIX_INTB_OUT_IRQ = 221, /* CCIX INTB Out interrupt */
    CCIX_INTC_OUT_IRQ = 222, /* CCIX INTC Out interrupt */
    CCIX_INTD_OUT_IRQ = 223, /* CCIX INTD Out interrupt */
    CCIX_PHY_INT_OUT_IRQ = 224, /* CCIX phy interrupt out interrupt */
    CCIX_AER_IRQ = 225, /* CCIX address enable interrupt */
    CCIX_LINK_DOWN_RESET_IRQ = 226, /* CCIX link down reset interrupt */
    CCIX_LOCAL_INT_REST_IRQ = 227, /* CCIX local error & status interrupt */
    CCIX_PERF_DATA_THRESHOLD_IRQ =
        228, /* CCIX performance data threshold interrupt */
    CCIX_NEG_SPD_CHANGE_IRQ = 229, /* CCIX nogotiated speed change interrupt */
    CCIX_LINK_TRAIN_DONE_IRQ = 230, /* CCIX link training done interrupt */
    CCIX_PLL_STATUS_RISE_IRQ = 231, /* CCIX PLL status rise interrupt */
    CCIX_MSG_FIFO_IRQ = 232, /* CCIX message FIFO interrupt */
    CCIX_LOCAL_INT_RAS_IRQ = 233, /* CCIX local RAS interrupt */
    CCIX_HOT_RESET_IRQ = 234, /* CCIX hot reset interrupt */
    CCIX_FLR_RST_IRQ = 235, /* CCIX function level reset interrupt */
    CCIX_PWR_STATE_CHANGE_IRQ = 236, /* CCIX power state change interrupt */
    PCIE_AER_IRQ = 237, /* PCIe address enable interrupt */
    PCIE_LOCAL_INT_REST_IRQ = 238, /* PCIe local error & status interrupt */
    PCIE_LOCAL_INT_RAS_IRQ = 239 /* PCIe local RAS interrupt */
} IRQn_Type;

#include <core_cm7.h>

#endif /* FMW_CMSIS_SCP_H */
