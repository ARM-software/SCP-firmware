/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <mod_sensor.h>
#include <mod_sensor_smcf_drv.h>

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#define MOD_NAME "[sensor_smcf_drv]"

/*!
 * \brief sensor_smcf_drv module context.
 */
struct mod_sensor_smcf_drv_ctx {
    /*! Number of devices */
    uint32_t num_of_devices;

    /*! Per device config table for sensor_smcf_drv */
    struct sensor_smcf_drv_element_config *element_config_table;

    /*! SMCF data buffer for sensor sample data for each device */
    struct mod_smcf_buffer sensor_smcf_data_buf;

    /*! SMCF data sampling API */
    const struct smcf_data_api *data_api;
} sensor_smcf_drv_ctx;

static int _get_smcf_sensor_data(unsigned int device_index)
{
    int status;
    struct mod_smcf_buffer smcf_tag_buf;
    static uint32_t tag_buf[SENSOR_SMCF_TAG_BUFFER_SIZE];

    /* Prepare smcf get_data arguments */
    memset(tag_buf, 0, sizeof(uint32_t) * SENSOR_SMCF_TAG_BUFFER_SIZE);
    memset(
        sensor_smcf_drv_ctx.sensor_smcf_data_buf.ptr,
        0,
        sensor_smcf_drv_ctx.sensor_smcf_data_buf.size);
    smcf_tag_buf.ptr = tag_buf;
    smcf_tag_buf.size = SENSOR_SMCF_TAG_BUFFER_SIZE;

    /* Prepare smcf get data arguments */
    status = sensor_smcf_drv_ctx.data_api->get_data(
        sensor_smcf_drv_ctx.element_config_table[device_index].smcf_mli_id,
        sensor_smcf_drv_ctx.sensor_smcf_data_buf,
        smcf_tag_buf);

    return status;
}

static int sensor_smcf_drv_get_sensor_single_value(
    fwk_id_t sensor_id,
    mod_sensor_value_t *value)
{
    int status;
    unsigned int device_idx;

    if (!fwk_module_is_valid_element_id(sensor_id) || value == NULL) {
        return FWK_E_PARAM;
    }

    device_idx = fwk_id_get_element_idx(sensor_id);
    status = _get_smcf_sensor_data(device_idx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    *value =
        *((mod_sensor_value_t *)sensor_smcf_drv_ctx.sensor_smcf_data_buf.ptr);

    return FWK_SUCCESS;
}

static int sensor_smcf_drv_get_sensor_multiple_samples(
    fwk_id_t sensor_id,
    uint32_t *sample_buff,
    size_t buffer_size)
{
    int status = FWK_SUCCESS;
    struct sensor_smcf_drv_element_config *device_sensor_cfg;
    unsigned int device_idx;

    if (!fwk_module_is_valid_element_id(sensor_id) || sample_buff == NULL ||
        buffer_size == 0) {
        return FWK_E_PARAM;
    };

    device_idx = fwk_id_get_element_idx(sensor_id);
    device_sensor_cfg = &(sensor_smcf_drv_ctx.element_config_table[device_idx]);
    if (buffer_size > device_sensor_cfg->max_samples_size) {
        return FWK_E_RANGE;
    }

    status = _get_smcf_sensor_data(device_idx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    memcpy(
        sample_buff, sensor_smcf_drv_ctx.sensor_smcf_data_buf.ptr, buffer_size);

    return FWK_SUCCESS;
}

static const struct mod_sensor_driver_api sensor_smcf_drv_get_value_api = {
    .get_value = sensor_smcf_drv_get_sensor_single_value,
};

static const struct mod_sensor_smcf_drv_multiple_samples_api
    sensor_smcf_drv_get_samples_api = {
        .get_samples = sensor_smcf_drv_get_sensor_multiple_samples,
    };

/*
 * Framework handlers
 */
static int sensor_smcf_drv_mod_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    if (element_count == 0) {
        return FWK_E_PARAM;
    }

    sensor_smcf_drv_ctx.num_of_devices = element_count;
    sensor_smcf_drv_ctx.element_config_table = fwk_mm_calloc(
        element_count, sizeof(struct sensor_smcf_drv_element_config));

    return FWK_SUCCESS;
}

static int sensor_smcf_drv_bind(fwk_id_t id, unsigned int round)
{
    if (round > 0) {
        return FWK_SUCCESS;
    }

    /* Bind to SMCF Data API */
    return fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SMCF),
        FWK_ID_API(FWK_MODULE_IDX_SMCF, MOD_SMCF_API_IDX_DATA),
        &sensor_smcf_drv_ctx.data_api);
}

static int sensor_smcf_drv_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    int status = FWK_SUCCESS;
    enum mod_sensor_smcf_drv_api_idx api_idx;

    if (fwk_id_is_equal(
            requester_id, FWK_ID_MODULE(FWK_MODULE_IDX_SENSOR_SMCF_DRV))) {
        return FWK_E_PARAM;
    }

    api_idx = (enum mod_sensor_smcf_drv_api_idx)fwk_id_get_api_idx(api_id);
    switch (api_idx) {
    case MOD_SENSOR_SMCF_DRV_API_IDX_GET_VALUE:
        *api = &sensor_smcf_drv_get_value_api;
        status = FWK_SUCCESS;
        break;

    case MOD_SENSOR_SMCF_DRV_API_IDX_GET_MULTIPLE_SAMPLES:
        *api = &sensor_smcf_drv_get_samples_api;
        status = FWK_SUCCESS;
        break;

    default:
        status = FWK_E_PARAM;
    };

    return status;
}

static int sensor_smcf_drv_start(fwk_id_t id)
{
    unsigned int i;
    uint32_t per_device_sample_data_max_size;

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        /* elements has already been started with module start */
        return FWK_SUCCESS;
    }

    if (!fwk_module_is_valid_module_id(id)) {
        return FWK_E_PARAM;
    }

    /* Calculate the max size for the smcf counter data buffer to accomodate
     * counter data for each element.
     * SMCF_COUNTER_DATA_SZ = MAX(COUNTER_DATA_SZ_FOR_EACH_device)
     */
    per_device_sample_data_max_size = 0;
    for (i = 0; i < sensor_smcf_drv_ctx.num_of_devices; ++i) {
        per_device_sample_data_max_size = FWK_MAX(
            per_device_sample_data_max_size,
            sensor_smcf_drv_ctx.element_config_table[i].max_samples_size);
    }

    sensor_smcf_drv_ctx.sensor_smcf_data_buf.size =
        (per_device_sample_data_max_size / sizeof(uint32_t));
    sensor_smcf_drv_ctx.sensor_smcf_data_buf.ptr = fwk_mm_calloc(
        sensor_smcf_drv_ctx.sensor_smcf_data_buf.size, sizeof(uint32_t));

    return FWK_SUCCESS;
}

static int sensor_smcf_drv_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    unsigned int device_idx;
    struct sensor_smcf_drv_element_config *element_cfg;
    if (!fwk_module_is_valid_element_id(element_id) || data == NULL) {
        return FWK_E_PARAM;
    }

    device_idx = fwk_id_get_element_idx(element_id);
    element_cfg = (struct sensor_smcf_drv_element_config *)data;
    sensor_smcf_drv_ctx.element_config_table[device_idx] = *element_cfg;

    return FWK_SUCCESS;
}

const struct fwk_module module_sensor_smcf_drv = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_SENSOR_SMCF_DRV_API_IDX_COUNT,
    .init = sensor_smcf_drv_mod_init,
    .element_init = sensor_smcf_drv_element_init,
    .start = sensor_smcf_drv_start,
    .bind = sensor_smcf_drv_bind,
    .process_bind_request = sensor_smcf_drv_process_bind_request,
};
