/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_TC0_IRQ_H
#define SCP_TC0_IRQ_H

#include <fwk_interrupt.h>

enum scp_tc0_interrupt {
    TIMREFCLK_IRQ              = 33, /* REFCLK Physical Timer */
};

#endif /* SCP_TC0_IRQ_H */
