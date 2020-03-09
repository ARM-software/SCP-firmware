/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_sensor.h"
#include "juno_id.h"

#include <mod_juno_adc.h>
#include <mod_juno_pvt.h>
#include <mod_juno_xrp7724.h>
#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <string.h>

static const struct fwk_element sensor_element_table_r0[] = {
    /*
     * PMIC Sensor
     */
    [MOD_JUNO_SENSOR_XRP7724_PMIC_TEMP_IDX] = {
        .name = "PMIC-temp",
        .data = &(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_XRP7724, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_API_IDX_SENSOR),

        },
    },

    /*
     * PVT Sensors
     */
    [MOD_JUNO_PVT_SENSOR_VOLT_BIG] = {
        .name = "pvt-V-big",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_BIG,
                                                 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [MOD_JUNO_PVT_SENSOR_VOLT_LITTLE] = {
        .name = "pvt-V-little",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_LITTLE,
                                                 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [MOD_JUNO_PVT_SENSOR_TEMP_SOC] = {
        .name = "pvt-T-soc",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_SOC,
                                                 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [MOD_JUNO_PVT_SENSOR_VOLT_SYS] = {
        .name = "pvt-V-sys",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_STDCELL,
                                                 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },

    /*
     * ADC Sensors
     */
    [MOD_JUNO_SENSOR_VOLT_SYS_IDX] = {
        .name = "adc-V-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_BIG_IDX] = {
        .name = "adc-V-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_LITTLE_IDX] = {
        .name = "adc-V-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_GPU_IDX] = {
        .name = "adc-V-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },

    #if USE_FULL_SET_SENSORS
    [MOD_JUNO_SENSOR_AMPS_SYS_IDX] = {
        .name = "adc-I-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_AMPS_BIG_IDX] = {
        .name = "adc-I-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_AMPS_LITTLE_IDX] = {
        .name = "adc-I-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_AMPS_GPU_IDX] = {
        .name = "adc-I-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_SYS_IDX] = {
        .name = "adc-W-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_BIG_IDX] = {
        .name = "adc-W-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_LITTLE_IDX] = {
        .name = "adc-W-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_GPU_IDX] = {
        .name = "adc-W-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_SYS_IDX] = {
        .name = "adc-J-sys",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_BIG_IDX] = {
        .name = "adc-J-big",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_LITTLE_IDX] = {
        .name = "adc-J-little",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_GPU_IDX] = {
        .name = "adc-J-gpu",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    #endif

    /* The termination description is added at runtime */
};

#if USE_FULL_SET_SENSORS
/* The following table lists PVT sensors available on juno R1 & R2 */
static const struct fwk_element pvt_sensors_juno_r1_r2_elem_table[] = {
    [0] = {
        .name = "pvt-V-big-2",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_BIG,
                                                 1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [1] = {
        .name = "pvt-V-little-2",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_LITTLE,
                                                 1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [2] = {
        .name = "pvt-V-gpu",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_GPU,
                                                 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [3] = {
        .name = "pvt-V-gpu-2",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_GPU,
                                                 1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    /* The termination description is added at runtime */
};
#endif

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

    #if USE_FULL_SET_SENSORS
    enum juno_idx_revision rev;
    #endif

    size_t sensor_elem_table_size;
    struct fwk_element *element_table;

    status = juno_id_get_platform(&platform_id);
    if (!fwk_expect(status == FWK_SUCCESS))
        return NULL;

    if (platform_id == JUNO_IDX_PLATFORM_FVP)
        return sensor_element_table_fvp;
    else {
        sensor_elem_table_size = FWK_ARRAY_SIZE(sensor_element_table_r0);

        #if USE_FULL_SET_SENSORS
        status = juno_id_get_revision(&rev);
        if (status != FWK_SUCCESS)
            return NULL;

        if (rev == JUNO_IDX_REVISION_R0) {
            /* Just add the termination description */
            element_table = fwk_mm_calloc(
                (sensor_elem_table_size + 1),
                sizeof(struct fwk_element));

            memcpy(element_table,
                   sensor_element_table_r0,
                   sizeof(sensor_element_table_r0));
        } else {
            /*
             * Add additional sensors available on Juno R1 & R2 and the
             * termination description.
             */
            element_table = fwk_mm_calloc(
                (sensor_elem_table_size + pvt_sensor_elem_table_size + 1),
                sizeof(struct fwk_element));

            memcpy(element_table,
                   sensor_element_table_r0,
                   sizeof(sensor_element_table_r0));

            memcpy(element_table + sensor_elem_table_size,
                   pvt_sensors_juno_r1_r2_elem_table,
                   sizeof(pvt_sensors_juno_r1_r2_elem_table));
        }
        #else
        /* Just add the termination description */
        element_table = fwk_mm_calloc(
            (sensor_elem_table_size + 1),
            sizeof(struct fwk_element));
        if (element_table == NULL)
            return NULL;

        memcpy(element_table,
               sensor_element_table_r0,
               sizeof(sensor_element_table_r0));
        #endif

        return element_table;
    }
}

struct fwk_module_config config_sensor = {
    .get_element_table = get_sensor_element_table,
    .data = NULL,
};
