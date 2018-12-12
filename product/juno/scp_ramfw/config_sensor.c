/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_module.h>
#include <mod_juno_adc.h>
#include <juno_id.h>

static const struct fwk_element sensor_element_table[] = {
    /*
     * ADC Sensors
     */
    [0] = {
        .name = "Juno amps-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [1] = {
        .name = "Juno amps-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [2] = {
        .name = "Juno amps-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [3] = {
        .name = "Juno amps-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [4] = {
        .name = "Juno volt-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [5] = {
        .name = "Juno volt-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [6] = {
        .name = "Juno volt-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [7] = {
        .name = "Juno volt-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [8] = {
        .name = "Juno watt-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [9] = {
        .name = "Juno watt-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [10] = {
        .name = "Juno watt-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [11] = {
        .name = "Juno watt-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [12] = {
        .name = "Juno joule-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [13] = {
        .name = "Juno joule-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [14] = {
        .name = "Juno joule-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [15] = {
        .name = "Juno joule-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },

    [16] = { 0 } /* Termination description */
};

/*
 * When running on a model at least one fake sensor is required to register in
 * order to properly initialize scmi sensor management.
 */
static const struct fwk_element sensor_element_table_fvp[] = {
    [0] = {
        .name = "Juno fake reg sensor",
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
