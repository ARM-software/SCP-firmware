/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Common CSS definitions shared across SCP, MCP and LCP.
 */

#ifndef CSS_COMMON_H
#define CSS_COMMON_H

#include <fwk_macros.h>

/* REF_CLK input clock speed */
#define CLOCK_RATE_REFCLK (125UL * FWK_MHZ)

/*
 * System Counter per-tick increment value required for 1GHz clock speed as
 * required for SBSA compliance. That is, (1GHz / CLOCK_RATE_REFCLK) = 8.
 */
#define SYSCNT_INCR 8

#endif /* CSS_COMMON_H */
