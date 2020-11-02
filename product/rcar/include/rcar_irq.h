/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_IRQ_H
#define RCAR_IRQ_H

#include <fwk_interrupt.h>

enum rcar_irq {
    /**/
    MIN_IRQ = 16,
    VIRTUAL_TIMER_IRQ = 27,
    NS_PHYSICAL_TIMER_IRQ = 30,
    /**/
    MFIS_AREICR1_IRQ = 257,
    MFIS_AREICR2_IRQ = 258,
    MFIS_AREICR3_IRQ = 259,
    MFIS_AREICR4_IRQ = 260,
    MFIS_AREICR5_IRQ = 261,
    /**/ /* The following numbers are virtual IID */
    SMCMH_IRQ_START = 1024,
    SMCMH_SECURE_IRQ = SMCMH_IRQ_START, /* SMC Secure */
    SMCMH_LOW_PRIO_IRQ, /* SMC Low Priority */
    SMCMH_HIGH_PRIO_IRQ, /* SMC High Priority */
    SMCMH_IRQ_END,
    MAX_IRQ = SMCMH_IRQ_END,
    /**/
};

#endif /* RCAR_IRQ_H */
