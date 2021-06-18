/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      MORELLO PLL register definitions
 */

#ifndef INTERNAL_MORELLO_PLL_H
#define INTERNAL_MORELLO_PLL_H

#include <fwk_macros.h>

/* PLL Control Register 0 bit positions. */
#define PLL_HARD_BYPASS_BIT_POS 0
#define PLL_PD_TIMER_BYPASS_BIT_POS 1
#define PLL_LOCK_SEL_BIT_POS 2
#define PLL_DSMEN_BIT_POS 4
#define PLL_DACEN_BIT_POS 5
#define PLL_BYPASS_POS 6
#define PLL_FBDIV_BIT_POS 8
#define PLL_REFDIV_POS 20
#define PLL_PLLEN_POS 31

/* PLL Control Register 1 bit positions. */
#define PLL_FRAC_POS 0
#define PLL_POSTDIV1_POS 24
#define PLL_POSTDIV2_POS 28
#define PLL_LOCK_STATUS_POS 31

/*! The minimum reference frequency post REFDIV stage */
#define MOD_MORELLO_PLL_REF_MIN (2UL * FWK_MHZ)

/*! The minimum frequency that the PLL hardware can output. */
#define MOD_MORELLO_PLL_RATE_MIN (16UL * FWK_MHZ)

/*! The maximum frequency that the PLL hardware can output. */
#define MOD_MORELLO_PLL_RATE_MAX (3200UL * FWK_MHZ)

/*! Step size for the PLL. */
#define MOD_MORELLO_PLL_STEP_SIZE UINT64_C(1000)

/*! The minimum feedback divider value */
#define MOD_MORELLO_PLL_FBDIV_MIN 16
/*! The maximum feedback divider value */
#define MOD_MORELLO_PLL_FBDIV_MAX 1600

/*! The minimum frequency output that post divider requires. */
#define MOD_MORELLO_PLL_FVCO_MIN (800UL * FWK_MHZ)
/*! The maximum frequency output that post divider handles. */
#define MOD_MORELLO_PLL_FVCO_MAX (3200UL * FWK_MHZ)

/*! The minimum reference clock divider value */
#define MOD_MORELLO_PLL_REFDIV_MIN 1
/*! The maximum reference clock divider value */
#define MOD_MORELLO_PLL_REFDIV_MAX 63

/*! The minimum post divider value */
#define MOD_MORELLO_PLL_POSTDIV_MIN 1
/*! The maximum post divider value */
#define MOD_MORELLO_PLL_POSTDIV_MAX 7

#endif /* INTERNAL_MORELLO_PLL_H */
