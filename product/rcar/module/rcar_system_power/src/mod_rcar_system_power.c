/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_power_domain.h>
#include <mod_system_power.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/* Module context */
struct system_power_ctx {
    /* Log API pointer */
    const struct mod_log_api *log_api;

    /* SYS0 power domain driver API pointer */
    const struct mod_pd_driver_api *sys0_api;

    /* SYS1 power domain driver API pointer*/
    const struct mod_pd_driver_api *sys1_api;

    /* Pointer to array of extended PPU power domain driver APIs */
    const struct mod_pd_driver_api *ext_ppu_apis;

    /* Power domain module restricted API pointer */
    const struct mod_pd_restricted_api *mod_pd_restricted_api;

    /* Power domain module driver input API pointer */
    const struct mod_pd_driver_input_api *mod_pd_driver_input_api;

    /* Driver API pointer */
    const struct mod_system_power_driver_api *driver_api;

    /* Power domain module identifier of the system power domain */
    fwk_id_t mod_pd_system_id;

    /* Current system-level power state */
    unsigned int state;

    /* Pointer to module config */
    const struct mod_system_power_config *config;
};

struct system_power_isr {
    unsigned int interrupt;
    void (*handler)(void);
};

static struct system_power_ctx system_power_ctx;

/*
 * Functions fulfilling the Power Domain module's driver API
 */

static int system_power_set_state(fwk_id_t pd_id, unsigned int state)
{
    return FWK_SUCCESS;
}

static int system_power_get_state(fwk_id_t pd_id, unsigned int *state)
{
    *state = system_power_ctx.state;

    return FWK_SUCCESS;
}

static int system_power_reset(fwk_id_t pd_id)
{
    return FWK_E_SUPPORT;
}

static int system_power_shutdown(
    fwk_id_t pd_id,
    enum mod_pd_system_shutdown system_shutdown)
{
    int status;

    status = system_power_set_state(pd_id, MOD_PD_STATE_OFF);
    if (status != FWK_SUCCESS)
        return status;

    return system_power_ctx.driver_api->system_shutdown(system_shutdown);
}

static void soc_wakeup_handler(void)
{
    int status;
    fwk_id_t pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, 0);
    uint32_t state = MOD_PD_COMPOSITE_STATE(
        MOD_PD_LEVEL_2, 0, MOD_PD_STATE_ON, MOD_PD_STATE_ON, MOD_PD_STATE_ON);

    status =
        system_power_ctx.mod_pd_restricted_api->set_state(pd_id, false, state);
    assert(status == FWK_SUCCESS);
    (void)status;
}

static const struct mod_pd_driver_api system_power_power_domain_driver_api = {
    .set_state = system_power_set_state,
    .get_state = system_power_get_state,
    .reset = system_power_reset,
    .shutdown = system_power_shutdown
};

/*
 * Functions fulfilling the Power Domain module's driver input API
 */

static int system_power_report_power_state_transition(
    fwk_id_t module_id,
    unsigned int state)
{
    int status;
    unsigned int sys0_state, sys1_state;

    system_power_ctx.sys0_api->get_state(
        system_power_ctx.config->ppu_sys0_id, &sys0_state);
    system_power_ctx.sys1_api->get_state(
        system_power_ctx.config->ppu_sys1_id, &sys1_state);

    if ((sys0_state == MOD_PD_STATE_ON) && (sys1_state == MOD_PD_STATE_ON))
        system_power_ctx.state = MOD_PD_STATE_ON;
    else if (
        (sys0_state == MOD_PD_STATE_OFF) && (sys1_state == MOD_PD_STATE_ON))
        system_power_ctx.state = MOD_SYSTEM_POWER_POWER_STATE_SLEEP0;
    else
        system_power_ctx.state = MOD_PD_STATE_OFF;

    status =
        system_power_ctx.mod_pd_driver_input_api->report_power_state_transition(
            system_power_ctx.mod_pd_system_id, system_power_ctx.state);
    assert(status == FWK_SUCCESS);
    (void)status;

    return FWK_SUCCESS;
}

static const struct mod_pd_driver_input_api
    system_power_power_domain_driver_input_api = {
        .report_power_state_transition =
            system_power_report_power_state_transition
    };

/*
 * Functions fulfilling the framework's module interface
 */

static int system_power_mod_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *data)
{
    assert(data != NULL);

    system_power_ctx.config = data;
    system_power_ctx.mod_pd_system_id = FWK_ID_NONE;

    if (system_power_ctx.config->ext_ppus_count > 0) {
        system_power_ctx.ext_ppu_apis = fwk_mm_calloc(
            system_power_ctx.config->ext_ppus_count,
            sizeof(system_power_ctx.ext_ppu_apis[0]));
        if (system_power_ctx.ext_ppu_apis == NULL)
            return FWK_E_NOMEM;
    }

    if (system_power_ctx.config->soc_wakeup_irq != FWK_INTERRUPT_NONE) {
        return fwk_interrupt_set_isr(
            system_power_ctx.config->soc_wakeup_irq, soc_wakeup_handler);
    } else
        return FWK_SUCCESS;
}

static int system_power_bind(fwk_id_t id, unsigned int round)
{
    int status;

    status = fwk_module_bind(
        system_power_ctx.config->driver_id,
        system_power_ctx.config->driver_api_id,
        &system_power_ctx.driver_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(
        fwk_module_id_power_domain,
        mod_pd_api_id_restricted,
        &system_power_ctx.mod_pd_restricted_api);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int system_power_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t pd_id,
    fwk_id_t api_id,
    const void **api)
{
    if (fwk_id_is_equal(api_id, mod_system_power_api_id_pd_driver)) {
        if (!fwk_id_is_equal(
                fwk_id_build_module_id(requester_id),
                fwk_module_id_power_domain))
            return FWK_E_ACCESS;

        *api = &system_power_power_domain_driver_api;
        system_power_ctx.mod_pd_system_id = requester_id;
    } else {
        if (!fwk_id_is_equal(
                requester_id, system_power_ctx.config->ppu_sys0_id) &&
            !fwk_id_is_equal(
                requester_id, system_power_ctx.config->ppu_sys1_id))
            return FWK_E_ACCESS;
        *api = &system_power_power_domain_driver_input_api;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_rcar_system_power = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_SYSTEM_POWER_API_COUNT,
    .init = system_power_mod_init,
    .bind = system_power_bind,
    .process_bind_request = system_power_process_bind_request,
};
