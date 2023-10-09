/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Platform System Support
 */

#ifndef MOD_SENSOR_SMCF_DRV_H
#define MOD_SENSOR_SMCF_DRV_H

#include <mod_smcf.h>

#include <fwk_id.h>

#include <stddef.h>

#define SENSOR_SMCF_DATA_SIZE_IN_BYTES 8
#define SENSOR_SMCF_DATA_SIZE_IN_WORDS 2
#define SENSOR_SMCF_TAG_BUFFER_SIZE    16

/*!
 * \brief sensor_smcf_drv element config.
 * \details The configuration data for each element which
 *      corresponds to a single sensor MLI within a device.
 */

struct sensor_smcf_drv_element_config {
    /*! SMCF MLI id for the element */
    fwk_id_t smcf_mli_id;

    /*! Per Monitor maximum sample data size in Bytes */
    uint32_t max_samples_size;
};

/*!
 * \brief Indexes of the interfaces exposed by the module.
 */
enum mod_sensor_smcf_drv_api_idx {
    /*! Sensor data api for data as a single value (mod_sensor_value_t) */
    MOD_SENSOR_SMCF_DRV_API_IDX_GET_VALUE,

    /*! Sensor data api for data in a buffer with given number of data values */
    MOD_SENSOR_SMCF_DRV_API_IDX_GET_MULTIPLE_SAMPLES,

    /*! Number of exposed interfaces */
    MOD_SENSOR_SMCF_DRV_API_IDX_COUNT,
};

/*!
 * \brief Sensor SMCF driver Multiple samples API.
 */
struct mod_sensor_smcf_drv_multiple_samples_api {
    /*!
     * \brief Get requested number of samples for a sensor MGI
     *
     * \param sensor_id Specific sensor mli id.
     * \param[out] sample_buff pointer to unsigned int(32bit) buffer
     *     to store sample data.
     * \param buffer_size size of the requested sample(s) in bytes.

     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM Invalid parameters provided in api call.
     * \retval ::FWK_E_RANGE Sample size requested is more than the available
     *     samples.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*get_samples)(
        fwk_id_t sensor_id,
        uint32_t *sample_buff,
        size_t buffer_size);
};

#endif /* MOD_SENSOR_SMCF_DRV_H */
