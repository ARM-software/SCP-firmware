/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_CLOCK_H
#define JUNO_CLOCK_H

#include "juno_scc.h"
#include "juno_clock_ids.h"

#include <fwk_attributes.h>
#include <fwk_macros.h>

#include <stdint.h>

/* Number of entries of the HDLCD lookup table */
#define JUNO_CLOCK_HDLCD_LOOKUP_COUNT 566
/*
 * Number of entries of the HDLCD lookup table if the HIGH_PXLCLK_ENABLE option
 * is set.
 */
#define JUNO_CLOCK_HDLCD_LOOKUP_HIGH_PXCLK_ENABLE_COUNT 746



struct FWK_PACKED juno_clock_preset {
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
};

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
