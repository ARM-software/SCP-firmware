/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_sensor.h"
#include "juno_id.h"

#if (PLATFORM_VARIANT == JUNO_VARIANT_BOARD)
#    include <mod_juno_adc.h>
#    include <mod_juno_pvt.h>
#    include <mod_juno_xrp7724.h>
#endif

#include <mod_scmi_sensor.h>
#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <string.h>

#if (PLATFORM_VARIANT == JUNO_VARIANT_BOARD)
static const struct fwk_element sensor_element_table_r0[] = {
    /*
     * PMIC Sensor
     */
    [MOD_JUNO_SENSOR_XRP7724_PMIC_TEMP_IDX] = {
        .name = "TEMP_PMIC",
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
        .name = "VOLT_BIG",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_BIG,
                                                 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [MOD_JUNO_PVT_SENSOR_VOLT_LITTLE] = {
        .name = "VOLT_LITTLE",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_LITTLE,
                                                 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [MOD_JUNO_PVT_SENSOR_TEMP_SOC] = {
        .name = "TEMP_SOC",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_SOC,
                                                 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [MOD_JUNO_PVT_SENSOR_VOLT_SYS] = {
        .name = "VOLT_SYS",
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
        .name = "BRD_VOLT_SYS",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_BIG_IDX] = {
        .name = "BRD_VOLT_BIG",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_LITTLE_IDX] = {
        .name = "BRD_VOLT_LITTLE",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_VOLT_GPU_IDX] = {
        .name = "BRD_VOLT_GPU",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_VOLT,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },

#    if USE_FULL_SET_SENSORS
    [MOD_JUNO_SENSOR_AMPS_SYS_IDX] = {
        .name = "BRD_CURR_SYS",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_AMPS_BIG_IDX] = {
        .name = "BRD_CURR_BIG",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_AMPS_LITTLE_IDX] = {
        .name = "BRD_CURR_LITTLE",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_AMPS_GPU_IDX] = {
        .name = "BRD_CURR_GPU",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_CURRENT,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_SYS_IDX] = {
        .name = "BRD_PWR_SYS",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_BIG_IDX] = {
        .name = "BRD_PWR_BIG",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_LITTLE_IDX] = {
        .name = "BRD_PWR_LITTLE",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_WATT_GPU_IDX] = {
        .name = "BRD_PWR_GPU",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_POWER,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_SYS_IDX] = {
        .name = "BRD_ENRG_SYS",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_SYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_BIG_IDX] = {
        .name = "BRD_ENRG_BIG",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_LITTLE_IDX] = {
        .name = "BRD_ENRG_LITTLE",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
    [MOD_JUNO_SENSOR_JOULE_GPU_IDX] = {
        .name = "BRD_ENRG_GPU",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                                 ADC_TYPE_ENERGY,
                                                 ADC_DEV_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                                             MOD_JUNO_ADC_API_IDX_DRIVER),
        }),
    },
#    endif

    /* The termination description is added at runtime */
};

#    if USE_FULL_SET_SENSORS
/* The following table lists PVT sensors available on juno R1 & R2 */
static const struct fwk_element pvt_sensors_juno_r1_r2_elem_table[] = {
    [0] = {
        .name = "TEMP_BIG",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_BIG,
                                                 1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [1] = {
        .name = "TEMP_LITTLE",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_LITTLE,
                                                 1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [2] = {
        .name = "TEMP_GPU_0",
        .data = (&(struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PVT,
                                                 JUNO_PVT_GROUP_GPU,
                                                 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PVT, 0),
        }),
    },
    [3] = {
        .name = "TEMP_GPU_1",
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

#elif (PLATFORM_VARIANT == JUNO_VARIANT_FVP)

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

#endif

static const struct fwk_element *get_sensor_element_table(fwk_id_t module_id)
{
    #if USE_FULL_SET_SENSORS
    enum juno_idx_revision rev;
    size_t pvt_sensor_elem_table_size;
    #endif

#if (PLATFORM_VARIANT == JUNO_VARIANT_FVP)
    return sensor_element_table_fvp;
#elif (PLATFORM_VARIANT == JUNO_VARIANT_BOARD)
    size_t sensor_elem_table_size;
    struct fwk_element *element_table;
    sensor_elem_table_size = FWK_ARRAY_SIZE(sensor_element_table_r0);

#    if USE_FULL_SET_SENSORS
    status = juno_id_get_revision(&rev);
    if (status != FWK_SUCCESS)
        return NULL;

    if (rev == JUNO_IDX_REVISION_R0) {
        /* Just add the termination description */
        element_table = fwk_mm_calloc(
            (sensor_elem_table_size + 1), sizeof(struct fwk_element));

        memcpy(
            element_table,
            sensor_element_table_r0,
            sizeof(sensor_element_table_r0));
    } else {
        pvt_sensor_elem_table_size =
            FWK_ARRAY_SIZE(pvt_sensors_juno_r1_r2_elem_table);

        /*
         * Add additional sensors available on Juno R1 & R2 and the
         * termination description.
         */
        element_table = fwk_mm_calloc(
            (sensor_elem_table_size + pvt_sensor_elem_table_size + 1),
            sizeof(struct fwk_element));

        memcpy(
            element_table,
            sensor_element_table_r0,
            sizeof(sensor_element_table_r0));

        memcpy(
            element_table + sensor_elem_table_size,
            pvt_sensors_juno_r1_r2_elem_table,
            sizeof(pvt_sensors_juno_r1_r2_elem_table));
    }
#    else
    /* Just add the termination description */
    element_table =
        fwk_mm_calloc((sensor_elem_table_size + 1), sizeof(struct fwk_element));
    if (element_table == NULL) {
        return NULL;
    }

    fwk_str_memcpy(
        element_table,
        sensor_element_table_r0,
        sizeof(sensor_element_table_r0));
#    endif

    return element_table;
#endif
}

struct fwk_module_config config_sensor = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_sensor_element_table),
};
