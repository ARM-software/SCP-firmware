/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Clock driver for the big/little cluster and GPU clocks of the Juno SoC
 */

#include "juno_scc.h"
#include "juno_soc_clock_ram_pll.h"
#include "scp_config.h"

#include <mod_juno_soc_clock_ram.h>

#include <fwk_assert.h>
#include <fwk_status.h>

#include <stdint.h>
#include <stdlib.h>

/*
 * Static helper functions
 */
static uint32_t pll_config_freq(uint32_t refclk_hz, unsigned int feedback_div,
    unsigned int ref_div, unsigned int output_div)
{
    return (refclk_hz * feedback_div) / (ref_div * output_div);
}

/*
 * API functions
 */

void juno_soc_clock_ram_pll_init(void)
{
    unsigned int pll_idx;
    const uint32_t pll_mask = SCP_CONFIG_SCP_STATUS_SYSINPLLLOCK |
                              SCP_CONFIG_SCP_STATUS_GPUINPLLLOCK |
                              SCP_CONFIG_SCP_STATUS_LITTLEINPLLLOCK |
                              SCP_CONFIG_SCP_STATUS_BIGINPLLLOCK;

    /* Release all system PLLs from reset */
    for (pll_idx = 0; pll_idx < PLL_IDX_COUNT; pll_idx++)
        SCC->PLL[pll_idx].REG0 &= ~PLL_REG0_PLL_RESET;

    /* Wait for PLLs to lock */
    while ((SCP_CONFIG->SCP_STATUS & pll_mask) != pll_mask)
        continue;
}

int juno_soc_clock_ram_pll_set(struct pll_reg *pll,
    const struct juno_soc_clock_ram_pll_setting *pll_setting)
{
    fwk_assert(pll != NULL);
    fwk_assert(pll_setting != NULL);

    fwk_assert((pll_setting->nf >= 2) && (pll_setting->nf <= PLL_NF_MAX));
    fwk_assert((pll_setting->nr >= 1) && (pll_setting->nr <= PLL_NR_MAX));
    fwk_assert((pll_setting->od >= 1) && (pll_setting->od <= PLL_OD_MAX));

    pll->REG0 = PLL_REG0_PLL_RESET |
            ((pll_setting->nf - 1) << PLL_REG0_NF_POS) |
            (1 << PLL_REG0_ENSAT_POS);

    pll->REG1 = ((pll_setting->nr - 1) |
            ((pll_setting->od - 1) << PLL_REG1_OD_POS) |
            ((pll_setting->nf / 2) - 1) << PLL_REG1_NB_POS);

    pll->REG0 &= ~PLL_REG0_PLL_RESET;

    /* Wait for the PLL to lock */
    while (!(pll->REG1 & PLL_REG1_LOCK_STATUS))
        continue;

    return FWK_SUCCESS;
}

int juno_soc_clock_ram_pll_get(struct pll_reg *pll, uint32_t refclk_hz,
    uint32_t *freq_hz)
{
    unsigned int feedback_div;
    unsigned int ref_div;
    unsigned int output_div;

    if ((pll == NULL) || (freq_hz == NULL))
        return FWK_E_PARAM;

    feedback_div = ((pll->REG0 & PLL_REG0_NF) >> PLL_REG0_NF_POS) + 1;
    ref_div = ((pll->REG1 & PLL_REG1_NR) >> PLL_REG1_NR_POS) + 1;
    output_div = ((pll->REG1 & PLL_REG1_OD) >> PLL_REG1_OD_POS) + 1;

    *freq_hz = pll_config_freq(refclk_hz, feedback_div, ref_div, output_div);

    return FWK_SUCCESS;
}
