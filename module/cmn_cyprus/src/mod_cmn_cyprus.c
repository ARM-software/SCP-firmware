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

#include <mod_cmn_cyprus.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
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

    status = cmn_cyprus_setup();
    if (status != FWK_SUCCESS) {
        fwk_trap();
    }

    return status;
}

const struct fwk_module module_cmn_cyprus = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = cmn_cyprus_init,
    .start = cmn_cyprus_start,
};
