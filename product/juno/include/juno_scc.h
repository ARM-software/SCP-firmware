/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno System Configuration Controller (SCC) register definitions.
 */

#ifndef JUNO_SCC_H
#define JUNO_SCC_H

#include "juno_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

enum pll_idx {
    PLL_IDX_BIG,
    PLL_IDX_LITTLE,
    PLL_IDX_GPU,
    PLL_IDX_SYS,
    PLL_IDX_HDLCD,
    PLL_IDX_COUNT,
};

struct pll_reg {
    FWK_RW  uint32_t  REG0;
    FWK_RW  uint32_t  REG1;
};

#define PLL_REG0_HARD_BYPASS       UINT32_C(0x00000001)
#define PLL_REG0_RST_TIMER_BYPASS  UINT32_C(0x00000002)
#define PLL_REG0_FORCE_LOCK        UINT32_C(0x00000010)
#define PLL_REG0_PWRDN             UINT32_C(0x00000100)
#define PLL_REG0_BYPASS            UINT32_C(0x00000200)
#define PLL_REG0_TEST              UINT32_C(0x00000400)
#define PLL_REG0_FASTEN            UINT32_C(0x00000800)
#define PLL_REG0_ENSAT             UINT32_C(0x00001000)
#define PLL_REG0_CLKF              UINT32_C(0x1FFF0000)
#define PLL_REG0_PLL_RESET         UINT32_C(0x80000000)
#define PLL_REG0_ENSAT_POS         12
#define PLL_REG0_NF_POS            16
#define PLL_REG0_NF                UINT32_C(0x1FFF0000)

#define PLL_REG1_NR_POS            0
#define PLL_REG1_OD_POS            8
#define PLL_REG1_NB_POS            12
#define PLL_REG1_NR                UINT32_C(0x0000003F)
#define PLL_REG1_OD                UINT32_C(0x00000F00)
#define PLL_REG1_NB                UINT32_C(0x00FFF000)
#define PLL_REG1_CLKR              UINT32_C(0x0000003F)
#define PLL_REG1_CLKOD             UINT32_C(0x00000F00)
#define PLL_REG1_BWADJ             UINT32_C(0x00FFF000)
#define PLL_REG1_LOCK_STATUS       UINT32_C(0x80000000)

#define PLL_NF_MAX  UINT16_C(4096)
#define PLL_NR_MAX  UINT8_C(64)
#define PLL_OD_MAX  UINT8_C(16)

enum pcsm_idx {
    PCSM_IDX_BIG_0,
    PCSM_IDX_BIG_1,
    PCSM_IDX_BIG_SCU,
    PCSM_IDX_LITTLE_0,
    PCSM_IDX_LITTLE_1,
    PCSM_IDX_LITTLE_2,
    PCSM_IDX_LITTLE_3,
    PCSM_IDX_LITTLE_SCU,
    PCSM_IDX_COUNT,
};

struct pcsm_reg {
            uint32_t  RESERVED1;
    FWK_RW  uint32_t  TRICKLE_DELAY;
    FWK_RW  uint32_t  HAMMER_DELAY;
    FWK_RW  uint32_t  RAM_DELAY;
            uint8_t   RESERVED2[0x100 - 0x10];
};

struct scc_reg {
    FWK_RW  uint32_t  FAXICLK;
    FWK_RW  uint32_t  SAXICLK;
    FWK_RW  uint32_t  HDLCDCLK;
    FWK_RW  uint32_t  TMIF2XCLK;
    FWK_RW  uint32_t  TSIF2XCLK;
    FWK_RW  uint32_t  USBHCLK;
    FWK_RW  uint32_t  PCIEACLK;
    FWK_RW  uint32_t  PCIETLCLK;
    FWK_RW  uint32_t  RESERVED1;
    FWK_RW  uint32_t  PXLCLK;
            uint8_t   RESERVED2[0x30 - 0x28];
    FWK_RW  uint32_t  SYSTEM_CLK_FORCE;
    FWK_RW  uint32_t  VSYS_MANUAL_RESET;
            uint8_t   RESERVED3[0x40 - 0x38];
    FWK_RW  uint32_t  SMC_MASK[4];
    FWK_RW  uint32_t  NIC400_TLX;
    FWK_RW  uint32_t  DMA_CONTROL0;
            uint8_t   RESERVED4[0x68 - 0x58];
    FWK_RW  uint32_t  HDLCD0_CONTROL;
    FWK_RW  uint32_t  HDLCD1_CONTROL;
            uint8_t   RESERVED5[0xF0 - 0x70];
    FWK_R   uint32_t  GPR0;
    FWK_R   uint32_t  GPR1;
    FWK_R   uint32_t  APP_ALT_BOOT;
    FWK_R   uint32_t  SCP_ALT_BOOT;
    struct pll_reg    PLL[PLL_IDX_COUNT];
    FWK_RW  uint32_t  DDR_PHY0_PLL;
    FWK_RW  uint32_t  DDR_PHY1_PLL;
            uint8_t   RESERVED6[0x200 - 0x130];
    struct pcsm_reg   PCSM[PCSM_IDX_COUNT];
            uint8_t   RESERVED7[0xA04 - 0xA00];
    FWK_RW  uint32_t  DDR_PHY0_RETNCTRL;
    FWK_RW  uint32_t  DDR_PHY1_RETNCTRL;
            uint8_t   RESERVED8[0x1000 - 0xA0C];
};

#define SCC ((struct scc_reg *) SCC_BASE)

#define SCC_TLX_MST_ENABLE                  UINT32_C(0x00000001)
#define SCC_TLX_SLV_PWRDNREQ                UINT32_C(0x00000002)
#define SCC_TLX_SLV_PWRDNACK                UINT32_C(0x00000004)
#define SCC_TLX_MST_PWRDNREQ                UINT32_C(0x00000008)
#define SCC_TLX_MST_PWRDNACK                UINT32_C(0x00000010)

#define SCC_HDLCD_CONTROL_PXLCLK_SEL        UINT32_C(0x00000001)
#define SCC_HDLCD_CONTROL_PXLCLK_SEL_PLL    UINT32_C(0x00000000)
#define SCC_HDLCD_CONTROL_PXLCLK_SEL_CLKIN  UINT32_C(0x00000001)

#define SCC_PXLCLK_CLKSEL_PLL               UINT32_C(0x00000002)

#define SCC_GPR0_SKIP_TLX_CLK_SETTING       UINT32_C(0x00400000)
#define SCC_GPR0_PCIE_AP_MANAGED            UINT32_C(0x00800000)
#define SCC_GPR0_PLATFORM_ID_PLAT           UINT32_C(0x0F000000)
#define SCC_GPR0_PLATFORM_ID_PLAT_POS       24
#define SCC_GPR0_DVFS_DISABLE               UINT32_C(0x10000000)
#define SCC_GPR0_HIGH_PXLCLK_ENABLE         UINT32_C(0x20000000)
#define SCC_GPR0_DDR_DISABLE                UINT32_C(0x40000000)
#define SCC_GPR0_CALIBRATION_ENABLE         UINT32_C(0x80000000)
#define SCC_GPR1_CRYPTO_DISABLE             UINT32_C(0x00000001)
#define SCC_GPR1_CFGTE                      UINT32_C(0x00000002)
#define SCC_GPR1_CFGEE                      UINT32_C(0x00000004)
#define SCC_GPR1_BOOT_MAP_ENABLE            UINT32_C(0x00000008)
#define SCC_GPR1_BOOT_MAP                   UINT32_C(0x00000FF0)
#define SCC_GPR1_BOOT_MAP_LITTLE            UINT32_C(0x000000F0)
#define SCC_GPR1_BOOT_MAP_LITTLE_POS        4
#define SCC_GPR1_BOOT_MAP_LITTLE_0          UINT32_C(0x00000010)
#define SCC_GPR1_BOOT_MAP_LITTLE_1          UINT32_C(0x00000020)
#define SCC_GPR1_BOOT_MAP_LITTLE_2          UINT32_C(0x00000040)
#define SCC_GPR1_BOOT_MAP_LITTLE_3          UINT32_C(0x00000080)
#define SCC_GPR1_BOOT_MAP_BIG               UINT32_C(0x00000300)
#define SCC_GPR1_BOOT_MAP_BIG_POS           8
#define SCC_GPR1_BOOT_MAP_BIG_0             UINT32_C(0x00000100)
#define SCC_GPR1_BOOT_MAP_BIG_1             UINT32_C(0x00000200)
#define SCC_GPR1_PRIMARY_CPU                UINT32_C(0x0000F000)
#define SCC_GPR1_PRIMARY_CPU_POS            12
#define SCC_GPR1_RESET                      UINT32_C(0x00030000)
#define SCC_GPR1_RESET_POWERON              UINT32_C(0x00000000)
#define SCC_GPR1_RESET_REBOOT               UINT32_C(0x00010000)
#define SCC_GPR1_RESET_WDOG                 UINT32_C(0x00020000)

#define SCC_APP_ALT_BOOT_ADDR               UINT32_C(0xFFFFFFFC)

#define SCC_DDR_PHY_RETNCTRL_ENABLE         UINT32_C(0x00000001)
#define SCC_DDR_PHY_RETNCTRL_DISABLE        UINT32_C(0x00000000)

#define SCC_DDR_PHY_PLL_BYPASS_EN           UINT32_C(0x00000001)
#define SCC_DDR_PHY_PLL_RANGE               UINT32_C(0x00000002)
#define SCC_DDR_PHY_PLL_REF_DIV             UINT32_C(0x00000F00)
#define SCC_DDR_PHY_PLL_FBK_DIV             UINT32_C(0x0000F000)

#define SCC_SYSTEM_CLK_FORCE_FAXICLK        UINT32_C(0x00000001)
#define SCC_SYSTEM_CLK_FORCE_SAXICLK        UINT32_C(0x00000002)
#define SCC_SYSTEM_CLK_FORCE_HDLCDCLK       UINT32_C(0x00000004)

#define SCC_PCLKDBG_CONTROL_CLKDIV          UINT32_C(0x0000000F)

#define SCC_SYSTEM_CLK_EN_FAXICLKEN         UINT32_C(0x00000001)
#define SCC_SYSTEM_CLK_EN_SAXICLKEN         UINT32_C(0x00000002)
#define SCC_SYSTEM_CLK_EN_HDLCDCLKEN        UINT32_C(0x00000004)
#define SCC_SYSTEM_CLK_EN_TMIF2XCLKEN       UINT32_C(0x00000008)
#define SCC_SYSTEM_CLK_EN_TSIF2XCLKEN       UINT32_C(0x00000010)
#define SCC_SYSTEM_CLK_EN_USBHCLKEN         UINT32_C(0x00000020)
#define SCC_SYSTEM_CLK_EN_PICEACCLKEN       UINT32_C(0x00000040)
#define SCC_SYSTEM_CLK_EN_PICETLCLKEN       UINT32_C(0x00000080)
#endif /* JUNO_SCC_H */
