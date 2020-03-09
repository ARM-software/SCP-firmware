/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_SOC_CLOCK_RAM_PLL_H
#define JUNO_SOC_CLOCK_RAM_PLL_H

#include "juno_scc.h"

#include <mod_juno_soc_clock_ram.h>

#include <stdint.h>

void juno_soc_clock_ram_pll_init(void);

int juno_soc_clock_ram_pll_set(
    struct pll_reg *pll,
    const struct juno_soc_clock_ram_pll_setting *pll_setting);

int juno_soc_clock_ram_pll_get(struct pll_reg *pll,
                               uint32_t refclk_hz,
                               uint32_t *freq_hz);

#endif /* JUNO_SOC_CLOCK_RAM_PLL_H */
