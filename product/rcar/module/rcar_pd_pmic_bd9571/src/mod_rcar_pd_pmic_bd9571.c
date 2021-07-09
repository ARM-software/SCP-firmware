/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_power_domain.h>
#include <config_rcar_pd_pmic_bd9571.h>

#include <mod_power_domain.h>
#include <mod_rcar_pd_pmic_bd9571.h>
#include <mod_rcar_pmic.h>
#include <mod_rcar_system.h>

#include <fwk_id.h>
#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_log.h>

#include <stdint.h>

/*
 * Internal variables
 */
static struct rcar_pmic_ctx rcar_pmic_ctx;

/*
 * Power domain driver interface
 */


static int pd_set_state(fwk_id_t pd_id, unsigned int state)
{
    struct rcar_pmic_pd_ctx *pd_ctx;
    int ret = FWK_SUCCESS;

    pd_ctx = rcar_pmic_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    ret = pd_ctx->api->set_pmic(pd_ctx->config->psu_id, state);

    switch (state) {
    case MOD_PD_STATE_ON:
        pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_ON);
        break;

    case MOD_PD_STATE_OFF:
        pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_OFF);
        break;

    default:
        FWK_LOG_ERR("[PD_PMIC] Requested power state (%i) is not supported.",
                        state);
        return FWK_E_PARAM;
    }

    pd_ctx->current_state = state;

    return ret;
}

static int pd_get_state(fwk_id_t pd_id, unsigned int *state)
{
    struct rcar_pmic_pd_ctx *pd_ctx;

    pd_ctx = rcar_pmic_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);
    *state = pd_ctx->current_state;

    return FWK_SUCCESS;
}

static int pd_reset(fwk_id_t pd_id)
{
    return FWK_SUCCESS;
}

static int pd_pmic_resume(void)
{
    struct rcar_pmic_pd_ctx *pd_ctx;

    pd_ctx = &rcar_pmic_ctx.pd_ctx_table[
                    RCAR_PD_PMIC_ELEMENT_IDX_PMIC_DDR_BKUP];
    pd_ctx->pd_driver_input_api->set_state(
        pd_ctx->bound_id, false, MOD_PD_STATE_OFF);

    return FWK_SUCCESS;
}
static const struct mod_pd_driver_api pd_driver = {
    .set_state = pd_set_state,
    .get_state = pd_get_state,
    .reset = pd_reset,
};

static const struct mod_rcar_system_drv_api api_system = {
    .resume = pd_pmic_resume,
};

/*
 * Framework handlers
 */

static int rcar_pmic_mod_init(
    fwk_id_t module_id,
    unsigned int pd_count,
    const void *unused)
{
    rcar_pmic_ctx.pd_ctx_table = fwk_mm_calloc(pd_count,
                                            sizeof(struct rcar_pmic_pd_ctx));
    if (rcar_pmic_ctx.pd_ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int rcar_pmic_pd_init(
    fwk_id_t pd_id,
    unsigned int unused,
    const void *data)
{
    const struct mod_rcar_pd_pmic_config *config = data;
    struct rcar_pmic_pd_ctx *pd_ctx;

    pd_ctx = rcar_pmic_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);
    pd_ctx->config = config;

    if (config->always_on)
        pd_set_state(pd_id ,MOD_PD_STATE_ON);

    return FWK_SUCCESS;
}

static int rcar_pmic_bind(fwk_id_t id, unsigned int round)
{
    struct rcar_pmic_pd_ctx *pd_ctx;

    /* Nothing to do during the first round of calls where the power module
       will bind to the power domains of this module. */
    if ((round == 0) || fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    pd_ctx = rcar_pmic_ctx.pd_ctx_table + fwk_id_get_element_idx(id);

    fwk_module_bind(
        pd_ctx->config->psu_id,
        mod_rcar_pmic_api_id_device,
        &pd_ctx->api);

    if (fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE))
        return FWK_SUCCESS;

    switch (fwk_id_get_module_idx(pd_ctx->bound_id)) {
    case FWK_MODULE_IDX_POWER_DOMAIN:
        fwk_module_bind(
            pd_ctx->bound_id,
            mod_pd_api_id_driver_input,
            &pd_ctx->pd_driver_input_api);
        break;

    default:
        return FWK_E_SUPPORT;
    }

    return FWK_SUCCESS;
}

static int rcar_pmic_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id,
    const void **api)
{
    struct rcar_pmic_pd_ctx *pd_ctx;

    pd_ctx = rcar_pmic_ctx.pd_ctx_table + fwk_id_get_element_idx(target_id);

    if (fwk_id_get_api_idx(api_id) == MOD_RCAR_PD_PMIC_API_TYPE_SYSTEM) {
        *api = &api_system;
    } else {
        switch (pd_ctx->config->pd_type) {
        case RCAR_PD_TYPE_DEVICE:
        case RCAR_PD_TYPE_DEVICE_DEBUG:
            if (fwk_id_get_module_idx(source_id) ==
                FWK_MODULE_IDX_POWER_DOMAIN) {
                pd_ctx->bound_id = source_id;
                *api = &pd_driver;
                break;
            }
            assert(false);
            return FWK_E_ACCESS;

        default:
            (void)pd_driver;
            return FWK_E_SUPPORT;
        }

    }

    return FWK_SUCCESS;
}

const struct fwk_module module_rcar_pd_pmic_bd9571 = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RCAR_PD_PMIC_API_COUNT,
    .init = rcar_pmic_mod_init,
    .element_init = rcar_pmic_pd_init,
    .bind = rcar_pmic_bind,
    .process_bind_request = rcar_pmic_process_bind_request,
};
