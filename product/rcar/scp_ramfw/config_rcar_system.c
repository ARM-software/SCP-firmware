/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clock_devices.h>
#include <clock_mstp_devices.h>
#include <clock_sd_devices.h>
#include <rcar_core.h>

#include <mod_clock.h>
#include <mod_rcar_clock.h>
#include <mod_rcar_system.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>

static struct fwk_element rcar_system_element_table[] = {
    {
        .name = "sd_closk",
        .data = &((struct mod_rcar_system_dev_config){
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SD_CLOCK, 0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_RCAR_SD_CLOCK,
                MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    {
        .name = "mstp_clock",
        .data = &((struct mod_rcar_system_dev_config){
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK, 0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                MOD_RCAR_CLOCK_API_TYPE_CLOCK),
        }),
    },
    { 0 }, /* Termination description. */
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return rcar_system_element_table;
}

struct fwk_module_config config_rcar_system = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
    .data = NULL,
};
