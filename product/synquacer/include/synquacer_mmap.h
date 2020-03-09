/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYNQUACER_MMAP_H
#define SYNQUACER_MMAP_H

#include "synquacer_common.h"
#include "synquacer_pik.h"

#define PERIPH_BASE UINT32_C(0x44000000)
#define POWER_PERIPH_BASE UINT32_C(0x50000000)
#define EXTERNAL_DEV_BASE UINT32_C(0xA0000000)

#define TRUSTED_RAM_BASE (EXTERNAL_DEV_BASE + 0x04000000)
#define NIC_BASE (EXTERNAL_DEV_BASE + 0x2A100000)
#define REFCLK_CNTCONTROL_BASE (EXTERNAL_DEV_BASE + 0x2A430000)
#define REFCLK_CNTREAD_BASE (EXTERNAL_DEV_BASE + 0x2A800000)
#define DDR_WINDOW_BASE (EXTERNAL_DEV_BASE + 0x2B000000)
#define NONTRUSTED_RAM_BASE (EXTERNAL_DEV_BASE + 0x2E000000)

#define DDR_WINDOW_SIZE UINT32_C(0x100000)

#define REFCLK_CNTCTL_BASE (PERIPH_BASE + 0x0000)
#define REFCLK_CNTBASE0_BASE (PERIPH_BASE + 0x1000)
#ifdef CONFIG_SCB_USE_AP_PL011
#define SCP_UART_BASE UINT32_C(0xCA400000)
#else
#define SCP_UART_BASE (PERIPH_BASE + 0x2000)
#endif
#define WDOG_BASE (PERIPH_BASE + 0x6000)
#define MHU_AP_BASE (PERIPH_BASE + 0x1000000)

#define REFCLK_CNTCONTROL ((cntcontrol_reg_t *)REFCLK_CNTCONTROL_BASE)
#define REFCLK_CNTCTL ((cntctl_reg_t *)REFCLK_CNTCTL_BASE)
#define REFCLK_CNTBASE0 ((cntbase_reg_t *)REFCLK_CNTBASE0_BASE)
#define WDOG ((sp805_reg_t *)WDOG_BASE)

#define MHU_SCP_TO_AP_NS(cluster) \
    ((uintptr_t)(MHU_AP_BASE + 0x00010000 * cluster + 0x00000020))
#define MHU_AP_TO_SCP_NS(cluster) \
    ((uintptr_t)(MHU_AP_BASE + 0x00010000 * cluster + 0x00000120))
#define MHU_SCP_TO_AP_S(cluster) \
    ((uintptr_t)(MHU_AP_BASE + 0x00010000 * cluster + 0x00000200))
#define MHU_AP_TO_SCP_S(cluster) \
    ((uintptr_t)(MHU_AP_BASE + 0x00010000 * cluster + 0x00000300))

/*
 * MHU secure/non-secure SRAM
 */
#define MHU_CLUSTER_PAYLOAD_INTERVAL (64 * 1024)
#define MHU_PAYLOAD_SIZE (256)
#define MHU_PAYLOAD_NS_BASE (PERIPH_BASE + 0x01200000)
#define MHU_PAYLOAD_S_BASE (PERIPH_BASE + 0x01400000)

#define MHU_PAYLOAD_S_CLUSTER_BASE(cluster) \
    (MHU_PAYLOAD_S_BASE + MHU_CLUSTER_PAYLOAD_INTERVAL * cluster)
/*
 * Context area used by trusted firmware. Should be zeroed by SCP.
 */
#define AP_CONTEXT_BASE (TRUSTED_RAM_BASE + 0x0003F000)
#define AP_CONTEXT_SIZE 8

#define CCN512_BASE (EXTERNAL_DEV_BASE + 0x32000000)

#define CONFIG_SOC_CORE_CLOCK UINT32_C(125000000)
#define CONFIG_SCB_TICK_US 500

/* SynQuacer peripheral address */
#define HSSPI_REG_BASE UINT32_C(0x48800000)
#define HSSPI_MEM_BASE UINT32_C(0xA8000000)

#define CONFIG_SOC_REG_ADDR_PBC_TOP UINT32_C(0x48900000)
#define CONFIG_SOC_REG_ADDR_PMU_TOP UINT32_C(0x48130000)

#define CONFIG_SOC_HSSPI_EEPROM_BASE_ADDR UINT32_C(0x48810000)
#define CONFIG_SOC_BOOT_HSSPI_REG_ADDR UINT32_C(0x48100008)

#define CONFIG_SCP_CONFIG_TABLE_ADDR UINT32_C(0xA8080000)

/* EEPROM CONFIG */
#define CONFIG_SCB_EEPROM_SIZE UINT32_C(65536)
#define CONFIG_SCP_CONFIG_TABLE_MAX_SIZE UINT32_C(65536)

#define CONFIG_SOC_REG_ADDR_SYSOC_TOP UINT32_C(0x48300000)
#define CONFIG_SOC_REG_ADDR_BOOT_CTL_TOP UINT32_C(0x48100000)
#define CONFIG_SOC_REG_ADDR_CFG_CTL_TOP UINT32_C(0x48110000)
#define CONFIG_SOC_REG_ADDR_XCPB_TOP UINT32_C(0x48160000)

#define AP_SCP_NIC UINT32_C(0x70100000)
#define DDRPHYREG0_NIC UINT32_C(0x7f000000)
#define DDRPHYREG1_NIC UINT32_C(0x7f400000)
#define DMABREG_NIC UINT32_C(0x7f800000)
#define FE_NIC UINT32_C(0x75000000)
#define SCBM_MV_NIC UINT32_C(0x72600000)

#define CONFIG_SOC_NIC_ADDR_INFO                                           \
    {                                                                      \
        NIC_BASE, AP_SCP_NIC, DDRPHYREG0_NIC, DDRPHYREG1_NIC, DMABREG_NIC, \
            FE_NIC,                                                        \
    }

#define CONFIG_SCB_NIC_INFO                               \
    {                                                     \
        { /* NIC_BASE */                                  \
          NIC_SETUP_SKIP, NIC_SETUP_SKIP, NIC_SETUP_SKIP, \
          NIC_SETUP_SKIP, NIC_SETUP_SKIP, 1,              \
          END_OF_NIC_LIST                                 \
        },                                                \
            { /* AP_SCP_NIC */                            \
              1,                                          \
              END_OF_NIC_LIST                             \
            },                                            \
            { /* DDRPHYREG0_NIC */                        \
              3,                                          \
              END_OF_NIC_LIST                             \
            },                                            \
            { /* DDRPHYREG1_NIC */                        \
              3,                                          \
              END_OF_NIC_LIST                             \
            },                                            \
            { /* DMABREG_NIC */                           \
              NIC_SETUP_SKIP,                             \
              1,                                          \
              END_OF_NIC_LIST                             \
            },                                            \
            { /* FE_NIC */                                \
              NIC_SETUP_SKIP,                             \
              NIC_SETUP_SKIP,                             \
              1,                                          \
              1,                                          \
              END_OF_NIC_LIST                             \
            },                                            \
    }

/* CRG config */
#define CONFIG_SOC_CRG11_NUM 2
#define CONFIG_SCB_CRG11_ID_PERI 0
#define CONFIG_SCB_CRG11_ID_EMMC 1

#define CONFIG_SOC_REG_ADDR_CRG_PERI_TOP UINT32_C(0x48310000)
#define CONFIG_SOC_REG_ADDR_CRG_EMMC_TOP UINT32_C(0x48320000)

#define F_UART3_BASE_ADDR UINT32_C(0x71040000)
#define CONFIG_SCB_UART_BAUD_RATE UART_SYSPARAM_BAUD_RATE_115200

#define CONFIG_SOC_REG_ADDR_SYSOC_TOP UINT32_C(0x48300000)
#define CONFIG_SOC_REG_ADDR_SYSOC_BUS_TOP CONFIG_SOC_REG_ADDR_SYSOC_TOP
#define CONFIG_SOC_REG_ADDR_SYSOC_SCP_TOP (CONFIG_SOC_REG_ADDR_SYSOC_TOP + 0x10)
#define CONFIG_SOC_REG_ADDR_SYSOC_DMA_TOP (CONFIG_SOC_REG_ADDR_SYSOC_TOP + 0x40)
#define CONFIG_SOC_REG_ADDR_SYSOC_PCIE_TOP \
                    (CONFIG_SOC_REG_ADDR_SYSOC_TOP + 0x50)
#define CONFIG_SOC_REG_ADDR_SYSOC_DDR_TOP (CONFIG_SOC_REG_ADDR_SYSOC_TOP + 0x60)

#define CONFIG_SOC_INIT_SYSOC_ADDR_INFOS                                      \
    {                                                                         \
        CONFIG_SOC_REG_ADDR_SYSOC_BUS_TOP, CONFIG_SOC_REG_ADDR_SYSOC_SCP_TOP, \
    }

#define CONFIG_SOC_INIT_SYSOC_RESET_INFOS                    \
    {                                                        \
        /* BUS  */ UINT32_C(0xf), /* SCP  */ UINT32_C(0x1f), \
    }

#define CONFIG_SCB_ARM_TF_BASE_ADDR UINT32_C(0xA8180000)
#define CONFIG_SCB_UEFI_BASE_ADDR UINT32_C(0xA8200000)

#define CONFIG_SCB_ARM_TB_BL1_BASE_ADDR UINT32_C(0xA4000000)
#define CONFIG_SCB_ARM_TB_BL2_BASE_ADDR UINT32_C(0xA4013000)
#define CONFIG_SCB_ARM_TB_BL3_BASE_ADDR UINT32_C(0xA401F000)
#define CONFIG_SCB_ARM_TB_BL32_BASE_ADDR UINT32_C(0xFC000000)

#define CONFIG_SCB_UART_RECV_BUF_BASE_ADDR UINT32_C(0xA4050000)

/* 125MHz / 4 = 31.25MHz */
#define CONFIG_SOC_HSSPI_CLK_CONFIG                         \
    {                                                       \
        .clk_sel = HSSPI_EN_MCTRL_CDSS_iHCLK, .clk_div = 4, \
        .syncon = HSSPI_EN_MCTRL_SYNCON_SYNC                \
    }

#define CONFIG_SOC_LPCM_SCB_TOP_ADDR UINT32_C(0x48318000)
#define CONFIG_SOC_LPCM_PCIE_TOP_ADDR UINT32_C(0x48319000)
#define CONFIG_SOC_LPCM_DMAB_TOP_ADDR UINT32_C(0x4831A000)
#define CONFIG_SOC_LPCM_EMMC_TOP_ADDR UINT32_C(0x48328000)

#define CONFIG_SOC_LPCM_SCB_RESET_FLAG UINT32_C(0x1)
#define CONFIG_SOC_LPCM_PCIE_RESET_FLAG UINT32_C(0x73)
#define CONFIG_SOC_LPCM_DMAB_RESET_FLAG UINT32_C(0x3)

#define CONFIG_SOC_REG_ADDR_SYS_OVER_REG_TOP UINT32_C(0x7FFF0000)
#define CONFIG_SOC_SYS_OVER_OFFSET_SEC_OVERRIDE UINT32_C(0x100)

#define CONFIG_SCB_DDR_FREQ DDR_FREQ_2133

#define CONFIG_SCB_USE_4BYTE_MODE

#define CONFIG_SCB_SMMU_PAGE_TABLE_BASE_ADDR UINT32_C(0xA4040000)

#define CONFIG_SOC_NORTH_SMMU_REG_BASE UINT32_C(0x78280000)
#define CONFIG_SOC_SOUTH_SMMU_REG_BASE UINT32_C(0x782C0000)
#define CONFIG_SOC_DMAB_SMMU_REG_BASE UINT32_C(0x7fb00000)

#define CONFIG_SOC_SD_CTL_REG_BASE UINT32_C(0x722E0000)

#define CONFIG_SOC_DMA330_REG_BASE UINT32_C(0x7fa00000)
#define CONFIG_SOC_DMAB_WRAPPER_REG UINT32_C(0x7fb80000)

#define CONFIG_SCB_ALL_OFF_LOG_GROUP UINT32_C(0x00)

/** Number of PCIe instances */
#define CONFIG_SOC_PCIE_NUM 2

/* cpl timeout range value 6: 65msec ~ 210msec */
#define CONFIG_SCB_PCIE_CPL_TIMEOUT_RANVE_VALUE 0x6U

/* Overriding BE[7:1] in PCIe TLP */
#define CONFIG_SCB_TWEAK_PCIE_TLP_BE_OVERRIDE 0x7U

#define CONFIG_SOC_PCIE_BAR0_MASK_DEFAULT 0x0000000FFFFFFFFFLLU

#define CONFIG_SOC_PCIE_PME_SUPPORT (0x1BU)
#define CONFIG_SOC_PCIE_D2_SUPPORT (1U)
#define CONFIG_SOC_PCIE_D1_SUPPORT (1U)
#define CONFIG_SOC_PCIE_AUX_CURRENT (7U)
#define CONFIG_SOC_PCIE_DSI (0)
#define CONFIG_SOC_PCIE_ASPM_SUPPORT (0x3U)

#define CONFIG_SOC_PRMUX_BASE_ADDR UINT32_C(0x74600000)
#define CONFIG_SOC_AP_GPIO_BASE (0x71000000)

/**
 * PRMUX settings
 *   1:GPIO_NS
 */
#define CONFIG_SCB_PRMUX_PINGRP \
    {                           \
        1, 1, 1, 1, 1, 1        \
    }

/**
 * GPIO direction settings
 *   1: out
 *   0: in
 */
#ifndef CONFIG_SCB_GPIO_DIRECTION
#define CONFIG_SCB_GPIO_DIRECTION \
    {                             \
        0, 0, 0, 0                \
    }
#endif /* CONFIG_SCB_GPIO_DIRECTION */

/**
 * GPIO function settings
 *   0: GPIO
 */
#ifndef CONFIG_SCB_GPIO_FUNCTION
#define CONFIG_SCB_GPIO_FUNCTION \
    {                            \
        0, 0, 0, 0               \
    }
#endif /* CONFIG_SCB_GPIO_FUNCTION */

#define CONFIG_SOC_I2C_ENABLE_BITMAP (1 << 0)
#define CONFIG_SOC_I2C_BASE_ADDRS \
    {                             \
        0x4A000000                \
    }
#define CONFIG_SOC_I2C_TYPES \
    {                        \
        I2C_TYPE_F_I2C       \
    }
#define CONFIG_SOC_I2C_CHANNELS \
    {                           \
        I2C_EN_CH0              \
    }

#define CONFIG_SCB_I2C_PARAMS                      \
    {                                              \
        { .I2C_PARAM_F_I2C = { .FSR_FS = 0x02,     \
                               .CSR_CS = 0x03,     \
                               .CCR_CS = 0x07,     \
                               .CCR_FM = 0x01 } }, \
    }

/* SRAM for one way CM3->AP mailbox for BMC bootflag */
#define CONFIG_SCB_BMC_BOOTFLAG_ADDR UINT32_C(0xCE00FFF8)

#ifdef CONFIG_SCB_DIST_FIRM
#define CONFIG_SCB_MANUAL_THERMAL_READ
#endif /* CONFIG_SCB_DIST_FIRM */

#define CONFIG_SOC_PRMUX_MAX_IDX 7

#endif /* SYNQUACER_MMAP_H */
