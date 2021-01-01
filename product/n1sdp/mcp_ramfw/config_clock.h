/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_CLOCK_H
#define CONFIG_CLOCK_H

#include <fwk_macros.h>

/*
 * PIK clock rates.
 */
#define PIK_CLK_RATE_MCP_CORECLK  (300 * FWK_MHZ)
#define PIK_CLK_RATE_MCP_AXICLK   (300 * FWK_MHZ)

/*
 * PIK clock indexes.
 */
enum clock_pik_idx {
    CLOCK_PIK_IDX_MCP_CORECLK,
    CLOCK_PIK_IDX_MCP_AXICLK,
    CLOCK_PIK_IDX_COUNT
};

#endif /* CONFIG_CLOCK_H */
