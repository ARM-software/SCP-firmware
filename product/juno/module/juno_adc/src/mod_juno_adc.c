/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno ADC Driver
 */

#include "juno_adc.h"
#include "v2m_sys_regs.h"

#include <mod_juno_adc.h>
#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

/*
 * ADC driver API functions.
 */
static int get_value(fwk_id_t id, uint64_t *value)
{
    uint32_t adc_value;
    uint64_t adc_quantity;
    enum juno_adc_dev_type dev_type;

    dev_type = fwk_id_get_sub_element_idx(id);

    switch (fwk_id_get_element_idx(id)) {
    case ADC_TYPE_CURRENT:
        adc_value = V2M_SYS_REGS->ADC_CURRENT[dev_type] &
                    JUNO_ADC_SYS_REG_AMPS_MASK;

        adc_quantity = ((uint64_t)adc_value) * JUNO_ADC_AMPS_MULTIPLIER;

        if ((dev_type == ADC_DEV_BIG) || (dev_type == ADC_DEV_GPU))
            adc_quantity /= ADC_CURRENT_CONST1;
        else
            adc_quantity /= ADC_CURRENT_CONST2;

        *value = adc_quantity;

        return FWK_SUCCESS;

    case ADC_TYPE_VOLT:
        adc_value = V2M_SYS_REGS->ADC_VOLT[dev_type] &
                    JUNO_ADC_SYS_REG_VOLT_MASK;

        adc_quantity = (((uint64_t)adc_value) * JUNO_ADC_VOLT_MULTIPLIER)
                          / ADC_VOLT_CONST;
        *value = adc_quantity;

        return FWK_SUCCESS;

    case ADC_TYPE_POWER:
        adc_value = V2M_SYS_REGS->ADC_POWER[dev_type] &
                    JUNO_ADC_SYS_REG_POWER_MASK;

        adc_quantity = ((uint64_t)adc_value) * JUNO_ADC_WATTS_MULTIPLIER;

        if ((dev_type == ADC_DEV_BIG) || (dev_type == ADC_DEV_GPU))
            adc_quantity /= ADC_POWER_CONST1;
        else
            adc_quantity /= ADC_POWER_CONST2;

        *value = adc_quantity;

        return FWK_SUCCESS;

    case ADC_TYPE_ENERGY:
        adc_quantity = V2M_SYS_REGS->ADC_ENERGY[dev_type] *
            JUNO_ADC_JOULE_MULTIPLIER;

        if ((dev_type == ADC_DEV_BIG) || (dev_type == ADC_DEV_GPU))
            adc_quantity /= ADC_ENERGY_CONST1;
        else
            adc_quantity /= ADC_ENERGY_CONST2;

        *value = adc_quantity;

        return FWK_SUCCESS;

    default:
        return FWK_E_PARAM;
    }
}

static int get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    const struct mod_juno_adc_dev_config *config;

    config = fwk_module_get_data(id);

    if (!fwk_expect(config->info != NULL))
        return FWK_E_DATA;

    *info = *(config->info);

    return FWK_SUCCESS;
}

static const struct mod_sensor_driver_api adc_sensor_api = {
    .get_value = get_value,
    .get_info = get_info,
};

/*
 * Framework handlers.
 */
static int juno_adc_init(fwk_id_t module_id,
                         unsigned int element_count,
                         const void *data)
{
    if (!fwk_expect(element_count > 0))
        return FWK_E_DATA;

    return FWK_SUCCESS;
}

static int juno_adc_element_init(fwk_id_t element_id,
                                 unsigned int sub_element_count,
                                 const void *data)
{
    return FWK_SUCCESS;
}

static int juno_adc_process_bind_request(fwk_id_t source_id,
                                         fwk_id_t target_id,
                                         fwk_id_t api_id,
                                         const void **api)
{
    if (!fwk_module_is_valid_sub_element_id(target_id))
        return FWK_E_ACCESS;

    if (fwk_id_get_module_idx(source_id) !=
        fwk_id_get_module_idx(fwk_module_id_sensor))
        return FWK_E_ACCESS;

    if (!fwk_id_is_equal(api_id, mod_juno_adc_api_id_driver))
        return FWK_E_ACCESS;

    *api = &adc_sensor_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_juno_adc = {
    .name = "Juno ADC Driver",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_JUNO_ADC_API_IDX_COUNT,
    .init = juno_adc_init,
    .element_init = juno_adc_element_init,
    .process_bind_request = juno_adc_process_bind_request
};
