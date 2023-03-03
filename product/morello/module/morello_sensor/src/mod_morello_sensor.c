/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/morello_scp2pcc.h>

#include <mod_morello_scp2pcc.h>
#include <mod_morello_sensor.h>
#include <mod_morello_sensor_driver.h>
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

/* MORELLO sensor driver information */
#define MORELLO_SENSOR_VERSION_MAJOR 1
#define MORELLO_SENSOR_VERSION_MINOR 0

static struct morello_sensor_ctx sensor_ctx;

/* Morello Sensor names */
static const char *const sensor_type_name[MOD_MORELLO_VOLT_SENSOR_COUNT] = {
    [MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER0] = "T-CLUS0",
    [MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER1] = "T-CLUS1",
    [MOD_MORELLO_TEMP_SENSOR_IDX_SYSTEM] = "T-SYS",
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE0] = "V-CLUS0CORE0",
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE1] = "V-CLUS0CORE1",
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE0] = "V-CLUS1CORE0",
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE1] = "V-CLUS1CORE1",
};

static void morello_sensor_timer_callback(uintptr_t unused)
{
    struct morello_temp_sensor_ctx *t_dev_ctx;
    struct morello_volt_sensor_ctx *v_dev_ctx;
    unsigned int count;
    int status;
    int32_t value;

    for (count = 0; count < sensor_ctx.module_config->t_sensor_count; count++) {
        t_dev_ctx = &sensor_ctx.t_dev_ctx_table[count];
        status =
            morello_sensor_lib_sample(&value, MOD_MORELLO_TEMP_SENSOR, count);
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
                    MOD_SCP2PCC_SEND_SHUTDOWN, NULL, 0, NULL, NULL);
                if (status != FWK_SUCCESS) {
                    FWK_LOG_ERR(
                        "[MORELLO SENSOR] Shutdown request to PCC failed");
                    return;
                }
            }

            t_dev_ctx->sensor_data_buffer[t_dev_ctx->buf_index++] = value;

            if (t_dev_ctx->buf_index == PVT_HISTORY_LEN) {
                t_dev_ctx->buf_index = 0;
            }
        }
    }
    /* Start new sample. */
    morello_sensor_lib_trigger_sample(MOD_MORELLO_TEMP_SENSOR);

    /* Get the Voltage Monitor values */
    for (count = 0; count < sensor_ctx.module_config->v_sensor_count; count++) {
        v_dev_ctx = &sensor_ctx.v_dev_ctx_table[count];
        status =
            morello_sensor_lib_sample(&value, MOD_MORELLO_VOLT_SENSOR, count);
        if (status != FWK_SUCCESS) {
            return;
        }

        v_dev_ctx->sensor_data_buffer[v_dev_ctx->buf_index++] = value;

        if (v_dev_ctx->buf_index == PVT_HISTORY_LEN) {
            v_dev_ctx->buf_index = 0;
        }
    }
    /* Start new sample. */
    morello_sensor_lib_trigger_sample(MOD_MORELLO_VOLT_SENSOR);
}

/*
 * Module API
 */
static int get_value(fwk_id_t element_id, mod_sensor_value_t *value)
{
#ifdef BUILD_HAS_SENSOR_SIGNED_VALUE
    return FWK_E_SUPPORT;
#else
    struct morello_temp_sensor_ctx *t_dev_ctx;
    struct morello_volt_sensor_ctx *v_dev_ctx;
    unsigned int el_idx;
    uint8_t t_sensor_count;
    uint8_t v_sensor_count;
    int32_t buf_value;

    el_idx = fwk_id_get_element_idx(element_id);
    t_sensor_count = sensor_ctx.module_config->t_sensor_count;
    v_sensor_count = sensor_ctx.module_config->v_sensor_count;

    if (el_idx > (t_sensor_count + v_sensor_count)) {
        return FWK_E_PARAM;
    }

    if (el_idx < t_sensor_count) {
        t_dev_ctx = &sensor_ctx.t_dev_ctx_table[el_idx];
        if (t_dev_ctx == NULL) {
            return FWK_E_DATA;
        }

        buf_value = t_dev_ctx->sensor_data_buffer
                        [t_dev_ctx->buf_index == 0 ? PVT_HISTORY_LEN - 1 :
                                                     t_dev_ctx->buf_index - 1];

    } else {
        v_dev_ctx = &sensor_ctx.v_dev_ctx_table[el_idx - t_sensor_count];
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

static const struct mod_sensor_driver_api morello_sensor_api = {
    .get_value = get_value,
};

/*
 * Framework handlers
 */
static int morello_sensor_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    fwk_assert(data != NULL);

    if (element_count == 0) {
        return FWK_E_DATA;
    }

    sensor_ctx.module_config = (struct mod_morello_sensor_config *)data;

    sensor_ctx.t_dev_ctx_table = fwk_mm_calloc(
        sensor_ctx.module_config->t_sensor_count,
        sizeof(sensor_ctx.t_dev_ctx_table[0]));

    sensor_ctx.v_dev_ctx_table = fwk_mm_calloc(
        sensor_ctx.module_config->v_sensor_count,
        sizeof(sensor_ctx.v_dev_ctx_table[0]));

    return FWK_SUCCESS;
}

static int morello_sensor_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct morello_temp_sensor_ctx *t_dev_ctx;
    struct morello_volt_sensor_ctx *v_dev_ctx;
    struct mod_morello_temp_sensor_config *t_config;
    struct mod_morello_volt_sensor_config *v_config;
    unsigned int el_idx;
    uint8_t t_sensor_count;

    fwk_assert(data != NULL);
    el_idx = fwk_id_get_element_idx(element_id);
    t_sensor_count = sensor_ctx.module_config->t_sensor_count;

    if (el_idx < t_sensor_count) {
        t_config = (struct mod_morello_temp_sensor_config *)data;

        t_dev_ctx = &sensor_ctx.t_dev_ctx_table[el_idx];

        t_dev_ctx->config = t_config;

        t_dev_ctx->sensor_data_buffer =
            fwk_mm_calloc(PVT_HISTORY_LEN, sizeof(int32_t));

        t_dev_ctx->buf_index = 0;
    } else {
        v_config = (struct mod_morello_volt_sensor_config *)data;

        v_dev_ctx = &sensor_ctx.v_dev_ctx_table[el_idx - t_sensor_count];

        v_dev_ctx->config = v_config;

        v_dev_ctx->sensor_data_buffer =
            fwk_mm_calloc(PVT_HISTORY_LEN, sizeof(int32_t));

        v_dev_ctx->buf_index = 0;
    }

    return FWK_SUCCESS;
}

static int morello_sensor_bind(fwk_id_t id, unsigned int round)
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

            return fwk_module_bind(
                FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_SCP2PCC),
                FWK_ID_API(FWK_MODULE_IDX_MORELLO_SCP2PCC, 0),
                &sensor_ctx.scp2pcc_api);
        }
    }
    return FWK_SUCCESS;
}

static int morello_sensor_start(fwk_id_t id)
{
    int status;
    uint32_t error_reg;
    int exit_status = FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        status = morello_sensor_lib_init(&error_reg);

        switch (status) {
        case FWK_E_DEVICE:
            FWK_LOG_ERR("[PVT] ID invalid: 0x%08X", (unsigned int)error_reg);
            exit_status = FWK_E_DEVICE;
            break;
        case FWK_E_DATA:
            FWK_LOG_ERR(
                "[PVT] Scratch test failed: 0x%08X", (unsigned int)error_reg);
            exit_status = FWK_E_DEVICE;
            break;
        case FWK_E_TIMEOUT:
            FWK_LOG_ERR("[PVT] Timeout waiting for sensor initialization!");
            exit_status = FWK_E_TIMEOUT;
            break;
        default:
            break;
        }

        if (exit_status != FWK_SUCCESS) {
            return exit_status;
        }

        /* Do the initial conversion */
        morello_sensor_timer_callback(0);

        status = sensor_ctx.timer_alarm_api->start(
            sensor_ctx.module_config->alarm_id,
            1000,
            MOD_TIMER_ALARM_TYPE_PERIODIC,
            &morello_sensor_timer_callback,
            0);

        if (status != FWK_SUCCESS) {
            return status;
        }

        FWK_LOG_INFO(
            "[PVT] Started driver version %d.%d",
            MORELLO_SENSOR_VERSION_MAJOR,
            MORELLO_SENSOR_VERSION_MINOR);
    }
    return FWK_SUCCESS;
}

static int morello_sensor_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_type,
    const void **api)
{
    *api = &morello_sensor_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_morello_sensor = {
    .api_count = 1,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = morello_sensor_init,
    .element_init = morello_sensor_element_init,
    .bind = morello_sensor_bind,
    .start = morello_sensor_start,
    .process_bind_request = morello_sensor_process_bind_request,
};
