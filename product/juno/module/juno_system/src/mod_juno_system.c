/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_id.h"
#include "juno_scmi.h"
#include "juno_sds.h"
#include "juno_system.h"

#include <mod_juno_system.h>
#include <mod_juno_xrp7724.h>
#include <mod_power_domain.h>
#include <mod_psu.h>
#include <mod_scmi.h>
#include <mod_sds.h>
#include <mod_system_power.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static fwk_id_t sds_feature_availability_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SDS, JUNO_SDS_RAM_FEATURES_IDX);

/* SCMI services required to enable the messaging stack */
static unsigned int scmi_notification_table[] = {
    JUNO_SCMI_SERVICE_IDX_PSCI,
    JUNO_SCMI_SERVICE_IDX_OSPM_0,
    JUNO_SCMI_SERVICE_IDX_OSPM_1,
};

struct juno_system_ctx {
    /* SDS API */
    struct mod_sds_api *sds_api;

    /* XRP7724 PMIC API */
    const struct mod_juno_xrp7724_api_system_mode *juno_xrp7724_api;

    /* Running platform identifier */
    enum juno_idx_platform platform_id;

    struct psu_ctx {
        /* PSU API */
        const struct mod_psu_device_api *api;

        /* Cookie to respond to pre-state notification */
        uint32_t cookie;

        /* Whether the pre-state notification response is delayed */
        bool response_delayed;
    } psu_ctx;
};

static struct juno_system_ctx juno_system_ctx;

/*
 * Static helpers
 */

static int messaging_stack_ready(void)
{
    const uint32_t feature_flags = JUNO_SDS_FEATURE_FIRMWARE_MASK;

    const struct mod_sds_structure_desc *sds_structure_desc =
        fwk_module_get_data(sds_feature_availability_id);

    /*
     * Write SDS Feature Availability to signal the completion of the messaging
     * stack
     */
    return juno_system_ctx.sds_api->struct_write(sds_structure_desc->id,
        0, (void *)(&feature_flags), sds_structure_desc->size);
}

static int process_gpu_power_state(
    const struct fwk_event *event,
    struct fwk_event *resp_event,
    bool enable)
{
    int status = juno_system_ctx.psu_ctx.api->set_enabled(gpu_psu_id, enable);

    if (status == FWK_PENDING) {
        if (enable) {
            /* For OFF->ON transition, delay the notification response */
            resp_event->is_delayed_response = true;
            juno_system_ctx.psu_ctx.response_delayed = true;

            juno_system_ctx.psu_ctx.cookie = event->cookie;
        }

        status = FWK_SUCCESS;
    }

    return status;
}

/*
 * Functions fulfilling the System Power's API
 */
static int juno_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    int status = FWK_SUCCESS;

    if (juno_system_ctx.platform_id == JUNO_IDX_PLATFORM_FVP) {
        NVIC_SystemReset();

        fwk_unreachable();
    }

    switch (system_shutdown) {
    case MOD_PD_SYSTEM_SHUTDOWN:
    case MOD_PD_SYSTEM_FORCED_SHUTDOWN:
        status = juno_system_ctx.juno_xrp7724_api->shutdown();
        break;

    case MOD_PD_SYSTEM_COLD_RESET:
        status = juno_system_ctx.juno_xrp7724_api->reset();
        break;

    default:
        status = FWK_E_SUPPORT;
    }

    return status;
}

static const struct mod_system_power_driver_api
    juno_system_system_power_driver_api = {
        .system_shutdown = juno_system_shutdown,
};

/*
 * Framework API
 */
static int juno_system_module_init(fwk_id_t module_id,
                                   unsigned int element_count,
                                   const void *data)
{
    int status;

    fwk_assert(element_count == 0);

    status = juno_id_get_platform(&juno_system_ctx.platform_id);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int juno_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round > 0)
        return FWK_SUCCESS;

    status = fwk_module_bind(fwk_module_id_sds,
                             FWK_ID_API(FWK_MODULE_IDX_SDS, 0),
                             &juno_system_ctx.sds_api);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    if (juno_system_ctx.platform_id != JUNO_IDX_PLATFORM_RTL)
        return FWK_SUCCESS;
    else {

    status = fwk_module_bind(
        gpu_psu_id,
        psu_api_id,
        &juno_system_ctx.psu_ctx.api);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return fwk_module_bind(fwk_module_id_juno_xrp7724,
        mod_juno_xrp7724_api_id_system_mode,
        &juno_system_ctx.juno_xrp7724_api);
    }
}

static int juno_system_process_bind_request(fwk_id_t source_id,
                                            fwk_id_t target_id,
                                            fwk_id_t api_id,
                                            const void **api)
{
    *api = &juno_system_system_power_driver_api;

    return FWK_SUCCESS;
}

static int juno_system_start(fwk_id_t id)
{
    int status;
    unsigned int i;

    /*
     * Subscribe to GPU Power Domain notifications.
     */
    status = fwk_notification_subscribe(
        mod_pd_notification_id_power_state_pre_transition,
        gpu_pd_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_notification_subscribe(
        mod_pd_notification_id_power_state_transition,
        gpu_pd_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    /*
     * Subscribe to these SCMI channels in order to know when they have all
     * initialized.
     * At that point we can consider the SCMI stack to be initialized from
     * the point of view of the PSCI agent.
     */
    for (i = 0; i < FWK_ARRAY_SIZE(scmi_notification_table); i++) {
        status = fwk_notification_subscribe(
            mod_scmi_notification_id_initialized,
            fwk_id_build_element_id(fwk_module_id_scmi,
                scmi_notification_table[i]),
            id);
        if (status != FWK_SUCCESS)
            return status;
    }

    /*
     * Subscribe to the SDS initialized notification so we can correctly let the
     * PSCI agent know that the SCMI stack is initialized.
     */
    return fwk_notification_subscribe(
        mod_sds_notification_id_initialized,
        fwk_module_id_sds,
        id);
}

static int juno_system_process_notification(const struct fwk_event *event,
                                            struct fwk_event *resp_event)
{
    static unsigned int scmi_notification_count = 0;
    static bool sds_notification_received = false;
    int status = FWK_SUCCESS;

    struct mod_pd_power_state_pre_transition_notification_params
        *state_pre_params;
    struct mod_pd_power_state_transition_notification_params *params;
    struct mod_pd_power_state_pre_transition_notification_resp_params
        *pd_resp_params;

    if (!fwk_expect(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE))) {
        status = FWK_E_PARAM;

        goto exit;
    }

    if (fwk_id_is_equal(event->id, mod_scmi_notification_id_initialized))
        scmi_notification_count++;
    else if (fwk_id_is_equal(event->id, mod_sds_notification_id_initialized))
        sds_notification_received = true;
    else if (fwk_id_is_equal(
                 event->id,
                 mod_pd_notification_id_power_state_pre_transition)) {
        state_pre_params =
            (struct mod_pd_power_state_pre_transition_notification_params *)
                event->params;
        pd_resp_params =
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            resp_event->params;

        if (fwk_id_is_equal(event->source_id, gpu_pd_id)) {
            if (state_pre_params->target_state == MOD_PD_STATE_ON)
                status = process_gpu_power_state(event, resp_event, true);
        } else
            status = FWK_E_PARAM;

        pd_resp_params->status = status;
    } else if (fwk_id_is_equal(
                   event->id, mod_pd_notification_id_power_state_transition)) {
        params = (struct mod_pd_power_state_transition_notification_params *)
            event->params;

        if (fwk_id_is_equal(event->source_id, gpu_pd_id)) {
            if (params->state == MOD_PD_STATE_OFF)
                status = process_gpu_power_state(event, resp_event, false);
        } else
            status = FWK_E_PARAM;
    } else {
        status = FWK_E_PARAM;

        goto exit;
    }

    if ((scmi_notification_count == FWK_ARRAY_SIZE(scmi_notification_table)) &&
        sds_notification_received) {
        messaging_stack_ready();

        scmi_notification_count = 0;
        sds_notification_received = false;
    }

exit:
    return status;
}

static int juno_system_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct mod_psu_driver_response *psu_params;
    struct mod_pd_power_state_pre_transition_notification_resp_params
        *pd_resp_params;
    struct fwk_event resp;
    int status;

    if (fwk_id_is_equal(event->id, mod_psu_event_id_set_enabled)) {
        /* Response event from the PSU module */
        psu_params = (struct mod_psu_driver_response *)event->params;

        if (!juno_system_ctx.psu_ctx.response_delayed)
            return psu_params->status;

        juno_system_ctx.psu_ctx.response_delayed = false;

        pd_resp_params =
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            resp.params;

        /*
         * Respond to the notification so the GPU power domain can be turned on
         */
        status = fwk_thread_get_delayed_response(
            fwk_module_id_juno_system, juno_system_ctx.psu_ctx.cookie, &resp);
        if (status != FWK_SUCCESS)
            return status;

        pd_resp_params->status = psu_params->status;
        return fwk_thread_put_event(&resp);
    }

    return FWK_E_PARAM;
}

const struct fwk_module module_juno_system = {
    .name = "Juno SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = juno_system_module_init,
    .bind = juno_system_bind,
    .process_bind_request = juno_system_process_bind_request,
    .start = juno_system_start,
    .process_notification = juno_system_process_notification,
    .process_event = juno_system_process_event,
};

/* No elements, no configuration data */
struct fwk_module_config config_juno_system = {
    .get_element_table = NULL,
    .data = NULL,
};
