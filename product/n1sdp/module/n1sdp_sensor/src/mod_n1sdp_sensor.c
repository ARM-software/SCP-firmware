/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/n1sdp_scp2pcc.h>

#include <mod_n1sdp_scp2pcc.h>
#include <mod_n1sdp_sensor.h>
#include <mod_n1sdp_sensor_driver.h>
#include <mod_power_domain.h>
#include <mod_sensor.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

/* N1SDP sensor driver information */
#define N1SDP_SENSOR_VERSION_MAJOR 1
#define N1SDP_SENSOR_VERSION_MINOR 1

struct n1sdp_sensor_ctx sensor_ctx;

static void n1sdp_sensor_timer_callback(uintptr_t unused)
{
    struct n1sdp_temp_sensor_ctx *t_dev_ctx;
    struct n1sdp_volt_sensor_ctx *v_dev_ctx;
    unsigned int count;
    uint32_t status;
    int32_t value;

    for (count = 0; count < sensor_ctx.module_config->t_sensor_count; count++) {
        t_dev_ctx = &sensor_ctx.t_dev_ctx_table[count];
        status = n1sdp_sensor_lib_sample(&value, MOD_N1SDP_TEMP_SENSOR, count);
        if (status == FWK_SUCCESS) {
            if (value >= t_dev_ctx->config->alarm_threshold &&
                value < t_dev_ctx->config->shutdown_threshold) {
                FWK_LOG_CRIT(
                    "%s temperature (%d) reached alarm threshold!",
                    sensor_type_name[count],
                    (int)value);
            } else if (value >= t_dev_ctx->config->shutdown_threshold) {
                FWK_LOG_CRIT(
                    "%s temperature (%d) reached shutdown threshold!",
                    sensor_type_name[count],
                    (int)value);

                status = sensor_ctx.scp2pcc_api->send(
                    NULL, 0, SCP2PCC_TYPE_SHUTDOWN);
            }

            t_dev_ctx->sensor_data_buffer[t_dev_ctx->buf_index++] = value;

            if (t_dev_ctx->buf_index == PVT_HISTORY_LEN) {
                t_dev_ctx->buf_index = 0;
            }
        }
    }
    /* Start new sample. */
    n1sdp_sensor_lib_trigger_sample(MOD_N1SDP_TEMP_SENSOR);

    status = n1sdp_sensor_lib_sample(&value, MOD_N1SDP_VOLT_SENSOR, 0);
    if (status == FWK_SUCCESS) {
        for (count = 0; count < sensor_ctx.module_config->v_sensor_count;
             count++) {
            v_dev_ctx = &sensor_ctx.v_dev_ctx_table[count];
            n1sdp_sensor_lib_sample(&value, MOD_N1SDP_VOLT_SENSOR, count);

            v_dev_ctx->sensor_data_buffer[v_dev_ctx->buf_index++] = value;

            if (v_dev_ctx->buf_index == PVT_HISTORY_LEN) {
                v_dev_ctx->buf_index = 0;
            }
        }
    }
    /* Start new sample. */
    n1sdp_sensor_lib_trigger_sample(MOD_N1SDP_VOLT_SENSOR);
}

/*
 * Module API
 */
static int get_value(fwk_id_t element_id, mod_sensor_value_t *value)
{
#ifdef BUILD_HAS_SENSOR_SIGNED_VALUE
    return FWK_E_SUPPORT;
#else
    struct n1sdp_temp_sensor_ctx *t_dev_ctx;
    struct n1sdp_volt_sensor_ctx *v_dev_ctx;
    unsigned int id;
    uint8_t t_sensor_count, v_sensor_count;
    int32_t buf_value;

    id = fwk_id_get_element_idx(element_id);
    t_sensor_count = sensor_ctx.module_config->t_sensor_count;
    v_sensor_count = sensor_ctx.module_config->v_sensor_count;

    if (id >= (t_sensor_count + v_sensor_count)) {
        return FWK_E_PARAM;
    }

    if (id < t_sensor_count) {
        t_dev_ctx = &sensor_ctx.t_dev_ctx_table[id];
        if (t_dev_ctx == NULL) {
            return FWK_E_DATA;
        }

        buf_value = t_dev_ctx->sensor_data_buffer
                        [t_dev_ctx->buf_index == 0 ? PVT_HISTORY_LEN - 1 :
                                                     t_dev_ctx->buf_index - 1];
    } else {
        v_dev_ctx = &sensor_ctx.v_dev_ctx_table[id - t_sensor_count];
        if (v_dev_ctx == NULL) {
            return FWK_E_DATA;
        }

        buf_value = v_dev_ctx->sensor_data_buffer
                        [v_dev_ctx->buf_index == 0 ? PVT_HISTORY_LEN - 1 :
                                                     v_dev_ctx->buf_index - 1];
    }
    *value = (uint64_t)buf_value;

    return FWK_SUCCESS;
#endif
}

static int get_info(fwk_id_t element_id, struct mod_sensor_info *info)
{
    struct mod_sensor_info *return_info;
    unsigned int id;
    uint8_t t_sensor_count, v_sensor_count;
    const struct mod_n1sdp_temp_sensor_config *t_config;
    const struct mod_n1sdp_volt_sensor_config *v_config;

    id = fwk_id_get_element_idx(element_id);
    t_sensor_count = sensor_ctx.module_config->t_sensor_count;
    v_sensor_count = sensor_ctx.module_config->v_sensor_count;

    if (id >= (t_sensor_count + v_sensor_count)) {
        return FWK_E_PARAM;
    }

    if (id < t_sensor_count) {
        t_config = fwk_module_get_data(element_id);
        return_info = t_config->info;
    } else {
        v_config = fwk_module_get_data(element_id);
        return_info = v_config->info;
    }

    if (!fwk_expect(return_info != NULL)) {
        return FWK_E_DATA;
    }

    *info = *return_info;

    return FWK_SUCCESS;
}

static const struct mod_sensor_driver_api n1sdp_sensor_api = {
    .get_value = get_value,
    .get_info = get_info,
};

/*
 * Framework handlers
 */
static int n1sdp_sensor_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    fwk_assert(data != NULL);

    if (element_count == 0) {
        return FWK_E_DATA;
    }

    sensor_ctx.module_config = (struct mod_n1sdp_sensor_config *)data;

    sensor_ctx.t_dev_ctx_table = fwk_mm_calloc(
        sensor_ctx.module_config->t_sensor_count,
        sizeof(sensor_ctx.t_dev_ctx_table[0]));

    if (sensor_ctx.t_dev_ctx_table == NULL) {
        return FWK_E_NOMEM;
    }

    sensor_ctx.v_dev_ctx_table = fwk_mm_calloc(
        sensor_ctx.module_config->v_sensor_count,
        sizeof(sensor_ctx.v_dev_ctx_table[0]));

    if (sensor_ctx.v_dev_ctx_table == NULL) {
        return FWK_E_NOMEM;
    }

    return FWK_SUCCESS;
}

static int n1sdp_sensor_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct n1sdp_temp_sensor_ctx *t_dev_ctx;
    struct n1sdp_volt_sensor_ctx *v_dev_ctx;
    struct mod_n1sdp_temp_sensor_config *t_config;
    struct mod_n1sdp_volt_sensor_config *v_config;
    unsigned int id;
    uint8_t t_sensor_count;

    fwk_assert(data != NULL);
    id = fwk_id_get_element_idx(element_id);
    t_sensor_count = sensor_ctx.module_config->t_sensor_count;

    if (id < t_sensor_count) {
        t_config = (struct mod_n1sdp_temp_sensor_config *)data;

        t_dev_ctx = &sensor_ctx.t_dev_ctx_table[id];
        if (t_dev_ctx == NULL) {
            return FWK_E_DATA;
        }

        t_dev_ctx->config = t_config;

        t_dev_ctx->sensor_data_buffer =
            fwk_mm_calloc(PVT_HISTORY_LEN, sizeof(int32_t));
        if (t_dev_ctx->sensor_data_buffer == NULL) {
            return FWK_E_NOMEM;
        }

        t_dev_ctx->buf_index = 0;
    } else {
        v_config = (struct mod_n1sdp_volt_sensor_config *)data;

        v_dev_ctx = &sensor_ctx.v_dev_ctx_table[id - t_sensor_count];
        if (v_dev_ctx == NULL) {
            return FWK_E_DATA;
        }

        v_dev_ctx->config = v_config;

        v_dev_ctx->sensor_data_buffer =
            fwk_mm_calloc(PVT_HISTORY_LEN, sizeof(int32_t));
        if (v_dev_ctx->sensor_data_buffer == NULL) {
            return FWK_E_NOMEM;
        }

        v_dev_ctx->buf_index = 0;
    }

    return FWK_SUCCESS;
}

static int n1sdp_sensor_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
            status = fwk_module_bind(
                sensor_ctx.module_config->alarm_id,
                sensor_ctx.module_config->alarm_api,
                &sensor_ctx.timer_alarm_api);

            if (status != FWK_SUCCESS) {
                return status;
            }

            status = fwk_module_bind(
                FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_SCP2PCC),
                FWK_ID_API(FWK_MODULE_IDX_N1SDP_SCP2PCC, 0),
                &sensor_ctx.scp2pcc_api);
            if (status != FWK_SUCCESS) {
                return status;
            }
        }
    }
    return FWK_SUCCESS;
}

static int n1sdp_sensor_start(fwk_id_t id)
{
    int status;
    uint32_t error_reg;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        status = n1sdp_sensor_lib_init(&error_reg);
        switch (status) {
        case FWK_E_DEVICE:
            FWK_LOG_INFO("[PVT] ID invalid: 0x%08X", (unsigned int)error_reg);
            return FWK_E_DEVICE;
        case FWK_E_DATA:
            FWK_LOG_INFO(
                "[PVT] Scratch test failed: 0x%08X", (unsigned int)error_reg);
            return FWK_E_DEVICE;
        case FWK_E_TIMEOUT:
            FWK_LOG_INFO("[PVT] Timeout waiting for sensor initialization!");
            return FWK_E_TIMEOUT;
        }

        status = sensor_ctx.timer_alarm_api->start(
            sensor_ctx.module_config->alarm_id,
            1000,
            MOD_TIMER_ALARM_TYPE_PERIODIC,
            &n1sdp_sensor_timer_callback,
            0);

        if (status != FWK_SUCCESS) {
            return status;
        }

        FWK_LOG_INFO(
            "[PVT] Started driver version %d.%d",
            N1SDP_SENSOR_VERSION_MAJOR,
            N1SDP_SENSOR_VERSION_MINOR);
    }
    return FWK_SUCCESS;
}

static int n1sdp_sensor_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_type,
    const void **api)
{
    *api = &n1sdp_sensor_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_sensor = {
    .api_count = 1,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = n1sdp_sensor_init,
    .element_init = n1sdp_sensor_element_init,
    .bind = n1sdp_sensor_bind,
    .start = n1sdp_sensor_start,
    .process_bind_request = n1sdp_sensor_process_bind_request,
};
