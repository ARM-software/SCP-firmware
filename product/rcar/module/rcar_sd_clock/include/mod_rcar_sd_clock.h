/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_SD_CLOCK_H
#define MOD_RCAR_SD_CLOCK_H

#include <clock_sd_devices.h>
#include <rcar_mmap.h>

#include <mod_clock.h>
#include <mod_rcar_clock.h>

#include <fwk_element.h>

#include <stdint.h>

/*!
 * \addtogroup GroupRCARModule RCAR Product Modules
 * @{
 */

/*!
 * \defgroup GroupRCARSdClock SD clock
 * @{
 */

/*!
 * \brief Rate lookup entry.
 */
struct mod_rcar_sd_clock_rate {
    /*! Divider used to obtain the rate value. */
    uint32_t divider_num;
    /*! Value to be set for the divider. */
    uint32_t divider;
    /*! Divider maskbit value. */
    uint32_t divider_mask;
};

/*!
 * \brief Subsystem clock device configuration.
 */
struct mod_rcar_sd_clock_dev_config {
    /*! The type of the clock device. */
    enum mod_rcar_clock_type type;

    /*! Pointer to the clock's control register. */
    volatile uint32_t const control_reg;

    /*! enable / disable bit position. */
    volatile bool stop_clk;

    /*! enable / disable bit position. */
    volatile uint32_t const stop_clk_bit;

    /*! Parent clock id. */
    uint32_t parent;

    /*! Required initialization Clock divider. */
    bool need_hardware_init;

    /*! Fixed Clock divider. */
    uint32_t div;

    /*! Fixed Clock divider. */
    uint32_t multi;

    /*! The type of rates the clock provides (discrete or continuous) */
    enum mod_clock_rate_type rate_type;

    /*! Pointer to the clock's rate lookup table. */
    const struct mod_rcar_sd_clock_rate *rate_table;

    /*! The number of rates in the rate lookup table. */
    uint32_t rate_count;
};

/*!
 * @cond
 */

/* Device context */
struct rcar_sd_clock_dev_ctx {
    bool initialized;
    uint64_t current_rate;
    uint64_t *rate_table;
    enum mod_clock_state current_state;
    const struct mod_rcar_sd_clock_dev_config *config;
};

/* Module context */
struct rcar_sd_clock_ctx {
    struct rcar_sd_clock_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
    uint32_t parent_clk[CLOCK_PARENT_IDX_COUNT];
};

struct rcar_gen3_cpg_pll_config {
    char extal_div;
    char pll1_mult;
    char pll1_div;
    char pll3_mult;
    char pll3_div;
    char osc_prediv;
};

/* control register */
#define CPG_SD0CKCR (CPG_BASE + 0x0074)
#define CPG_SD1CKCR (CPG_BASE + 0x0078)
#define CPG_SD2CKCR (CPG_BASE + 0x0268)
#define CPG_SD3CKCR (CPG_BASE + 0x026C)
#define CPG_SDNCKCR_MASK 0x1F
#define CPG_SDNCKCR_SD_64 0x11
#define CPG_SDNCKCR_SD_32 0xD
#define CPG_SDNCKCR_SD_16 0x9
#define CPG_SDNCKCR_SD_8 0x5
#define CPG_SDNCKCR_SD_4 0x1
#define CPG_SDNCKCR_SD_2 0x0

#define CPG_CANFDCKCR (CPG_BASE + 0x0244)
#define CPG_CSI0CKCR (CPG_BASE + 0x000C)
#define CPG_MSOCKCR (CPG_BASE + 0x0014)
#define CPG_HDMICKCR (CPG_BASE + 0x0250)
#define CPG_CON_MASK 0x3F
#define CPG_CON_MAX 64

#define CPG_FRQCRB_ZTRFC_MASK 0xF00000
#define CPG_FRQCRB_ZTRFC_24 0x800000
#define CPG_FRQCRB_ZTRFC_18 0x700000
#define CPG_FRQCRB_ZTRFC_16 0x600000
#define CPG_FRQCRB_ZTRFC_12 0x500000
#define CPG_FRQCRB_ZTRFC_8 0x400000
#define CPG_FRQCRB_ZTRFC_6 0x300000
#define CPG_FRQCRB_ZTFC_MASK 0xF0000
#define CPG_FRQCRB_ZTFC_24 0x80000
#define CPG_FRQCRB_ZTFC_18 0x70000
#define CPG_FRQCRB_ZTFC_16 0x60000
#define CPG_FRQCRB_ZTFC_12 0x50000
#define CPG_FRQCRB_ZTFC_8 0x40000
#define CPG_FRQCRB_ZTFC_6 0x30000
#define CPG_FRQCRB_ZTFC_4 0x20000
#define CPG_FRQCRB_ZTRD2FC_MASK 0xF
#define CPG_FRQCRB_ZTRD2FC_24 0x8
#define CPG_FRQCRB_ZTRD2FC_18 0x7
#define CPG_FRQCRB_ZTRD2FC_16 0x6
#define CPG_FRQCRB_ZTRD2FC_12 0x5

#define CPG_PLL_CONFIG_INDEX(md) \
    ((((md)&BIT(14)) >> 11) | (((md)&BIT(13)) >> 11) | \
     (((md)&BIT(19)) >> 18) | (((md)&BIT(17)) >> 17))

static const struct rcar_gen3_cpg_pll_config cpg_pll_configs[16] = {
    /* EXTAL div PLL1 mult/div PLL3 mult/div OSC prediv */
    {
        1,
        192,
        1,
        192,
        1,
        16,
    },
    {
        1,
        192,
        1,
        128,
        1,
        16,
    },
    { 0, /* Prohibited setting */ },
    {
        1,
        192,
        1,
        192,
        1,
        16,
    },
    {
        1,
        160,
        1,
        160,
        1,
        19,
    },
    {
        1,
        160,
        1,
        106,
        1,
        19,
    },
    { 0, /* Prohibited setting */ },
    {
        1,
        160,
        1,
        160,
        1,
        19,
    },
    {
        1,
        128,
        1,
        128,
        1,
        24,
    },
    {
        1,
        128,
        1,
        84,
        1,
        24,
    },
    { 0, /* Prohibited setting */ },
    {
        1,
        128,
        1,
        128,
        1,
        24,
    },
    {
        2,
        192,
        1,
        192,
        1,
        32,
    },
    {
        2,
        192,
        1,
        128,
        1,
        32,
    },
    { 0, /* Prohibited setting */ },
    {
        2,
        192,
        1,
        192,
        1,
        32,
    },
};

/*!
 * @endcond
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_RCAR_SD_CLOCK_H */
