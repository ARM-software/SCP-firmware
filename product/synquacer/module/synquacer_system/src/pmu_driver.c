/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "low_level_access.h"
#include "synquacer_common.h"
#include "synquacer_mmap.h"

#include <internal/pmu.h>

#include <stdint.h>

#define CONFIG_SOC_REG_ADDR_PMU_CTL (CONFIG_SOC_REG_ADDR_PMU_TOP + 0)
#define CONFIG_SOC_REG_ADDR_PMU_INT_EN (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x8U)
#define CONFIG_SOC_REG_ADDR_PMU_INT_ST (CONFIG_SOC_REG_ADDR_PMU_TOP + 0xCU)
#define CONFIG_SOC_REG_ADDR_PMU_INT_CLR (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x10U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_STR (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x18U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_EN0 (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x20U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_EN1 (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x28U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_DN_EN (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x40U)

#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC0 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x51U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC1 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x54U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC2 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x58U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC3 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x5CU)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC4 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x60U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC5 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x64U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC6 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x68U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC7 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x6CU)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_CYC_SEL \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x70U)

#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY0 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x80U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY1 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x84U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY2 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x88U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY3 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x8CU)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY4 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x90U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY5 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x94U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY6 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x98U)
#define CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY7 \
    (CONFIG_SOC_REG_ADDR_PMU_TOP + 0x9CU)

#define PD_MAX_NO 31U

/** @name bit fields for PMU_CTL */
//@{
/** ClocK Gating Enabel */
#define CONFIG_SOC_PMU_CTL_REG_CKGE (1UL << 31)
/** SET ENable */
#define CONFIG_SOC_PMU_CTL_REG_SETEN (0xAAUL << 8)
/** All Down SET */
#define CONFIG_SOC_PMU_CTL_REG_ADSET (1UL << 2)
/** Power Down SET */
#define CONFIG_SOC_PMU_CTL_REG_PDSET (1UL << 1)
/** Power Up SET */
#define CONFIG_SOC_PMU_CTL_REG_PUSET (1UL << 0)
//@}

static const uint32_t pmu_power_on_cycle_reg_addr[] = {
    CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC0, CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC1,
    CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC2, CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC3,
    CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC4, CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC5,
    CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC6, CONFIG_SOC_REG_ADDR_PMU_PWR_ON_CYC7
};

static const uint32_t pmu_pwr_on_dly_reg_addr[] = {
    CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY0, CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY1,
    CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY2, CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY3,
    CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY4, CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY5,
    CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY6, CONFIG_SOC_REG_ADDR_PMU_PWR_ON_DLY7
};

void pmu_on(uint32_t pd_on_flag)
{
    writel(CONFIG_SOC_REG_ADDR_PMU_PWR_ON_EN0, pd_on_flag);

    writel(
        CONFIG_SOC_REG_ADDR_PMU_CTL,
        (CONFIG_SOC_PMU_CTL_REG_SETEN | CONFIG_SOC_PMU_CTL_REG_PUSET));
}

void pmu_off(uint32_t pd_off_flag)
{
    writel(CONFIG_SOC_REG_ADDR_PMU_PWR_DN_EN, pd_off_flag);

    writel(
        CONFIG_SOC_REG_ADDR_PMU_CTL,
        (CONFIG_SOC_PMU_CTL_REG_SETEN | CONFIG_SOC_PMU_CTL_REG_PDSET));
}

uint32_t pmu_read_pd_power_status(void)
{
    return readl(CONFIG_SOC_REG_ADDR_PMU_PWR_STR);
}

void pmu_write_power_on_cycle(uint8_t pd_no, uint8_t value)
{
    uint32_t tmp;
    uint32_t idx, bit_field_offset, intsts;

    if (pd_no > PD_MAX_NO)
        return;

    /* calc on cycle idx*/
    idx = (pd_no >> 2);

    /* calc field offset*/
    bit_field_offset = (pd_no & 0x3U);

    DI(intsts);

    tmp = readl(pmu_power_on_cycle_reg_addr[idx]);

    /* clear on cycle*/
    tmp &= ~(0xffUL << (8 * bit_field_offset));

    /* set new on cycle*/
    tmp |= (value << (8 * bit_field_offset));

    writel(pmu_power_on_cycle_reg_addr[idx], tmp);

    EI(intsts);
}

uint8_t pmu_read_power_on_cycle(uint8_t pd_no)
{
    uint32_t tmp;
    uint8_t value = 0;
    uint32_t idx, bit_field_offset;

    if (pd_no > PD_MAX_NO)
        return value;

    /* calc on cycle idx*/
    idx = (pd_no >> 2);

    /* calc field offset*/
    bit_field_offset = (pd_no & 0x3U);

    tmp = readl(pmu_power_on_cycle_reg_addr[idx]);

    value = (tmp >> (8 * bit_field_offset)) & 0xffU;

    return value;
}

void pmu_enable_int(uint32_t enable_bit)
{
    uint32_t value;
    uint32_t intsts;

    DI(intsts);

    /* read  and set int enable */
    value = (readl(CONFIG_SOC_REG_ADDR_PMU_INT_EN) | enable_bit);

    /* write int enable */
    writel(CONFIG_SOC_REG_ADDR_PMU_INT_EN, value);

    EI(intsts);
}

void pmu_disable_int(uint32_t disable_bit)
{
    uint32_t value;
    uint32_t intsts;

    DI(intsts);

    /* read int enable */
    value = readl(CONFIG_SOC_REG_ADDR_PMU_INT_EN);

    /* read clear int enable */
    value &= (~(disable_bit));

    writel(CONFIG_SOC_REG_ADDR_PMU_INT_EN, value);

    EI(intsts);
}

uint8_t pmu_read_int_satus(void)
{
    return (readl(CONFIG_SOC_REG_ADDR_PMU_INT_ST) & 0xffU);
}

void pmu_clr_int_satus(uint32_t clr_bit)
{
    writel(CONFIG_SOC_REG_ADDR_PMU_INT_CLR, clr_bit);
}

void pmu_on_wakeup(uint8_t pd_no)
{
    uint32_t value;
    uint32_t intsts;

    if (pd_no > PD_MAX_NO)
        return;

    DI(intsts);

    value = (readl(CONFIG_SOC_REG_ADDR_PMU_PWR_ON_EN1) | (1UL << pd_no));

    writel(CONFIG_SOC_REG_ADDR_PMU_PWR_ON_EN1, value);

    EI(intsts);
}

#define PMU_PRIORITY_MASK 0x1fU
#define PMU_PRIORITY_FIELD_MAX 0x1fU

void pmu_write_power_on_priority(uint8_t pd_no, uint8_t value)
{
    uint32_t tmp, idx, bit_field_offset;
    uint32_t intsts;

    if (pd_no > PD_MAX_NO)
        return;

    if (value > PMU_PRIORITY_FIELD_MAX)
        return;

    /* calc on cycle idx*/
    idx = (pd_no >> 2);

    /* calc field offset*/
    bit_field_offset = (pd_no & 0x3U);

    DI(intsts);

    tmp = readl(pmu_pwr_on_dly_reg_addr[idx]);

    /* clear on cycle*/
    tmp &= ~(PMU_PRIORITY_MASK << (8 * bit_field_offset));

    /* set new on cycle*/
    tmp |= (value << (8 * bit_field_offset));

    writel(pmu_pwr_on_dly_reg_addr[idx], tmp);

    EI(intsts);
}

void pmu_write_pwr_cyc_sel(uint32_t value)
{
    writel(CONFIG_SOC_REG_ADDR_PMU_PWR_CYC_SEL, value);
}

uint32_t pmu_read_pwr_cyc_sel(void)
{
    return readl(CONFIG_SOC_REG_ADDR_PMU_PWR_CYC_SEL);
}
