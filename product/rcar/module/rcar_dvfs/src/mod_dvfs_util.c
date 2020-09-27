/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <mod_dvfs_private.h>

int __mod_dvfs_set_opp(
    const struct mod_dvfs_domain_ctx *ctx,
    const struct mod_dvfs_opp *new_opp)
{
    int status;
    struct mod_dvfs_opp current_opp;

    status = __mod_dvfs_get_current_opp(ctx, &current_opp);
    if (status != FWK_SUCCESS)
        return status;

    if (new_opp->voltage > current_opp.voltage) {
        /* Raise the voltage before raising the frequency */
        status = ctx->apis.psu->set_voltage(
            ctx->config->psu_id,
            new_opp->voltage);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;
    }

    if (new_opp->frequency != current_opp.frequency) {
        status = ctx->apis.clock->set_rate(
            ctx->config->clock_id,
            new_opp->frequency,
            MOD_CLOCK_ROUND_MODE_NONE);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;
    }

    if (new_opp->voltage < current_opp.voltage) {
        /* Lower the voltage after lowering the frequency */
        status = ctx->apis.psu->set_voltage(
            ctx->config->psu_id,
            new_opp->voltage);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;
    }

    return FWK_SUCCESS;
}

int __mod_dvfs_get_current_opp(
    const struct mod_dvfs_domain_ctx *ctx,
    struct mod_dvfs_opp *opp)
{
    int status;

    status = ctx->apis.clock->get_rate(
        ctx->config->clock_id,
        &opp->frequency);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    status = ctx->apis.psu->get_voltage(
        ctx->config->psu_id,
        &opp->voltage);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}
