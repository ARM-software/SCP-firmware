/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2022, Arm Limited and Contributors. All rights reserved.
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
#define DIV_FROM_CLOCK(CLOCK_RATE, TARGET_FREQ) (CLOCK_RATE / (TARGET_FREQ))

/*
 * Clock references
 */
#define CLOCK_RATE_SYSINCLK (1000UL * FWK_MHZ)
#define CLOCK_RATE_REFCLK (100UL * FWK_MHZ)
#define CLOCK_RATE_DDRPLLCLK (1066UL * FWK_MHZ)
#define CLOCK_RATE_SCP_PL011CLK (100000 * FWK_KHZ)
#define CLOCK_RATE_AP_PL011CLK (62500 * FWK_KHZ)

#if defined(CONFIG_SCB_USE_SCP_PL011)
#define CLOCK_RATE_UART CLOCK_RATE_SCP_PL011CLK
#elif defined(CONFIG_SCB_USE_AP_PL011)
#define CLOCK_RATE_UART CLOCK_RATE_AP_PL011CLK
#else
#define CLOCK_RATE_UART (100000 * FWK_KHZ)
#endif

#endif /* SYSTEM_CLOCK_H */
