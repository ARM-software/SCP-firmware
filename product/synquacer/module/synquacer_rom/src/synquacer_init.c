/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"
#include "pik_scp.h"
#include "pik_system.h"
#include "synquacer_mmap.h"

#include <ppu_v0.h>

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

void fw_clk_init(void);
void fw_sysoc_init(void);
void fw_lpcm_init(void);

#define REG_ADDR_RSTSTA (0)
#define REG_ADDR_RSTSET (0x4U)
#define REG_ADDR_RSTCLR (0x8U)

#define LPCM_RESET_CTRL_ADDR UINT32_C(0x400)
#define LPCM_RESET_CTRL_OFFSET UINT32_C(0x8)

#define LPCM_RESET_STA_ADDR 0x404U
#define LPCM_RESET_CLR 0x1U

#define STATUS_WAIT true
#define STATUS_NO_WAIT false

#define REG_ADDR_RSTSTA (0)
#define REG_ADDR_RSTSET (0x4U)
#define REG_ADDR_RSTCLR (0x8U)

#define LPCM_RESET_CTRL_ADDR UINT32_C(0x400)
#define LPCM_RESET_CTRL_OFFSET UINT32_C(0x8)

#define LPCM_RESET_STA_ADDR 0x404U
#define LPCM_RESET_CLR 0x1U

static void fw_clock_pll_init(void)
{
    /* 1 bit for each cluster */
    const uint32_t pll_mask = 0x00000FFF;

    while ((PIK_SCP->PLL_STATUS[1] & pll_mask) != pll_mask ||
           (PIK_SCP->PLL_STATUS[9] & pll_mask) != pll_mask)
        continue;
}

static void pll_init(void)
{
    uint32_t pll_mask = PLL_STATUS_0_REFCLK | PLL_STATUS_0_SYSPLLLOCK;
    /* Wait for PLL to lock */
    while ((PIK_SCP->PLL_STATUS[0] & pll_mask) != pll_mask)
        continue;
}

void fw_sysoc_init(void)
{
    uint32_t i, value;

    const uint32_t init_sysoc_addr_infos[] = {
        CONFIG_SOC_REG_ADDR_SYSOC_DMA_TOP,
        CONFIG_SOC_REG_ADDR_SYSOC_PCIE_TOP,
        CONFIG_SOC_REG_ADDR_SYSOC_DDR_TOP
    };

    const uint32_t init_sysoc_reset_infos[] = { UINT32_C(0x3), /* DMA  */
                                                UINT32_C(0x3), /* PCIE */
                                                UINT32_C(0xff),
                                                /* DDR  */ };

    for (i = 0; i < FWK_ARRAY_SIZE(init_sysoc_addr_infos); i++) {
        value = readl(init_sysoc_addr_infos[i] + REG_ADDR_RSTSTA);
        if (value == 0)
            continue; /* reset all not active */

        /* reset set only active bit */
        writel(
            init_sysoc_addr_infos[i] + REG_ADDR_RSTCLR,
            (init_sysoc_reset_infos[i] & value));

        while ((readl(init_sysoc_addr_infos[i] + REG_ADDR_RSTSTA) &
                init_sysoc_reset_infos[i]) != 0)
            ;
    }
}

void fw_lpcm_init(void)
{
    uint32_t i, j;

    const uint32_t lpcm_reset_addr_infos[] = {
        CONFIG_SOC_LPCM_PCIE_TOP_ADDR + LPCM_RESET_CTRL_ADDR,
        CONFIG_SOC_LPCM_DMAB_TOP_ADDR + LPCM_RESET_CTRL_ADDR
    };

    const uint32_t lpcm_reset_flag[] = { CONFIG_SOC_LPCM_PCIE_RESET_FLAG,
                                         CONFIG_SOC_LPCM_DMAB_RESET_FLAG };

    for (i = 0; i < FWK_ARRAY_SIZE(lpcm_reset_addr_infos); i++) {
        for (j = 0; j < (sizeof(uint32_t) * 8); j++) {
            if ((lpcm_reset_flag[i] & (0x1U << j)) == 0)
                continue; /* no reset signal */

            /* reset clr set lpcm reset_ctrl*/
            writel(
                lpcm_reset_addr_infos[i] + (LPCM_RESET_CTRL_OFFSET * j),
                LPCM_RESET_CLR);
        }
    }
}

static void set_clkforce_set(void)
{
    /* 0x0A04 : CLKFORCE_SET HW BUG WORKAROUND bit8 bit6 bit 2 */
    /* 0 = clock gated. 1 = Clock is forced to always operate. */
    /* bit8 : SYSPLLCLKDBGFORCE*/
    /* bit7 : DMCCLKFORCE */
    /* bit6 : SYSPERCLKFORCE */
    /* bit5 : PCLKSCPFORCE */
    /* bit2 : CCNCLKFORCE */
    /* bit0 : PPUCLKFORCE */
    PIK_SYSTEM->CLKFORCE_SET = 0x00000144U;
}

void fw_clear_clkforce(uint32_t value)
{
    /*
     * Writing 1 to a bit enables dynamic hardware clock-gating.
     * Writing 0 to a bit is ignored.
     * bit8 : SYSPLLCLKDBGFORCE
     * bit7 : DMCCLKFORCE
     * bit6 : SYSPERCLKFORCE
     * bit5 : PCLKSCPFORCE
     * bit2 : CCNCLKFORCE
     * bit0 : PPUCLKFORCE
     */
    PIK_SYSTEM->CLKFORCE_CLR = value;
}

uint32_t fw_get_clkforce_status(void)
{
    return PIK_SYSTEM->CLKFORCE_STATUS;
}

static struct ppu_v0_reg *const ppu_reg_p[] = {
    (struct ppu_v0_reg *)0x50041000, /* SYS-LOGIC-PPU0 */
    (struct ppu_v0_reg *)0x50042000, /* SYS-L3RAM0-PPU0 */
    (struct ppu_v0_reg *)0x50043000, /* SYS-L3RAM1-PPU0 */
    (struct ppu_v0_reg *)0x50044000, /* SYS-SFRAM-PPU0 */
    (struct ppu_v0_reg *)0x50045000, /* SYS-SRAM-PPU0 */
    (struct ppu_v0_reg *)0x50021000, /* SYS-DEBUG-PPU0 */
};

static void peri_ppu_on(void)
{
    uint32_t i;

    for (i = 0; i < FWK_ARRAY_SIZE(ppu_reg_p); i++)
        ppu_reg_p[i]->POWER_POLICY = PPU_V0_MODE_ON;
}

static void peri_ppu_wait(void)
{
    uint32_t i;

    for (i = 0; i < FWK_ARRAY_SIZE(ppu_reg_p); i++) {
        while ((ppu_reg_p[i]->POWER_STATUS & PPU_V0_PSR_POWSTAT) !=
               PPU_V0_MODE_ON)
            ;
    }
}

void fw_clk_init(void)
{
    pll_init();
    set_clkforce_set();

    peri_ppu_on();

    fw_clock_pll_init();

    peri_ppu_wait();
}

static void fw_sram_sysoc_init(void)
{
    uint32_t i, value;

    const uint32_t init_sysoc_addr_infos[] = CONFIG_SOC_INIT_SYSOC_ADDR_INFOS;
    const uint32_t init_sysoc_reset_infos[] = CONFIG_SOC_INIT_SYSOC_RESET_INFOS;

    for (i = 0; i < FWK_ARRAY_SIZE(init_sysoc_addr_infos); i++) {
        value = readl(init_sysoc_addr_infos[i] + REG_ADDR_RSTSTA);
        if (value == 0)
            continue; /* reset all not active */

        /* reset set only active bit */
        writel(
            (init_sysoc_addr_infos[i] + REG_ADDR_RSTCLR),
            (init_sysoc_reset_infos[i] & value));

        while ((readl(init_sysoc_addr_infos[i] + REG_ADDR_RSTSTA) &
                init_sysoc_reset_infos[i]) != 0)
            ;
    }
}

static void fw_sram_lpcm_init(void)
{
    /* reset clr set lpcm reset_ctrl*/
    writel(CONFIG_SOC_LPCM_SCB_TOP_ADDR + LPCM_RESET_CTRL_ADDR, LPCM_RESET_CLR);
}

void synquacer_system_init(void)
{
    fw_sram_sysoc_init();

    fw_sram_lpcm_init();

    fw_clk_init();

    fw_sysoc_init();

    fw_lpcm_init();
}
