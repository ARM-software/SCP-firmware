/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno ADC Driver
 */

#ifndef MOD_JUNO_ADC_H
#define MOD_JUNO_ADC_H

#include <mod_sensor.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \ingroup GroupJunoModule
 * \defgroup GroupADC ADC Sensor Driver
 * \{
 */

/*!
 * \brief ADC Type indices.
 */
enum juno_adc_sensor_type {
    /*! Voltage type ADC */
    ADC_TYPE_VOLT = 0,
    /*! Current type ADC */
    ADC_TYPE_CURRENT,
    /*! Power type ADC */
    ADC_TYPE_POWER,
    /*! Energy type ADC */
    ADC_TYPE_ENERGY,
    /*! Max number of ADC type */
    ADC_TYPE_COUNT
};

/*!
 * \brief ADC Device Type indices.
 */
enum juno_adc_dev_type {
    /*! ADC for the SYS device */
    ADC_DEV_SYS = 0,
    /*! ADC for the big cpu */
    ADC_DEV_BIG,
    /* ADC for the LITTLE cpu */
    ADC_DEV_LITTLE,
    /*! ADC for the GPU */
    ADC_DEV_GPU,
    /*! Number of ADC devices */
    ADC_DEV_TYPE_COUNT
};

/*!
 * \brief Element configuration.
 */
struct mod_juno_adc_dev_config {
    /*! Type of ADC */
    enum juno_adc_sensor_type type;

    /*! Type of device */
    enum juno_adc_dev_type dev_type;

    /*! Auxiliary sensor information */
    struct mod_sensor_info *info;
};

/*!
 * \brief Juno ADC API indices.
 */
enum mod_juno_adc_api_idx {
    MOD_JUNO_ADC_API_IDX_DRIVER,
    MOD_JUNO_ADC_API_IDX_COUNT,
};

/*!
 * \brief Juno ADC API identifier.
 */
static const fwk_id_t mod_juno_adc_api_id_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_ADC,
                    MOD_JUNO_ADC_API_IDX_DRIVER);

/*!
 * \}
 */

#endif /* MOD_JUNO_ADC_H */
