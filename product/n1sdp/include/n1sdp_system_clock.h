/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_SYSTEM_CLOCK_H
#define N1SDP_SYSTEM_CLOCK_H

#include <fwk_macros.h>

#define CLOCK_RATE_REFCLK       (100UL * FWK_MHZ)
#define CLOCK_RATE_SYSPLLCLK    (2400UL * FWK_MHZ)

#define BAUD_RATE_19200         (19200)
#define BAUD_RATE_38400         (38400)
#define BAUD_RATE_115200        (115200)

#endif /* N1SDP_SYSTEM_CLOCK_H */
