/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_CLOCK_IDS_H
#define JUNO_CLOCK_IDS_H

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

#endif /* JUNO_CLOCK_IDS_H */
