/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSTEM_CLOCK_H
#define SYSTEM_CLOCK_H

#include <fwk_macros.h>

/*!
 * \brief Calculates the necessary divider for obtaining a target frequency
 *      from a given clock.
 *
 * \param CLOCK_RATE The tick rate of the clock to be divided.
 *
 * \param TARGET_FREQ The target frequency to be obtained by the division.
 *
 * \return The divider needed to obtain TARGET_FREQ from CLOCK_RATE.
 */
#define DIV_FROM_CLOCK(CLOCK_RATE, TARGET_FREQ) ((CLOCK_RATE) / (TARGET_FREQ))

#define CLOCK_RATE_REFCLK       (50UL * FWK_MHZ)
#define CLOCK_RATE_SYSPLLCLK    (2000UL * FWK_MHZ)

#endif /* SYSTEM_CLOCK_H */
