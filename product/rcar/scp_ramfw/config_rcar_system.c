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
#include <config_rcar_pd_pmic_bd9571.h>

#include <mod_clock.h>
#include <mod_rcar_clock.h>
#include <mod_rcar_system.h>
#include <mod_reg_sensor.h>
#include <mod_rcar_scif.h>
#include <mod_rcar_pd_sysc.h>
#include <mod_rcar_pd_pmic_bd9571.h>

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
                MOD_RCAR_CLOCK_API_TYPE_SYSTEM),
        }),
    },
    {
        .name = "mstp_clock",
        .data = &((struct mod_rcar_system_dev_config){
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MSTP_CLOCK, 0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_RCAR_MSTP_CLOCK,
                MOD_RCAR_CLOCK_API_TYPE_SYSTEM),
        }),
    },
    {
        .name = "scif",
        .data = &((struct mod_rcar_system_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_SCIF, 0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_RCAR_SCIF,
                MOD_RCAR_SCIF_API_TYPE_SYSTEM),
        }),
    },
    {
        .name = "sensor",
        .data = &((struct mod_rcar_system_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_REG_SENSOR,
                MOD_RCAR_REG_SENSOR_API_TYPE_SYSTEM),
        }),
    },
    {
        .name = "pmic",
        .data = &((struct mod_rcar_system_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PD_PMIC_BD9571,
                RCAR_PD_PMIC_ELEMENT_IDX_PMIC_DDR_BKUP),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_RCAR_PD_PMIC_BD9571,
                MOD_RCAR_PD_PMIC_API_TYPE_SYSTEM),
        }),
    },
    {
        .name = "pd_sysc",
        .data = &((struct mod_rcar_system_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_SYSTEM),
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
