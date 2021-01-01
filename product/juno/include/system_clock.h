/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System clock rate definitions.
 */

#ifndef SYSTEM_CLOCK_H
#define SYSTEM_CLOCK_H

#include <fwk_macros.h>

#define CLOCK_RATE_REFCLK   (50UL * FWK_MHZ)
#define CLOCK_RATE_SYSCLK   (1600UL * FWK_MHZ)

/* Default for Juno SoC */
#define SYSINCLK            (1600UL * FWK_MHZ)

/* Pixel clocks default values for Juno HDLCD */
#define PXL_CLK_IN_RATE     (47500000)
#define PXL_REF_CLK_RATE    (50UL * FWK_MHZ)

#endif /* SYSTEM_CLOCK_H */
