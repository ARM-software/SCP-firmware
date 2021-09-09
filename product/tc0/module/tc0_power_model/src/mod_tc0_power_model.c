/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     TC0 Power Model.
 */

#include <mod_tc0_power_model.h>
#include <mod_thermal_mgmt.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

uint32_t tc0_pm_level_to_power(fwk_id_t pm_id, const uint32_t level)
{
    const struct mod_tc0_power_model_dev_config *config;

    config = fwk_module_get_data(pm_id);
    return (config->coeff * level);
}

uint32_t tc0_pm_power_to_level(fwk_id_t pm_id, const uint32_t power)
{
    const struct mod_tc0_power_model_dev_config *config;

    config = fwk_module_get_data(pm_id);
    return (power / config->coeff);
}

static const struct mod_thermal_mgmt_driver_api tc0_thermal_mgmt_driver_api = {
    .level_to_power = tc0_pm_level_to_power,
    .power_to_level = tc0_pm_power_to_level,
};

/*
 * Framework handlers.
 */

static int tc0_power_model_mod_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *data)
{
    return FWK_SUCCESS;
}

static int tc0_power_model_elem_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    return FWK_SUCCESS;
}

static int tc0_power_model_bind(fwk_id_t id, unsigned int round)
{
    return FWK_SUCCESS;
}

static int tc0_power_model_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t pd_id,
    fwk_id_t api_id,
    const void **api)
{
    fwk_id_t power_model_api_id = FWK_ID_API(
        FWK_MODULE_IDX_TC0_POWER_MODEL,
        MOD_TC0_POWER_MODEL_THERMAL_DRIVER_API_IDX);

    if (fwk_id_is_equal(api_id, power_model_api_id)) {
        *api = &tc0_thermal_mgmt_driver_api;

        return FWK_SUCCESS;
    }

    return FWK_E_ACCESS;
}

const struct fwk_module module_tc0_power_model = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = tc0_power_model_mod_init,
    .element_init = tc0_power_model_elem_init,
    .bind = tc0_power_model_bind,
    .process_bind_request = tc0_power_model_process_bind_request,
};
