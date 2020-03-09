/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <ppu_v0.h>

#include <fwk_assert.h>
#include <fwk_status.h>

#include <stddef.h>

void ppu_v0_init(struct ppu_v0_reg *ppu)
{
    assert(ppu != NULL);

    /* Set mode as masked to all input edge interrupts */
    ppu->IESR = 0;

    /* Mask all interrupts */
    ppu->IMR = PPU_V0_IMR_MASK;

    /* Acknowledge any interrupt left pending */
    ppu->ISR = PPU_V0_ISR_MASK;
}

int ppu_v0_request_power_mode(struct ppu_v0_reg *ppu, enum ppu_v0_mode mode)
{
    uint32_t power_policy;
    assert(ppu != NULL);
    assert(mode < PPU_V0_MODE_COUNT);

    power_policy =
        ppu->POWER_POLICY & ~(PPU_V0_PPR_POLICY | PPU_V0_PPR_DYNAMIC_EN);
    ppu->POWER_POLICY = power_policy | mode;

    return FWK_SUCCESS;
}

int ppu_v0_set_power_mode(struct ppu_v0_reg *ppu, enum ppu_v0_mode mode)
{
    int status;
    assert(ppu != NULL);

    status = ppu_v0_request_power_mode(ppu, mode);
    if (status != FWK_SUCCESS)
        return status;

    while ((ppu->POWER_STATUS & (PPU_V0_PSR_POWSTAT | PPU_V0_PSR_DYNAMIC)) !=
           mode)
        continue;

    return FWK_SUCCESS;
}

int ppu_v0_get_power_mode(struct ppu_v0_reg *ppu, enum ppu_v0_mode *mode)
{
    assert(ppu != NULL);
    assert(mode != NULL);

    *mode = (enum ppu_v0_mode)(ppu->POWER_STATUS & PPU_V0_PSR_POWSTAT);

    return FWK_SUCCESS;
}
