/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_CLOCK_H
#define JUNO_CLOCK_H

#include "juno_scc.h"

#include <fwk_macros.h>

#include <stdint.h>

/* Number of entries of the HDLCD lookup table */
#define JUNO_CLOCK_HDLCD_LOOKUP_COUNT 566
/*
 * Number of entries of the HDLCD lookup table if the HIGH_PXLCLK_ENABLE option
 * is set.
 */
#define JUNO_CLOCK_HDLCD_LOOKUP_HIGH_PXCLK_ENABLE_COUNT 746

/* Juno clock indices */
enum juno_clock_idx {
    JUNO_CLOCK_IDX_I2SCLK,
    JUNO_CLOCK_IDX_HDLCDREFCLK,
    JUNO_CLOCK_IDX_HDLCDPXL,
    JUNO_CLOCK_IDX_HDLCD0,
    JUNO_CLOCK_IDX_HDLCD1,
    JUNO_CLOCK_IDX_BIGCLK,
    JUNO_CLOCK_IDX_LITTLECLK,
    JUNO_CLOCK_IDX_GPUCLK,
    JUNO_CLOCK_IDX_COUNT
};

/* Juno CDCEL937 clock indices */
enum juno_clock_cdcel937_idx {
    /*
     * Generated clocks:
     * These clocks are generated through this driver.
     */
    JUNO_CLOCK_CDCEL937_IDX_I2SCLK,
    JUNO_CLOCK_CDCEL937_IDX_HDLCDREFCLK,
    JUNO_CLOCK_CDCEL937_IDX_HDLCDPXL,

    /*
     * Derived clocks:
     * These clocks are used only as reference for
     * the HDLCD module acting as a HAL.
     */
    JUNO_CLOCK_CDCEL937_IDX_HDLCD0,
    JUNO_CLOCK_CDCEL937_IDX_HDLCD1,

    /* Number of CDCEL clocks */
    JUNO_CLOCK_CDCEL937_IDX_COUNT
};

/* Juno HDLCD clock indices */
enum juno_clock_hdlcd_idx {
    JUNO_CLOCK_HDLCD_IDX_HDLCD0,
    JUNO_CLOCK_HDLCD_IDX_HDLCD1,
    JUNO_CLOCK_HDLCD_IDX_COUNT
};

/* Juno SOC clock indices for the RAM firmware */
enum juno_clock_soc_ram_idx {
    JUNO_CLOCK_SOC_RAM_IDX_BIGCLK,
    JUNO_CLOCK_SOC_RAM_IDX_LITTLECLK,
    JUNO_CLOCK_SOC_RAM_IDX_GPUCLK,
    JUNO_CLOCK_SOC_RAM_IDX_COUNT
};

struct juno_clock_preset {
    /*
     * \brief Denominator value
     *
     * \note This value should be between 1 and 511
     */
    uint16_t M;
    /*
     * \brief Numerator value
     *
     * \note This value should be between 1 and 4095
     */
    uint16_t N;
    /*
     * \brief Output divider
     *
     * \note This value should be between 1 and 127
     */
    uint16_t PDIV;
} __attribute((packed));

struct juno_clock_lookup {
    struct pll_reg pll;
    uint32_t rate_hz;
    struct juno_clock_preset preset;
    /*! Reserved to keep the structure 32-bit aligned */
    uint8_t RESERVED[2];
};

struct juno_clock_hdlcd_lookup {
    uint32_t SIGNATURE_A;
    struct juno_clock_lookup CLK[JUNO_CLOCK_HDLCD_LOOKUP_COUNT];
    uint8_t RESERVED[24];
    uint32_t SIGNATURE_B;
};

struct juno_clock_hdlcd_lookup_high_pxlclk_enable {
    uint32_t SIGNATURE_A;
    struct juno_clock_lookup
        CLK[JUNO_CLOCK_HDLCD_LOOKUP_HIGH_PXCLK_ENABLE_COUNT];
    uint8_t RESERVED[24];
    uint32_t SIGNATURE_B;
};

#endif /* JUNO_CLOCK_H */
