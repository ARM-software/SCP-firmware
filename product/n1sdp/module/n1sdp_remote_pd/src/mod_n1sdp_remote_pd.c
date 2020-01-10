/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      N1SDP Remote Power Domain (PD) management driver.
 */

#include "n1sdp_core.h"

#include <mod_n1sdp_c2c_i2c.h>
#include <mod_n1sdp_remote_pd.h>
#include <mod_power_domain.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

/* N1SDP remote PD driver device context */
struct n1sdp_remote_pd_device_ctx {
    /* Pointer to the device configuration */
    const struct mod_n1sdp_remote_pd_config *config;

    /* Identifier of the entity bound to the power domain driver API */
    fwk_id_t bound_id;

    /* Power module driver input API */
    struct mod_pd_driver_input_api *pd_driver_input_api;
};

/* N1SDP remote PD driver module context */
struct n1sdp_remote_pd_ctx {
    /* Table of device contexts */
    struct n1sdp_remote_pd_device_ctx *dev_ctx_table;

    /* Total power domains */
    unsigned int pd_count;

    /* C2C power domain API */
    struct n1sdp_c2c_pd_api *c2c_pd_api;

    /* Logical SYSTOP PD state */
    unsigned int logical_systop_state;
};

static struct n1sdp_remote_pd_ctx remote_pd_ctx;

/*
 * Remote PD API
 */
static int remote_pd_get_state(fwk_id_t pd_id, unsigned int *state)
{
    unsigned int element_id;

    element_id = fwk_id_get_element_idx(pd_id);
    fwk_assert(element_id < remote_pd_ctx.pd_count);

    /*
     * The last element is logical systop so report current state from
     * context variable.
     */
    if (element_id == (remote_pd_ctx.pd_count - 1)) {
        *state = remote_pd_ctx.logical_systop_state;
        return FWK_SUCCESS;
    }

    return remote_pd_ctx.c2c_pd_api->get_state(
        N1SDP_C2C_CMD_POWER_DOMAIN_GET_STATE, (uint8_t)element_id,
        state);
}

static int remote_pd_set_state(fwk_id_t pd_id, unsigned int state)
{
    int status;
    unsigned int element_id;
    struct n1sdp_remote_pd_device_ctx *dev_ctx;

    element_id = fwk_id_get_element_idx(pd_id);
    fwk_assert(element_id < remote_pd_ctx.pd_count);

    dev_ctx = &remote_pd_ctx.dev_ctx_table[element_id];

    /*
     * The last element is logical systop so no action is required
     * except reporting the state transition.
     */
    if (element_id == (remote_pd_ctx.pd_count - 1)) {
        remote_pd_ctx.logical_systop_state = state;
        return dev_ctx->pd_driver_input_api->report_power_state_transition(
            dev_ctx->bound_id, state);
    }

    switch (state) {
    case MOD_PD_STATE_OFF:
        status = remote_pd_ctx.c2c_pd_api->set_state(
            N1SDP_C2C_CMD_POWER_DOMAIN_OFF, (uint8_t)element_id,
            (uint8_t)dev_ctx->config->pd_type);
        if (status != FWK_SUCCESS)
            return status;

        status = dev_ctx->pd_driver_input_api->report_power_state_transition(
            dev_ctx->bound_id, MOD_PD_STATE_OFF);
        fwk_assert(status == FWK_SUCCESS);
        break;

    case MOD_PD_STATE_ON:
        status = remote_pd_ctx.c2c_pd_api->set_state(
            N1SDP_C2C_CMD_POWER_DOMAIN_ON, (uint8_t)element_id,
            (uint8_t)dev_ctx->config->pd_type);
        if (status != FWK_SUCCESS)
            return status;

        status = dev_ctx->pd_driver_input_api->report_power_state_transition(
            dev_ctx->bound_id, MOD_PD_STATE_ON);
        fwk_assert(status == FWK_SUCCESS);
        break;

    default:
        FWK_LOG_ERR(
            "[C2C] Requested CPU power state (%i) is not supported!", state);
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int remote_pd_reset(fwk_id_t pd_id)
{
    int status;

    status = remote_pd_set_state(pd_id, MOD_PD_STATE_OFF);
    if (status == FWK_SUCCESS)
        status = remote_pd_set_state(pd_id, MOD_PD_STATE_ON);

    return status;
}

static int remote_pd_prepare_for_system_suspend(fwk_id_t pd_id)
{
    return remote_pd_set_state(pd_id, MOD_PD_STATE_OFF);
}

static int remote_pd_shutdown(fwk_id_t pd_id,
                              enum mod_pd_system_shutdown system_shutdown)
{
    return FWK_SUCCESS;
}

static const struct mod_pd_driver_api remote_pd_driver = {
    .set_state = remote_pd_set_state,
    .get_state = remote_pd_get_state,
    .reset = remote_pd_reset,
    .prepare_core_for_system_suspend =
        remote_pd_prepare_for_system_suspend,
    .shutdown = remote_pd_shutdown,
};

/*
 * Framework handlers
 */

static int remote_pd_init(fwk_id_t module_id, unsigned int device_count,
                          const void *unused)
{
    if (!n1sdp_is_multichip_enabled() || (n1sdp_get_chipid() != 0x0))
        return FWK_SUCCESS;

    if (device_count == 0)
        return FWK_E_PARAM;

    remote_pd_ctx.dev_ctx_table = fwk_mm_calloc(device_count,
        sizeof(remote_pd_ctx.dev_ctx_table[0]));

    remote_pd_ctx.pd_count = device_count;

    return FWK_SUCCESS;
}

static int remote_pd_element_init(fwk_id_t device_id, unsigned int slot_count,
                                  const void *data)
{
    struct mod_n1sdp_remote_pd_config *config =
        (struct mod_n1sdp_remote_pd_config *)data;
    struct n1sdp_remote_pd_device_ctx *dev_ctx;

    if (!n1sdp_is_multichip_enabled() || (n1sdp_get_chipid() != 0x0))
        return FWK_SUCCESS;

    dev_ctx = &remote_pd_ctx.dev_ctx_table[fwk_id_get_element_idx(device_id)];
    dev_ctx->config = config;

    return FWK_SUCCESS;
}

static int remote_pd_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct n1sdp_remote_pd_device_ctx *dev_ctx;

    if (!n1sdp_is_multichip_enabled() || (n1sdp_get_chipid() != 0x0))
        return FWK_SUCCESS;

    if ((round == 0) && fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_C2C),
                                 FWK_ID_API(FWK_MODULE_IDX_N1SDP_C2C,
                                            N1SDP_C2C_API_IDX_PD),
                                 &remote_pd_ctx.c2c_pd_api);
        if (status != FWK_SUCCESS)
            return status;
    }

    if ((round == 1) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        dev_ctx = &remote_pd_ctx.dev_ctx_table[fwk_id_get_element_idx(id)];
        status = fwk_module_bind(dev_ctx->bound_id,
                                 mod_pd_api_id_driver_input,
                                 &dev_ctx->pd_driver_input_api);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int remote_pd_process_bind_request(fwk_id_t source_id,
                                          fwk_id_t target_id,
                                          fwk_id_t api_id,
                                          const void **api)
{
    struct n1sdp_remote_pd_device_ctx *dev_ctx;

    if (!n1sdp_is_multichip_enabled() || (n1sdp_get_chipid() != 0x0))
        return FWK_E_ACCESS;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_ACCESS;

    dev_ctx = &remote_pd_ctx.dev_ctx_table[fwk_id_get_element_idx(target_id)];

    *api = &remote_pd_driver;
    dev_ctx->bound_id = source_id;

    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_remote_pd = {
    .name = "N1SDP Remote PD",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = N1SDP_REMOTE_PD_API_COUNT,
    .init = remote_pd_init,
    .element_init = remote_pd_element_init,
    .bind = remote_pd_bind,
    .process_bind_request = remote_pd_process_bind_request,
};
