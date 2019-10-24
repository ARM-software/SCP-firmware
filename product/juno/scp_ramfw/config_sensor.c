/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_juno_adc.h>
#include <mod_juno_xrp7724.h>
#include <config_sensor.h>
#include <juno_id.h>

static const struct fwk_element sensor_element_table[] = {
    /*
     * ADC Sensors
     */
    [MOD_JUNO_SENSOR_AMPS_SYS_IDX] = {
        .name = "Amps-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_AMPS_BIG_IDX] = {
        .name = "Amps-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_AMPS_LITTLE_IDX] = {
        .name = "Amps-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_AMPS_GPU_IDX] = {
        .name = "Amps-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_SYS_IDX] = {
        .name = "Volt-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_BIG_IDX] = {
        .name = "Volt-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_LITTLE_IDX] = {
        .name = "Volt-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_GPU_IDX] = {
        .name = "Volt-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_SYS_IDX] = {
        .name = "Watt-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_BIG_IDX] = {
        .name = "Watt-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_LITTLE_IDX] = {
        .name = "Watt-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_GPU_IDX] = {
        .name = "Watt-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_SYS_IDX] = {
        .name = "Joule-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_BIG_IDX] = {
        .name = "Joule-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_LITTLE_IDX] = {
        .name = "Joule-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_GPU_IDX] = {
        .name = "Joule-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_XRP7724_PMIC_TEMP_IDX] = {
        .name = "PMIC-Temp",
        .data = &(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_XRP7724, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_API_IDX_SENSOR),

        },
    },

    [MOD_JUNO_SENSOR_IDX_COUNT] = { 0 } /* Termination description */
};

/*
 * When running on a model at least one fake sensor is required to register in
 * order to properly initialize scmi sensor management.
 */
static const struct fwk_element sensor_element_table_fvp[] = {
    [0] = {
        .name = "Fake sensor",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
        }),
    },

    [1] = { 0 } /* Termination description */
};

static const struct fwk_element *get_sensor_element_table(fwk_id_t module_id)
{
    int status;
    enum juno_idx_platform platform_id = JUNO_IDX_PLATFORM_COUNT;

    status = juno_id_get_platform(&platform_id);
    if (!fwk_expect(status == FWK_SUCCESS))
        return NULL;

    if (platform_id == JUNO_IDX_PLATFORM_FVP)
        return sensor_element_table_fvp;
    else
        return sensor_element_table;
}

struct fwk_module_config config_sensor = {
    .get_element_table = get_sensor_element_table,
    .data = NULL,
};
