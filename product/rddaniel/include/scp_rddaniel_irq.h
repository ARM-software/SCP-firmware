/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_RDDANIEL_IRQ_H
#define SCP_RDDANIEL_IRQ_H

enum scp_rddaniel_interrupt {
    SOC_WAKEUP0_IRQ            = 16, /* SoC Expansion Wakeup */
    TIMREFCLK_IRQ              = 33, /* REFCLK Physical Timer */
    MHU_AP_SEC_IRQ             = 83, /* MHU secure irq bewteen SCP and AP */
};

#endif /* SCP_RDDANIEL_IRQ_H */
