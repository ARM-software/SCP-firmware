/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_ddr.h"

#include <mod_synquacer_memc.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

const struct mod_f_i2c_api *f_i2c_api;
static int synquacer_memc_config(void);

/* Framework API */
static int mod_synquacer_memc_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *config)
{
    return FWK_SUCCESS;
}

static int mod_synquacer_memc_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    return FWK_SUCCESS;
}

static int mod_synquacer_memc_bind(fwk_id_t id, unsigned int round)
{
    int status;

    /* Nothing to do in the second round of calls. */
    if (round == 1)
        return FWK_SUCCESS;

    /* Nothing to do in case of elements. */
    if (fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_F_I2C),
        FWK_ID_API(FWK_MODULE_IDX_F_I2C, 0),
        &f_i2c_api);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int mod_synquacer_memc_start(fwk_id_t id)
{
    synquacer_memc_config();
    return FWK_SUCCESS;
}

static int synquacer_memc_config(void)
{
    fw_ddr_init();

    FWK_LOG_INFO("[SYNQUACER MEMC] DMC init done.");

    return FWK_SUCCESS;
}

const struct fwk_module module_synquacer_memc = {
    .name = "synquacer_memc",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_synquacer_memc_init,
    .element_init = mod_synquacer_memc_element_init,
    .bind = mod_synquacer_memc_bind,
    .start = mod_synquacer_memc_start,
    .api_count = 0,
    .event_count = 0,
};
