/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* The use of "subordinate" may not be in sync with platform documentation */

#include <mmio.h>

#include <mod_clock.h>
#include <mod_rcar_clock.h>

#include <fwk_element.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

static struct rcar_clock_ctx module_ctx;
static int current_a57_opp_limit = 0;
static const struct op_points *current_a57_opp_table;
static int current_a53_opp_limit = 0;
static const struct op_points *current_a53_opp_table;
static int dvfs_inited = 0;

/* TODO These should be taken from avs_driver.c */
#define EFUSE_AVS0 (0U)
#define EFUSE_AVS_NUM (8U)
static uint32_t efuse_avs = EFUSE_AVS0;

/* Describe OPPs exactly how they are described in the device-tree */
static const struct op_points rcar_h3_a57_op_points[EFUSE_AVS_NUM]
                                                   [NR_H3_A57_OPP] = {
                                                       {
                                                           {
                                                               500000000,
                                                               830000,
                                                           },
                                                           {
                                                               1000000000,
                                                               830000,
                                                           },
                                                           {
                                                               1500000000,
                                                               830000,
                                                           },
                                                           {
                                                               1600000000,
                                                               900000,
                                                           },
                                                           {
                                                               1700000000,
                                                               960000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               820000,
                                                           },
                                                           {
                                                               1000000000,
                                                               820000,
                                                           },
                                                           {
                                                               1500000000,
                                                               820000,
                                                           },
                                                           {
                                                               1600000000,
                                                               890000,
                                                           },
                                                           {
                                                               1700000000,
                                                               950000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               810000,
                                                           },
                                                           {
                                                               1000000000,
                                                               810000,
                                                           },
                                                           {
                                                               1500000000,
                                                               810000,
                                                           },
                                                           {
                                                               1600000000,
                                                               880000,
                                                           },
                                                           {
                                                               1700000000,
                                                               930000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               800000,
                                                           },
                                                           {
                                                               1000000000,
                                                               800000,
                                                           },
                                                           {
                                                               1500000000,
                                                               800000,
                                                           },
                                                           {
                                                               1600000000,
                                                               870000,
                                                           },
                                                           {
                                                               1700000000,
                                                               910000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               790000,
                                                           },
                                                           {
                                                               1000000000,
                                                               790000,
                                                           },
                                                           {
                                                               1500000000,
                                                               790000,
                                                           },
                                                           {
                                                               1600000000,
                                                               860000,
                                                           },
                                                           {
                                                               1700000000,
                                                               890000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               780000,
                                                           },
                                                           {
                                                               1000000000,
                                                               780000,
                                                           },
                                                           {
                                                               1500000000,
                                                               780000,
                                                           },
                                                           {
                                                               1600000000,
                                                               850000,
                                                           },
                                                           {
                                                               1700000000,
                                                               880000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               770000,
                                                           },
                                                           {
                                                               1000000000,
                                                               770000,
                                                           },
                                                           {
                                                               1500000000,
                                                               770000,
                                                           },
                                                           {
                                                               1600000000,
                                                               840000,
                                                           },
                                                           {
                                                               1700000000,
                                                               870000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               760000,
                                                           },
                                                           {
                                                               1000000000,
                                                               760000,
                                                           },
                                                           {
                                                               1500000000,
                                                               760000,
                                                           },
                                                           {
                                                               1600000000,
                                                               830000,
                                                           },
                                                           {
                                                               1700000000,
                                                               860000,
                                                           },
                                                       },
                                                   };

static const struct op_points rcar_h3_a53_op_points[NR_H3_A53_OPP] = {
    {
        800000000,
        820000,
    },
    {
        1000000000,
        820000,
    },
    {
        1200000000,
        820000,
    },
};

static const struct op_points rcar_m3_a57_op_points[EFUSE_AVS_NUM]
                                                   [NR_M3_A57_OPP] = {
                                                       {
                                                           {
                                                               500000000,
                                                               830000,
                                                           },
                                                           {
                                                               1000000000,
                                                               830000,
                                                           },
                                                           {
                                                               1500000000,
                                                               830000,
                                                           },
                                                           {
                                                               1600000000,
                                                               900000,
                                                           },
                                                           {
                                                               1700000000,
                                                               900000,
                                                           },
                                                           {
                                                               1800000000,
                                                               960000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               820000,
                                                           },
                                                           {
                                                               1000000000,
                                                               820000,
                                                           },
                                                           {
                                                               1500000000,
                                                               820000,
                                                           },
                                                           {
                                                               1600000000,
                                                               890000,
                                                           },
                                                           {
                                                               1700000000,
                                                               890000,
                                                           },
                                                           {
                                                               1800000000,
                                                               950000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               810000,
                                                           },
                                                           {
                                                               1000000000,
                                                               810000,
                                                           },
                                                           {
                                                               1500000000,
                                                               810000,
                                                           },
                                                           {
                                                               1600000000,
                                                               880000,
                                                           },
                                                           {
                                                               1700000000,
                                                               880000,
                                                           },
                                                           {
                                                               1800000000,
                                                               930000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               800000,
                                                           },
                                                           {
                                                               1000000000,
                                                               800000,
                                                           },
                                                           {
                                                               1500000000,
                                                               800000,
                                                           },
                                                           {
                                                               1600000000,
                                                               870000,
                                                           },
                                                           {
                                                               1700000000,
                                                               870000,
                                                           },
                                                           {
                                                               1800000000,
                                                               910000,
                                                           },
                                                       },

                                                       {
                                                           {
                                                               500000000,
                                                               790000,
                                                           },
                                                           {
                                                               1000000000,
                                                               790000,
                                                           },
                                                           {
                                                               1500000000,
                                                               790000,
                                                           },
                                                           {
                                                               1600000000,
                                                               860000,
                                                           },
                                                           {
                                                               1700000000,
                                                               860000,
                                                           },
                                                           {
                                                               1800000000,
                                                               890000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               780000,
                                                           },
                                                           {
                                                               1000000000,
                                                               780000,
                                                           },
                                                           {
                                                               1500000000,
                                                               780000,
                                                           },
                                                           {
                                                               1600000000,
                                                               850000,
                                                           },
                                                           {
                                                               1700000000,
                                                               850000,
                                                           },
                                                           {
                                                               1800000000,
                                                               880000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               770000,
                                                           },
                                                           {
                                                               1000000000,
                                                               770000,
                                                           },
                                                           {
                                                               1500000000,
                                                               770000,
                                                           },
                                                           {
                                                               1600000000,
                                                               840000,
                                                           },
                                                           {
                                                               1700000000,
                                                               840000,
                                                           },
                                                           {
                                                               1800000000,
                                                               870000,
                                                           },
                                                       },
                                                       {
                                                           {
                                                               500000000,
                                                               760000,
                                                           },
                                                           {
                                                               1000000000,
                                                               760000,
                                                           },
                                                           {
                                                               1500000000,
                                                               760000,
                                                           },
                                                           {
                                                               1600000000,
                                                               830000,
                                                           },
                                                           {
                                                               1700000000,
                                                               830000,
                                                           },
                                                           {
                                                               1800000000,
                                                               860000,
                                                           },
                                                       },
                                                   };

static const struct op_points rcar_m3_a53_op_points[NR_M3_A53_OPP] = {
    {
        800000000,
        820000,
    },
    {
        1000000000,
        820000,
    },
    {
        1200000000,
        820000,
    },
    {
        1300000000,
        820000,
    },
};

uint32_t rcar_dvfs_get_opp_voltage(int domain, int oppnr)
{
    if (domain == A57_DOMAIN) {
        if (oppnr < 0 || oppnr >= current_a57_opp_limit)
            return ~0;

        /* Protocol requires voltage to be in mV */
        return current_a57_opp_table[oppnr].volt / 1000;
    } else if (domain == A53_DOMAIN) {
        if (oppnr < 0 || oppnr >= current_a53_opp_limit)
            return ~0;

        /* Protocol requires voltage to be in mV */
        return current_a53_opp_table[oppnr].volt / 1000;
    }

    return ~0;
}

uint32_t rcar_dvfs_get_opp_frequency(int domain, int oppnr)
{
    if (domain == A57_DOMAIN) {
        if (oppnr < 0 || oppnr >= current_a57_opp_limit)
            return ~0;

        /* Protocol requires frequency to be in Hz */
        return current_a57_opp_table[oppnr].freq;
    } else if (domain == A53_DOMAIN) {
        if (oppnr < 0 || oppnr >= current_a53_opp_limit)
            return ~0;

        /* Protocol requires frequency to be in Hz */
        return current_a53_opp_table[oppnr].freq;
    }

    return ~0;
}

static unsigned long pll_clk_parent_rate(void)
{
    static const unsigned long extal_freq[] = {
        16660000U, /* MD14_MD13_TYPE_0 */
        20000000U, /* MD14_MD13_TYPE_1 */
        25000000U, /* MD14_MD13_TYPE_2 */
        33330000U, /* MD14_MD13_TYPE_3 */
    };
    unsigned long rate;
    int idx;

    idx = (mmio_read_32(RCAR_MODEMR) & MODEMR_BOOT_PLL_MASK) >>
        MODEMR_BOOT_PLL_SHIFT;

    rate = extal_freq[idx];
    /* Divider setting of EXTAL input is 1/2 when MD14=1 MD13=1 */
    if (idx == MD14_MD13_TYPE_3)
        rate = DIV_ROUND(rate, 2);

    return rate;
}

static unsigned long pll0_clk_round_rate(unsigned long rate)
{
    unsigned long parent_rate = pll_clk_parent_rate();
    unsigned int mult;

    if (rate < Z_CLK_MAX_THRESHOLD)
        rate = Z_CLK_MAX_THRESHOLD; /* Set lowest value: 1.5GHz */

    mult = DIV_ROUND(rate, parent_rate);
    mult = max(mult, 90U); /* Lowest value is 1.5GHz (stc == 90) */
    mult = min(mult, 108U);

    rate = parent_rate * mult;
    /* Round to closest value at 100MHz unit */
    rate = 100000000 * DIV_ROUND(rate, 100000000);

    return rate;
}

static unsigned long pll0_clk_recalc_rate(void)
{
    unsigned long parent_rate = pll_clk_parent_rate();
    unsigned int val;
    unsigned long rate;

    val =
        (mmio_read_32(CPG_PLL0CR) & CPG_PLLCR_STC_MASK) >> CPG_PLLCR_STC_SHIFT;

    rate = parent_rate * (val + 1);
    /* Round to closest value at 100MHz unit */
    rate = 100000000 * DIV_ROUND(rate, 100000000);

    return rate;
}

static int pll0_clk_set_rate(unsigned long rate)
{
    unsigned long parent_rate = pll_clk_parent_rate();
    unsigned int stc_val;
    uint32_t val;

    stc_val = DIV_ROUND(rate, parent_rate);
    stc_val = max(stc_val, 90U); /* Lowest value is 1.5GHz (stc == 90) */
    stc_val = min(stc_val, 108U);

    stc_val -= 1;
    val = mmio_read_32(CPG_PLL0CR);
    val &= ~CPG_PLLCR_STC_MASK;
    val |= stc_val << CPG_PLLCR_STC_SHIFT;
    mmio_write_32(CPG_PLL0CR, val);

    while (!(mmio_read_32(CPG_BASE + CPG_PLLECR) & CPG_PLLECR_PLL0ST))
        continue;

    return 0;
}

static unsigned long pll2_clk_round_rate(unsigned long rate)
{
    unsigned long parent_rate = pll_clk_parent_rate();
    unsigned int mult;

    if (rate < Z2_CLK_MAX_THRESHOLD)
        rate = Z2_CLK_MAX_THRESHOLD; /* Set lowest value: 1.2GHz */

    mult = DIV_ROUND(rate, parent_rate);
    mult = max(mult, 72U); /* Lowest value is 1.2GHz (stc == 72) */
    mult = min(mult, 78U);

    rate = parent_rate * mult;
    /* Round to closest value at 100MHz unit */
    rate = 100000000 * DIV_ROUND(rate, 100000000);

    return rate;
}

static unsigned long pll2_clk_recalc_rate(void)
{
    unsigned long parent_rate = pll_clk_parent_rate();
    unsigned int val;
    unsigned long rate;

    val =
        (mmio_read_32(CPG_PLL2CR) & CPG_PLLCR_STC_MASK) >> CPG_PLLCR_STC_SHIFT;

    rate = parent_rate * (val + 1);
    /* Round to closest value at 100MHz unit */
    rate = 100000000 * DIV_ROUND(rate, 100000000);

    return rate;
}

static int pll2_clk_set_rate(unsigned long rate)
{
    unsigned long parent_rate = pll_clk_parent_rate();
    unsigned int stc_val;
    uint32_t val;

    stc_val = DIV_ROUND(rate, parent_rate);
    stc_val = max(stc_val, 72U); /* Lowest value is 1.2GHz (stc == 72) */
    stc_val = min(stc_val, 78U);

    stc_val -= 1;
    val = mmio_read_32(CPG_PLL2CR);
    val &= ~CPG_PLLCR_STC_MASK;
    val |= stc_val << CPG_PLLCR_STC_SHIFT;
    mmio_write_32(CPG_PLL2CR, val);

    while (!(mmio_read_32(CPG_BASE + CPG_PLLECR) & CPG_PLLECR_PLL2ST))
        continue;

    return 0;
}

static unsigned long z_clk_round_rate(
    unsigned long rate,
    unsigned long *parent_rate)
{
    unsigned long prate = *parent_rate;
    unsigned int mult;

    if (!prate)
        prate = 1;

    if (rate <= Z_CLK_MAX_THRESHOLD) { /* Focus on changing z-clock */
        prate = Z_CLK_MAX_THRESHOLD; /* Set parent to: 1.5GHz */
        mult = DIV_ROUND(rate * 32, prate);
    } else {
        /* Focus on changing parent. Fix z-clock divider is 32/32 */
        mult = 32;
    }
    mult = max(mult, 1U);
    mult = min(mult, 32U);

    /* Re-calculate the parent_rate to propagate new rate for it */
    prate = DIV_ROUND(rate * 32, mult);
    prate = 100000000 * DIV_ROUND(prate, 100000000);
    rate = 100000000 * DIV_ROUND(prate / 32 * mult, 100000000);
    *parent_rate = prate;

    return rate;
}

static unsigned long z_clk_recalc_rate(unsigned long parent_rate)
{
    unsigned int mult;
    unsigned int val;
    unsigned long rate;

    val = (mmio_read_32(CPG_FRQCRC) & CPG_FRQCRC_ZFC_MASK) >>
        CPG_FRQCRC_ZFC_SHIFT;
    mult = 32 - val;

    rate = DIV_ROUND(parent_rate * mult, 32);
    /* Round to closest value at 100MHz unit */
    rate = 100000000 * DIV_ROUND(rate, 100000000);

    return rate;
}

static int z_clk_set_rate(unsigned long rate, unsigned long parent_rate)
{
    unsigned int mult;
    uint32_t val, kick;
    unsigned int i;

    if (rate <= Z_CLK_MAX_THRESHOLD) { /* Focus on changing z-clock */
        parent_rate = Z_CLK_MAX_THRESHOLD; /* Set parent to: 1.5GHz */
        mult = DIV_ROUND(rate * 32, parent_rate);
    } else {
        mult = 32;
    }
    mult = max(mult, 1U);
    mult = min(mult, 32U);

    if (mmio_read_32(CPG_FRQCRB) & CPG_FRQCRB_KICK)
        return -1;

    val = mmio_read_32(CPG_FRQCRC);
    val &= ~CPG_FRQCRC_ZFC_MASK;
    val |= (32 - mult) << CPG_FRQCRC_ZFC_SHIFT;
    mmio_write_32(CPG_FRQCRC, val);

    /*
     * Set KICK bit in FRQCRB to update hardware setting and wait for
     * clock change completion.
     */
    kick = mmio_read_32(CPG_FRQCRB);
    kick |= CPG_FRQCRB_KICK;
    mmio_write_32(CPG_FRQCRB, kick);

    /*
     * Note: There is no HW information about the worst case latency.
     *
     * Using experimental measurements, it seems that no more than
     * ~10 iterations are needed, independently of the CPU rate.
     * Since this value might be dependent of external xtal rate, pll1
     * rate or even the other emulation clocks rate, use 1000 as a
     * "super" safe value.
     */
    for (i = 1000; i; i--) {
        if (!(mmio_read_32(CPG_FRQCRB) & CPG_FRQCRB_KICK)) {
            return 0;
        }

        /*cpu_relax();*/
    }

    return -1;
}

static unsigned long z2_clk_round_rate(
    unsigned long rate,
    unsigned long *parent_rate)
{
    unsigned long prate = *parent_rate;
    unsigned int mult;

    if (!prate)
        prate = 1;

    if (rate <= Z2_CLK_MAX_THRESHOLD) { /* Focus on changing z2-clock */
        prate = Z2_CLK_MAX_THRESHOLD; /* Set parent to: 1.2GHz */
        mult = DIV_ROUND(rate * 32, prate);
    } else {
        /* Focus on changing parent. Fix z2-clock divider is 32/32 */
        mult = 32;
    }
    mult = max(mult, 1U);
    mult = min(mult, 32U);

    /* Re-calculate the parent_rate to propagate new rate for it */
    prate = DIV_ROUND(rate * 32, mult);
    prate = 100000000 * DIV_ROUND(prate, 100000000);
    rate = 100000000 * DIV_ROUND(prate / 32 * mult, 100000000);
    *parent_rate = prate;

    return rate;
}

static unsigned long z2_clk_recalc_rate(unsigned long parent_rate)
{
    unsigned int mult;
    unsigned int val;
    unsigned long rate;

    val = mmio_read_32(CPG_FRQCRC) & CPG_FRQCRC_Z2FC_MASK;
    mult = 32 - val;

    rate = DIV_ROUND(parent_rate * mult, 32);
    /* Round to closest value at 100MHz unit */
    rate = 100000000 * DIV_ROUND(rate, 100000000);

    return rate;
}

static int z2_clk_set_rate(unsigned long rate, unsigned long parent_rate)
{
    unsigned int mult;
    uint32_t val, kick;
    unsigned int i;

    if (rate <= Z2_CLK_MAX_THRESHOLD) { /* Focus on changing z2-clock */
        parent_rate = Z2_CLK_MAX_THRESHOLD; /* Set parent to: 1.2GHz */
        mult = DIV_ROUND(rate * 32, parent_rate);
    } else {
        mult = 32;
    }
    mult = max(mult, 1U);
    mult = min(mult, 32U);

    if (mmio_read_32(CPG_FRQCRB) & CPG_FRQCRB_KICK)
        return -1;

    val = mmio_read_32(CPG_FRQCRC);
    val &= ~CPG_FRQCRC_Z2FC_MASK;
    val |= 32 - mult;
    mmio_write_32(CPG_FRQCRC, val);

    /*
     * Set KICK bit in FRQCRB to update hardware setting and wait for
     * clock change completion.
     */
    kick = mmio_read_32(CPG_FRQCRB);
    kick |= CPG_FRQCRB_KICK;
    mmio_write_32(CPG_FRQCRB, kick);

    /*
     * Note: There is no HW information about the worst case latency.
     *
     * Using experimental measurements, it seems that no more than
     * ~10 iterations are needed, independently of the CPU rate.
     * Since this value might be dependent of external xtal rate, pll1
     * rate or even the other emulation clocks rate, use 1000 as a
     * "super" safe value.
     */
    for (i = 1000; i; i--) {
        if (!(mmio_read_32(CPG_FRQCRB) & CPG_FRQCRB_KICK)) {
            return 0;
        }

        /*cpu_relax();*/
    }

    return -1;
}

#if 0
static int set_voltage(unsigned long volt)
{
/* TODO : Not supported because it conflicts with I2C control from the kernel */
    uint8_t val;
    int ret;

    if (volt < BD9571MWV_MIN_MV * 1000 || volt > BD9571MWV_MAX_MV * 1000)
        return -1;

    val = DIV_ROUND(volt, BD9571MWV_STEP_MV * 1000);
    val &= REG_DATA_DVFS_SetVID_MASK;

    ret = rcar_iic_dvfs_send(SUBORDINATE_ADDR_PMIC, REG_ADDR_DVFS_SetVID, val);
    if (ret) {
        return ret;
    }
    return 0;
}
#endif

#if 0
static unsigned long get_voltage(void)
{
    uint8_t val;
    unsigned long volt;
    int ret;

    ret =
      rcar_iic_dvfs_receive(SUBORDINATE_ADDR_PMIC, REG_ADDR_DVFS_SetVID, &val);
    if (ret) {
        return ret;
    }

    val &= REG_DATA_DVFS_SetVID_MASK;
    volt = (unsigned long)val * BD9571MWV_STEP_MV * 1000;

    return volt;
}
#endif

static const struct op_points *find_opp(int domain, unsigned long freq)
{
    int i;

    if (domain == A57_DOMAIN) {
        for (i = 0; i < current_a57_opp_limit; i++) {
            if (current_a57_opp_table[i].freq == freq)
                return &current_a57_opp_table[i];
        }
    } else if (domain == A53_DOMAIN) {
        for (i = 0; i < current_a53_opp_limit; i++) {
            if (current_a53_opp_table[i].freq == freq)
                return &current_a53_opp_table[i];
        }
    }

    return NULL;
}

static int set_a57_opp(unsigned long target_freq)
{
    unsigned long freq, old_freq, prate, old_prate;
    const struct op_points *opp;
    int ret;

    prate = 0;
    freq = z_clk_round_rate(target_freq, &prate);

    old_prate = pll0_clk_recalc_rate();
    old_freq = z_clk_recalc_rate(old_prate);

    /* Return early if nothing to do */
    if (old_freq == freq) {
        return 0;
    }

    opp = find_opp(A57_DOMAIN, freq);
    if (!opp) {
        return -1;
    }

    /* Scaling up? Scale voltage before frequency */
    prate = pll0_clk_round_rate(prate);
    if (old_prate != prate)
        pll0_clk_set_rate(prate);

    ret = z_clk_set_rate(freq, prate);
    if (ret) {
        /* Restore voltage */
        if (old_prate != prate)
            pll0_clk_set_rate(old_prate);
    }

    return ret;
}

static int set_a53_opp(unsigned long target_freq)
{
    unsigned long freq, old_freq, prate, old_prate;
    const struct op_points *opp;
    int ret = 0;

    prate = 0;
    freq = z2_clk_round_rate(target_freq, &prate);

    old_prate = pll2_clk_recalc_rate();
    old_freq = z2_clk_recalc_rate(old_prate);

    /* Return early if nothing to do */
    if (old_freq == freq) {
        return 0;
    }

    opp = find_opp(A53_DOMAIN, freq);
    if (!opp) {
        return -1;
    }

    prate = pll2_clk_round_rate(prate);
    if (old_prate != prate)
        pll2_clk_set_rate(prate);

    ret = z2_clk_set_rate(freq, prate);

    return ret;
}

int rcar_dvfs_get_nr_opp(int domain)
{
    if (domain == A57_DOMAIN)
        return current_a57_opp_limit;
    else if (domain == A53_DOMAIN)
        return current_a53_opp_limit;

    return 0;
}

int rcar_dvfs_opp_init(void)
{
    uint32_t product;

    if (dvfs_inited)
        return 0;

    product = mmio_read_32(RCAR_PRR) & RCAR_PRODUCT_MASK;

    if (product == RCAR_PRODUCT_H3) {
        current_a57_opp_limit = ARRAY_SIZE(rcar_h3_a57_op_points[efuse_avs]);
        current_a57_opp_table = &rcar_h3_a57_op_points[efuse_avs][0];

        current_a53_opp_limit = ARRAY_SIZE(rcar_h3_a53_op_points);
        current_a53_opp_table = &rcar_h3_a53_op_points[0];
    } else if (product == RCAR_PRODUCT_M3) {
        current_a57_opp_limit = ARRAY_SIZE(rcar_m3_a57_op_points[efuse_avs]);
        current_a57_opp_table = &rcar_m3_a57_op_points[efuse_avs][0];

        current_a53_opp_limit = ARRAY_SIZE(rcar_m3_a53_op_points);
        current_a53_opp_table = &rcar_m3_a53_op_points[0];
    } else
        return -1;

    dvfs_inited = 1;

    return 0;
}

/*
 * Static helper functions
 */

static int do_rcar_clock_set_rate(
    fwk_id_t dev_id,
    uint64_t rate,
    enum mod_clock_round_mode round_mode)
{
    struct rcar_clock_dev_ctx *ctx;
    int ret = 0;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    /* base clock */
    switch (ctx->config->rate_table->divider_reg) {
    case MOD_RCAR_CLOCK_A57_DIVIDER_DIV_EXT:
        ret = set_a57_opp(rate);
        break;
    case MOD_RCAR_CLOCK_A53_DIVIDER_DIV_EXT:
        ret = set_a53_opp(rate);
        break;
    default:
        return FWK_E_SUPPORT;
    }
    ctx->current_rate = rate;
    return ret;
}

/*
 * Clock driver API functions
 */

static int rcar_clock_set_rate(
    fwk_id_t dev_id,
    uint64_t rate,
    enum mod_clock_round_mode round_mode)
{
    struct rcar_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!ctx->initialized)
        return FWK_E_INIT;

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED)
        return FWK_E_PWRSTATE;

    return do_rcar_clock_set_rate(dev_id, rate, round_mode);
}

static int rcar_clock_get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    struct rcar_clock_dev_ctx *ctx;

    if (rate == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *rate = ctx->current_rate;

    return FWK_SUCCESS;
}

static const struct mod_rcar_clock_drv_api api_clock = {
    .set_rate = rcar_clock_set_rate,
    .get_rate = rcar_clock_get_rate,
};

/*
 * Framework handler functions
 */

static int rcar_clock_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    struct mod_ext_clock_rate *ext;
    module_ctx.dev_count = element_count;

    if (element_count == 0)
        return FWK_SUCCESS;

    module_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct rcar_clock_dev_ctx));
    if (module_ctx.dev_ctx_table == NULL)
        return FWK_E_NOMEM;

    ext = (struct mod_ext_clock_rate *)data;
    module_ctx.extal_clk = ext->ext_clk_rate;

    return FWK_SUCCESS;
}

static int rcar_clock_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    unsigned int i = 0;
    uint64_t current_rate;
    uint64_t last_rate = 0;
    struct rcar_clock_dev_ctx *ctx;
    const struct mod_rcar_clock_dev_config *dev_config = data;

    if (!fwk_module_is_valid_element_id(element_id))
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);

    /* Verify that the rate entries in the device's lookup table are ordered */
    while (i < dev_config->rate_count) {
        current_rate = dev_config->rate_table[i].rate;

        /* The rate entries must be in ascending order */
        if (current_rate < last_rate)
            return FWK_E_DATA;

        last_rate = current_rate;
        i++;
    }

    ctx->config = dev_config;

    if (ctx->config->defer_initialization)
        return FWK_SUCCESS;

    ctx->current_state = MOD_CLOCK_STATE_RUNNING;
    ctx->initialized = true;

    /*
     * Clock devices that are members of a clock group must skip initialization
     * at this time since they will be set to a specific rate by the CSS Clock
     * driver during the start stage or in response to a notification.
     */
    if (ctx->config->is_group_member)
        return FWK_SUCCESS;

    return do_rcar_clock_set_rate(
        element_id, dev_config->initial_rate, MOD_CLOCK_ROUND_MODE_NONE);
}

static int rcar_clock_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &api_clock;
    return FWK_SUCCESS;
}
static int rcar_clock_start(fwk_id_t id)
{
    /* for Not Module */
    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    rcar_dvfs_opp_init();
    return FWK_SUCCESS;
}

const struct fwk_module module_rcar_clock = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RCAR_CLOCK_API_COUNT,
    .event_count = 0,
    .init = rcar_clock_init,
    .element_init = rcar_clock_element_init,
    .process_bind_request = rcar_clock_process_bind_request,
    .start = rcar_clock_start,
};
