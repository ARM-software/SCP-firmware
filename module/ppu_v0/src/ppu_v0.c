/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ppu_v0.h"

#include <fwk_assert.h>
#include <fwk_status.h>

#include <stddef.h>

struct set_power_status_check_params_v0 {
    enum ppu_v0_mode mode;
    struct ppu_v0_reg *reg;
};

static bool ppu_v0_set_power_status_check(void *data)
{
    struct set_power_status_check_params_v0 *params;

    fwk_assert(data != NULL);
    params = (struct set_power_status_check_params_v0 *)data;

    return (
        (params->reg->POWER_STATUS &
         (PPU_V0_PSR_POWSTAT | PPU_V0_PSR_DYNAMIC)) == params->mode);
}

void ppu_v0_init(struct ppu_v0_reg *ppu)
{
    fwk_assert(ppu != NULL);

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
    fwk_assert(ppu != NULL);
    fwk_assert(mode < PPU_V0_MODE_COUNT);

    power_policy = ppu->POWER_POLICY &
        ~(PPU_V0_PPR_POLICY | PPU_V0_PPR_DYNAMIC_EN);
    ppu->POWER_POLICY = power_policy | mode;

    return FWK_SUCCESS;
}

int ppu_v0_set_power_mode(
    struct ppu_v0_reg *ppu,
    enum ppu_v0_mode mode,
    struct ppu_v0_timer_ctx *timer_ctx)
{
    int status;
    fwk_assert(ppu != NULL);
    struct set_power_status_check_params_v0 params;

    status = ppu_v0_request_power_mode(ppu, mode);
    if (status != FWK_SUCCESS)
        return status;
    if (timer_ctx == NULL) {
        while ((ppu->POWER_STATUS &
                (PPU_V0_PSR_POWSTAT | PPU_V0_PSR_DYNAMIC)) != mode)
            continue;
    } else {
        params.mode = mode;
        params.reg = ppu;
        return timer_ctx->timer_api->wait(
            timer_ctx->timer_id,
            timer_ctx->delay_us,
            ppu_v0_set_power_status_check,
            &params);
    }

    return FWK_SUCCESS;
}

int ppu_v0_get_power_mode(struct ppu_v0_reg *ppu, enum ppu_v0_mode *mode)
{
    fwk_assert(ppu != NULL);
    fwk_assert(mode != NULL);

    *mode = (enum ppu_v0_mode)(ppu->POWER_STATUS & PPU_V0_PSR_POWSTAT);

    return FWK_SUCCESS;
}
