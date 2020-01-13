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
    PPU_CORES0_IRQ             = 50, /* Consolidated PPU Interrupt for cores
                                        1-32, 129-160 */
    PPU_CORES1_IRQ             = 51, /* Consolidated PPU Interrupt for cores
                                        33-64, 161-192 */
    PPU_CORES2_IRQ             = 52, /* Consolidated PPU Interrupt for cores
                                        65-96, 193-224 */
    PPU_CORES3_IRQ             = 53, /* Consolidated PPU Interrupt for cores
                                        97-128, 225-256 */
    PPU_CLUSTERS_IRQ           = 54, /* Consolidated clusters PPU */
};

#endif /* SCP_RDDANIEL_IRQ_H */
