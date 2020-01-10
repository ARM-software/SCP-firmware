/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno Thermal Protection
 */

#include "config_juno_thermal.h"
#include "config_power_domain.h"
#include "juno_alarm_idx.h"
#include "juno_id.h"

#include <mod_juno_thermal.h>
#include <mod_power_domain.h>
#include <mod_sensor.h>
#include <mod_system_power.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

enum mod_juno_thermal_event_idx {
    MOD_JUNO_THERMAL_EVENT_IDX_TIMER,
    MOD_JUNO_THERMAL_EVENT_IDX_COUNT,
};

static const struct mod_pd_restricted_api *pd_api;

static const fwk_id_t systop_pd_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN,
    POWER_DOMAIN_IDX_SYSTOP);

/*
 * The sensor used for monitoring the temperature is available only on the real
 * board, thus when running on FVP this module does not need to provide
 * protection.
 */
static bool is_platform_fvp;

struct thermal_dev_ctx {
    const struct mod_juno_thermal_element_config *config;
    const struct mod_sensor_api *sensor_api;
    const struct mod_timer_alarm_api *alarm_api;
};

static const fwk_id_t mod_juno_thermal_event_id_timer =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_JUNO_THERMAL,
                      MOD_JUNO_THERMAL_EVENT_IDX_TIMER);

static struct thermal_dev_ctx *ctx_table;

/*
 * Periodical alarm callback
 */

static void juno_thermal_alarm_callback(uintptr_t param)
{
    int status;
    unsigned int elem_idx = (unsigned int)param;

    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_JUNO_THERMAL),
        .target_id = fwk_id_build_element_id(
            fwk_module_id_juno_thermal,
            elem_idx),
        .id = mod_juno_thermal_event_id_timer,
    };

    status = fwk_thread_put_event(&event);
    fwk_assert(status == FWK_SUCCESS);
}

/*
 * Framework handlers
 */

static int juno_thermal_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *config)
{
    int status;
    enum juno_idx_platform platform_id = JUNO_IDX_PLATFORM_COUNT;

    fwk_assert(element_count == 1);

    status = juno_id_get_platform(&platform_id);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_PANIC;

    is_platform_fvp = (platform_id == JUNO_IDX_PLATFORM_FVP);

    if (is_platform_fvp)
        return FWK_SUCCESS;

    ctx_table = fwk_mm_calloc(element_count, sizeof(struct thermal_dev_ctx));
    if (ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int juno_thermal_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    struct thermal_dev_ctx *ctx;
    fwk_id_t sensor_id;

    if (is_platform_fvp)
        return FWK_SUCCESS;

    ctx = &ctx_table[fwk_id_get_element_idx(element_id)];
    ctx->config = (struct mod_juno_thermal_element_config *)data;

    sensor_id = ctx->config->sensor_id;

    /* Validate identifiers */
    if (fwk_id_get_module_idx(sensor_id) != FWK_MODULE_IDX_SENSOR)
        return FWK_E_DATA;
    else
        return FWK_SUCCESS;
}

static int juno_thermal_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct thermal_dev_ctx *ctx;

    if ((round > 0) || is_platform_fvp)
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /* Bind to power domain - only binding to module is allowed */
        return fwk_module_bind(
            fwk_module_id_power_domain, mod_pd_api_id_restricted, &pd_api);
    }

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    status = fwk_module_bind(
        ctx->config->alarm_id,
        MOD_TIMER_API_ID_ALARM,
        &ctx->alarm_api);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    status = fwk_module_bind(
        ctx->config->sensor_id,
        mod_sensor_api_id_sensor,
        &ctx->sensor_api);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int juno_thermal_start(fwk_id_t id)
{
    int status;
    struct thermal_dev_ctx *ctx;

    /* Nothing to start for module */
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE) || is_platform_fvp)
        return FWK_SUCCESS;

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    status = fwk_notification_subscribe(
        mod_pd_notification_id_power_state_pre_transition,
        systop_pd_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    status = ctx->alarm_api->start(
        ctx->config->alarm_id,
        ctx->config->period_ms,
        MOD_TIMER_ALARM_TYPE_PERIODIC,
        juno_thermal_alarm_callback,
        (uintptr_t)fwk_id_get_element_idx(id));

    return status;
}

static int check_threshold_breach(uint64_t temperature, uint64_t threshold)
{
    if (temperature > threshold) {
        FWK_LOG_WARN("[THERMAL] system shutdown");

        return pd_api->system_shutdown(MOD_PD_SYSTEM_FORCED_SHUTDOWN);
    }

    return FWK_SUCCESS;
}

static int juno_thermal_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct thermal_dev_ctx *ctx;
    int status;
    uint64_t value;

    ctx = &ctx_table[fwk_id_get_element_idx(event->target_id)];

    /* Event from timer callback */
    if (fwk_id_is_equal(event->id, mod_juno_thermal_event_id_timer)) {
        status = ctx->sensor_api->get_value(ctx->config->sensor_id, &value);
        if (status == FWK_SUCCESS) {
            status = check_threshold_breach(
                value,
                ctx->config->thermal_threshold_mdc);
        } else if (status == FWK_PENDING)
            return FWK_SUCCESS;

    /* Response event from sensor HAL */
    } else if (fwk_id_is_equal(event->id, mod_sensor_event_id_read_request)) {
        struct mod_sensor_event_params *params =
        (struct mod_sensor_event_params *)event->params;

        if (params->status != FWK_SUCCESS)
            return params->status;
        status = check_threshold_breach(
            params->value,
            ctx->config->thermal_threshold_mdc);

    } else
        return FWK_E_PARAM;

    return status;
}

static int juno_thermal_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;
    struct thermal_dev_ctx *ctx;

    if (fwk_id_is_equal(
        event->id,
        mod_pd_notification_id_power_state_pre_transition)) {

        struct mod_pd_power_state_pre_transition_notification_params
            *pd_pre_transition_params;
        struct mod_pd_power_state_pre_transition_notification_resp_params
            *pd_resp_params;

        pd_pre_transition_params =
        (struct mod_pd_power_state_pre_transition_notification_params *)event
            ->params;
        pd_resp_params =
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            resp_event->params;

        if ((pd_pre_transition_params->target_state == MOD_PD_STATE_OFF) ||
            (pd_pre_transition_params->target_state ==
                MOD_SYSTEM_POWER_POWER_STATE_SLEEP0)) {

            /* Stop the timer alarm and change subscription for post-state */
            ctx = &ctx_table[fwk_id_get_element_idx(event->target_id)];

            status = ctx->alarm_api->stop(ctx->config->alarm_id);
            if (status != FWK_SUCCESS) {
                pd_resp_params->status = status;
                return status;
            }

            status = fwk_notification_unsubscribe(
                mod_pd_notification_id_power_state_pre_transition,
                systop_pd_id,
                event->target_id);
            if (status != FWK_SUCCESS) {
                pd_resp_params->status = status;
                return status;
            }

            status = fwk_notification_subscribe(
                mod_pd_notification_id_power_state_transition,
                systop_pd_id,
                event->target_id);

        } else
            status = FWK_SUCCESS;

        pd_resp_params->status = status;

        return status;
    } else if (fwk_id_is_equal(
        event->id,
        mod_pd_notification_id_power_state_transition)) {

        struct mod_pd_power_state_transition_notification_params *params =
            (struct mod_pd_power_state_transition_notification_params *)
                event->params;

        if (params->state == MOD_PD_STATE_ON) {

            /* Restart timer and change subscription for pre-state */
            ctx = &ctx_table[fwk_id_get_element_idx(event->target_id)];

            status = ctx->alarm_api->start(
                ctx->config->alarm_id,
                ctx->config->period_ms,
                MOD_TIMER_ALARM_TYPE_PERIODIC,
                juno_thermal_alarm_callback,
                (uintptr_t)fwk_id_get_element_idx(event->target_id));
            if (status != FWK_SUCCESS)
                return status;

            status = fwk_notification_unsubscribe(
                mod_pd_notification_id_power_state_transition,
                systop_pd_id,
                event->target_id);
            if (status != FWK_SUCCESS)
                return status;

            status = fwk_notification_subscribe(
                mod_pd_notification_id_power_state_pre_transition,
                systop_pd_id,
                event->target_id);
        } else
            status = FWK_SUCCESS;

        return status;
    } else
        return FWK_E_PARAM;
}

const struct fwk_module module_juno_thermal = {
    .name = "JUNO THERMAL",
    .type = FWK_MODULE_TYPE_SERVICE,
    .event_count = MOD_JUNO_THERMAL_EVENT_IDX_COUNT,
    .init = juno_thermal_init,
    .element_init = juno_thermal_element_init,
    .bind = juno_thermal_bind,
    .start = juno_thermal_start,
    .process_event = juno_thermal_process_event,
    .process_notification = juno_thermal_process_notification,
};
