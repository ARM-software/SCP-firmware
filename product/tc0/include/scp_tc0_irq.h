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
    MHU_AP_NONSEC_IRQ          = 82, /* MHU non-secure irq bewteen SCP and AP */
    MHU_AP_SEC_IRQ             = 83, /* MHU secure irq bewteen SCP and AP */
};

#endif /* SCP_TC0_IRQ_H */
