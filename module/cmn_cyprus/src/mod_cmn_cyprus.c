/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Driver module for CMN Cyprus interconnect.
 */

#include <internal/cmn_cyprus_ctx.h>
#include <internal/cmn_cyprus_discovery_setup.h>
#include <internal/cmn_cyprus_hnsam_setup.h>
#include <internal/cmn_cyprus_rnsam_setup.h>

#include <mod_clock.h>
#include <mod_cmn_cyprus.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>

/* Max Mesh size */
#define CMN_CYPRUS_MESH_X_MAX 12
#define CMN_CYPRUS_MESH_Y_MAX 12

/* Module context */
static struct cmn_cyprus_ctx ctx;

static int cmn_cyprus_setup(void)
{
    int status;

    FWK_LOG_INFO(MOD_NAME "Configuring CMN...");

    /* Discover the mesh and setup the context data */
    status = cmn_cyprus_discovery(&ctx);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME "Error! CMN Discovery failed with %s",
            fwk_status_str(status));
        return status;
    }

    /* Program the HN-F SAM */
    status = cmn_cyprus_setup_hnf_sam(&ctx);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME "Error! HN-F SAM setup failed with %s",
            fwk_status_str(status));
        return status;
    }

    /* Program the RNSAM */
    status = cmn_cyprus_setup_rnsam(&ctx);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            MOD_NAME "Error! RNSAM setup failed with %s",
            fwk_status_str(status));
        return status;
    }

    FWK_LOG_INFO(MOD_NAME "Done");

    return FWK_SUCCESS;
}

/* Framework handlers */
static int cmn_cyprus_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *data)
{
    fwk_assert(data != NULL);

    /* Save the module config data in the context */
    ctx.config = (const struct mod_cmn_cyprus_config *)data;

    ctx.cfgm = (struct cmn_cyprus_cfgm_reg *)ctx.config->periphbase;

    /* Validate config data */
    if (ctx.config->periphbase == 0) {
        return FWK_E_DATA;
    }

    if ((ctx.config->mesh_size_x == 0) ||
        (ctx.config->mesh_size_x > CMN_CYPRUS_MESH_X_MAX)) {
        return FWK_E_DATA;
    }

    if ((ctx.config->mesh_size_y == 0) ||
        (ctx.config->mesh_size_y > CMN_CYPRUS_MESH_Y_MAX)) {
        return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

int cmn_cyprus_start(fwk_id_t id)
{
    int status;

    if (fwk_optional_id_is_defined(ctx.config->clock_id) &&
        !fwk_id_is_equal(ctx.config->clock_id, FWK_ID_NONE)) {
        return fwk_notification_subscribe(
            mod_clock_notification_id_state_changed, ctx.config->clock_id, id);
    }

    status = cmn_cyprus_setup();
    if (status != FWK_SUCCESS) {
        fwk_trap();
    }

    return status;
}

static int cmn_cyprus_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;
    struct clock_notification_params *params;

    fwk_assert(
        fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));

    params = (struct clock_notification_params *)event->params;

    /* Setup CMN after the clock is initialized */
    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
        status = cmn_cyprus_setup();
        if (status != FWK_SUCCESS) {
            fwk_trap();
        }

        return fwk_notification_unsubscribe(
            mod_clock_notification_id_state_changed,
            ctx.config->clock_id,
            FWK_ID_MODULE(FWK_MODULE_IDX_CMN_CYPRUS));
    }

    return FWK_SUCCESS;
}

static int cmn_cyprus_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t id,
    fwk_id_t api_id,
    const void **api)
{
    enum mod_cmn_cyprus_api_idx api_idx;
    int status;

    api_idx = (enum mod_cmn_cyprus_api_idx)fwk_id_get_api_idx(api_id);

    /* Invalid parameters */
    if (!fwk_module_is_valid_module_id(id)) {
        return FWK_E_PARAM;
    }

    switch (api_idx) {
    case MOD_CMN_CYPRUS_API_IDX_MAP_IO_REGION:
        get_rnsam_memmap_api(api);
        status = FWK_SUCCESS;
        break;

    default:
        status = FWK_E_PARAM;
        break;
    };

    return status;
}

const struct fwk_module module_cmn_cyprus = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_CMN_CYPRUS_API_COUNT,
    .init = cmn_cyprus_init,
    .start = cmn_cyprus_start,
    .process_notification = cmn_cyprus_process_notification,
    .process_bind_request = cmn_cyprus_process_bind_request,
};
